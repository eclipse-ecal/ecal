/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

/**
 * @file   msg/protobuf/client_base.h
 * @brief  Base class for templated Protobuf service clients.
 *
 * This file defines a base class for Protobuf-based service clients. It encapsulates
 * common functionality such as converting the underlying generic client instances into
 * templated ones and provides a helper for uniformly processing these instances.
 *
 * The class is now parameterized on two template arguments:
 *   - T: The service description type (which must provide or allow access to a protobuf ServiceDescriptor,
 *        typically via a protected GetDescriptor() method).
 *   - ClientInstanceT: The client instance wrapper type used for each service call.
 *
 * For example:
 *   - For untyped service calls, you might instantiate with ClientInstanceT = CClientInstanceUntyped<T>.
 *   - For typed service calls, you might instantiate with ClientInstanceT = CClientInstanceTyped<T>.
 *
 * This design allows the derived service client classes (such as the untyped and typed variants)
 * to use the same base functionality while receiving the appropriately wrapped client instance.
 */

#pragma once

#include <ecal/deprecate.h>
#include <ecal/service/client.h>

#include <ecal/msg/protobuf/client_protobuf_types.h>
#include <ecal/msg/protobuf/client_protobuf_utils.h>

#include <string>
#include <vector>
#include <utility>

namespace eCAL
{
  namespace protobuf
  {
    /// Default timeout value for blocking calls.
    static constexpr long long DEFAULT_TIME_ARGUMENT = -1;

    /**
     * @brief Base class for a templated Protobuf service client.
     *
     * This class provides common functionality for service clients that use Protobuf.
     * It converts the underlying generic eCAL client instances into templated client instances,
     * and it supplies a helper method to process each instance uniformly.
     *
     * @tparam T                The Protobuf service description type.
     * @tparam ClientInstanceT  The client instance wrapper type (e.g., CClientInstanceUntyped<T> or CClientInstanceTyped<T>).
     */
    template <typename T, typename ClientInstanceT>
    class CServiceClientBase : public eCAL::CServiceClient
    {
    public:
      /**
       * @brief Constructor using a default service name from the service descriptor.
       *
       * @param event_callback_  Optional event callback.
       */
      CServiceClientBase(const ClientEventCallbackT& event_callback_ = ClientEventCallbackT())
        : eCAL::CServiceClient(GetServiceNameFromDescriptor<T>(), CreateServiceMethodInformationSet<T>(), event_callback_)
      {
      }

      /**
       * @brief Constructor specifying an explicit service name.
       *
       * @param service_name_    The name of the service.
       * @param event_callback_  Optional event callback.
       */
      explicit CServiceClientBase(const std::string& service_name_, const ClientEventCallbackT& event_callback_ = ClientEventCallbackT())
        : eCAL::CServiceClient(service_name_, CreateServiceMethodInformationSet<T>(), event_callback_)
      {
      }

      // Non-copyable and non-movable.
      CServiceClientBase(const CServiceClientBase&) = delete;
      CServiceClientBase& operator=(const CServiceClientBase&) = delete;
      CServiceClientBase(CServiceClientBase&&) = delete;
      CServiceClientBase& operator=(CServiceClientBase&&) = delete;

      virtual ~CServiceClientBase() override = default;

      /**
       * @brief Converts base client instances to templated client instances.
       *
       * @return A vector of ClientInstanceT objects.
       */
      std::vector<ClientInstanceT> GetClientInstances() const
      {
        std::vector<eCAL::CClientInstance> base_instances = eCAL::CServiceClient::GetClientInstances();
        std::vector<ClientInstanceT> instances;
        instances.reserve(base_instances.size());
        for (auto& inst : base_instances)
        {
          instances.push_back(ClientInstanceT(std::move(inst)));
        }
        return instances;
      }

    protected:
      /**
       * @brief Generic helper that iterates over all client instances, applies a callable, and aggregates results.
       *
       * The callable should return a std::pair<bool, ResponseT> for each instance.
       *
       * @tparam ResponseT The type of response expected.
       * @tparam FuncT     The type of callable.
       * 
       * @param  func_     A callable that is passed each client instance.
       * 
       * @return A pair consisting of an overall success flag and a vector of responses.
       */
      template <typename ResponseT, typename FuncT>
      std::pair<bool, std::vector<ResponseT>> ProcessInstances(FuncT&& func_) const
      {
        bool overall_success = true;
        std::vector<ResponseT> responses;
        for (auto& instance : GetClientInstances())
        {
          auto ret = std::forward<FuncT>(func_)(instance);
          overall_success &= ret.first;
          responses.push_back(std::move(ret.second));
        }
        return { overall_success, responses };
      }
    };

  } // namespace protobuf
} // namespace eCAL
