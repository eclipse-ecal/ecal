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
 * @file   client_base.h
**/

#pragma once

#include <ecal/deprecate.h>
#include <ecal/service/client.h>

#include <ecal/msg/protobuf/client_instance.h>
#include <ecal/msg/protobuf/client_protobuf_types.h>
#include <ecal/msg/protobuf/client_protobuf_utils.h>

#include <string>
#include <vector>
#include <utility>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief Base class for a templated Protobuf service client.
     *
     * Contains common functionality such as converting base client
     * instances into templated ones and a helper method for processing
     * each client instance.
     */
    template <typename T>
    class CServiceClientBase : public eCAL::CServiceClient
    {
    public:
      static constexpr long long DEFAULT_TIME_ARGUMENT = -1;  /*!< Use DEFAULT_TIME_ARGUMENT in the `CallWithResponse()` and `CallWithCallback()` functions for blocking calls */

      CServiceClientBase(const ClientEventCallbackT& event_callback = ClientEventCallbackT())
        : eCAL::CServiceClient(GetServiceNameFromDescriptor<T>(), CreateServiceMethodInformationSet<T>(), event_callback)
      {
      }

      explicit CServiceClientBase(const std::string& service_name, const ClientEventCallbackT& event_callback = ClientEventCallbackT())
        : eCAL::CServiceClient(service_name, CreateServiceMethodInformationSet<T>(), event_callback)
      {
      }

      // Non-copyable and non-movable.
      CServiceClientBase(const CServiceClientBase&) = delete;
      CServiceClientBase& operator=(const CServiceClientBase&) = delete;
      CServiceClientBase(CServiceClientBase&&) = delete;
      CServiceClientBase& operator=(CServiceClientBase&&) = delete;

      virtual ~CServiceClientBase() override = default;

      /**
       * @brief Converts base client instances to templated instances.
       *
       * @return A vector of CClientInstance<T>.
       */
      std::vector<CClientInstance<T>> GetClientInstances() const
      {
        std::vector<eCAL::CClientInstance> base_instances = eCAL::CServiceClient::GetClientInstances();
        std::vector<CClientInstance<T>> proto_instances;
        proto_instances.reserve(base_instances.size());
        for (auto& inst : base_instances)
        {
          proto_instances.push_back(CClientInstance<T>(std::move(inst)));
        }
        return proto_instances;
      }

    protected:
      /**
       * @brief Generic helper that iterates over all client instances,
       *        applies a callable, and aggregates results.
       *
       * The callable should return a std::pair<bool, ResponseT> for each instance.
       *
       * @tparam ResponseT The type of response expected.
       * @tparam Func      The type of callable.
       * @param func       A callable that is passed each client instance.
       * @return A pair consisting of an overall success flag and a vector of responses.
       */
      template <typename ResponseT, typename Func>
      std::pair<bool, std::vector<ResponseT>> ProcessInstances(Func&& func) const
      {
        bool overall_success = true;
        std::vector<ResponseT> responses;
        for (auto& instance : GetClientInstances())
        {
          auto ret = func(instance);
          overall_success &= ret.first;
          responses.push_back(std::move(ret.second));
        }
        return { overall_success, responses };
      }
    };

  } // namespace protobuf
} // namespace eCAL
