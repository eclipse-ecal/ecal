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
 * @file   service/types.h
 * @brief  Type definitions for eCAL services
**/

#pragma once

#include <ecal/namespace.h>
#include <ecal/types.h>

#include <functional>
#include <string>
#include <vector>
#include <set>

namespace eCAL
{
  /**
   * @brief  Service call state.
  **/
  enum class eCallState
  {
    none = 0,    //!< undefined
    executed,    //!< executed (successfully)
    timeouted,   //!< timeout
    failed       //!< failed
  };

  /**
   * @brief eCAL service client event callback type.
  **/
  enum class eClientEvent
  {
    none = 0,
    connected = 1,
    disconnected = 2,
    timeout = 3,
  };

  inline std::string to_string(eClientEvent event_) {
    switch (event_) {
    case eClientEvent::none:                   return "NONE";
    case eClientEvent::connected:              return "CONNECTED";
    case eClientEvent::disconnected:           return "DISCONNECTED";
    default:            return "Unknown";
    }
  }

  /**
   * @brief eCAL service server event callback type.
  **/
  enum class eServerEvent
  {
    none = 0,
    connected = 1,
    disconnected = 2,
  };

  inline std::string to_string(eServerEvent event_) {
    switch (event_) {
    case eServerEvent::none:                   return "NONE";
    case eServerEvent::connected:              return "CONNECTED";
    case eServerEvent::disconnected:           return "DISCONNECTED";
    default:            return "Unknown";
    }
  }

  namespace Registration
  {

    struct SServiceId
    {
      SEntityId    service_id;
      std::string  service_name;

      bool operator==(const SServiceId& other) const
      {
        return service_id == other.service_id && service_name == other.service_name;
      }

      bool operator<(const SServiceId& other) const
      {
        return std::tie(service_id, service_name) < std::tie(other.service_id, other.service_name);
      }
    };

    struct SServiceMethodId
    {
      SEntityId    service_id;
      std::string  service_name;
      std::string  method_name;

      bool operator==(const SServiceMethodId& other) const
      {
        return service_id == other.service_id && service_name == other.service_name && method_name == other.method_name;
      }

      bool operator<(const SServiceMethodId& other) const
      {
        return std::tie(service_id, service_name, method_name) < std::tie(other.service_id, other.service_name, other.method_name);
      }
    };
  }

  /**
   * @brief Service method information struct containing the request and response type information.
  **/
  struct SServiceMethodInformation
  {
    std::string              method_name;     //!< The name of the method.
    SDataTypeInformation     request_type;    //!< The type of the method request.
    SDataTypeInformation     response_type;   //!< The type of the method response.

    bool operator==(const SServiceMethodInformation& other) const
    {
      return method_name == other.method_name && request_type == other.request_type && response_type == other.response_type;
    }

    bool operator<(const SServiceMethodInformation& other) const
    {
      return std::tie(method_name, method_name, response_type) < std::tie(other.method_name, other.method_name, other.response_type);
    }
  };

  /**
   * @brief Service response struct containing the (responding) server informations and the response itself.
  **/
  struct SServiceIDResponse
  {
    Registration::SServiceMethodId service_method_id;            //!< service method information (service id (entity id, process id, host name), service name, method name)
    std::string                    error_msg;                    //!< human readable error message
    int                            ret_state  = 0;               //!< return state of the called service method
    eCallState                     call_state = eCallState::none; //!< call state (see eCallState)
    std::string                    response;                     //!< service response
  };
  using ServiceIDResponseVecT = std::vector<SServiceIDResponse>; //!< vector of multiple service responses

  /**
   * @brief Service method callback function type (low level server interface).
   *
   * @param method_info The method information struct containing the request and response type information.
   * @param request_    The request.
   * @param response_   The response returned from the method call.
  **/
  using MethodInfoCallbackT = std::function<int(const SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_)>;

  /**
   * @brief Service response callback function type (low level client interface).
   * 
   * @param entity_id_         Unique service id (entity id, process id, host name, service name, method name)
   * @param service_response_  Service response struct containing the (responding) server informations and the response itself.
  **/
  using ResponseIDCallbackT = std::function<void (const Registration::SEntityId& entity_id_, const struct SServiceIDResponse& service_response_)>;

  /**
   * @brief Map of <method name, method information (like request type, reponse type)>.
  **/
  using ServiceMethodInfoSetT = std::set<SServiceMethodInformation>;
  
  ECAL_CORE_NAMESPACE_V6
  {
      /**
     * @brief eCAL client event callback struct.
    **/
    struct SClientEventCallbackData
    {
      eClientEvent      type{ eClientEvent::none };  //!< event type
      long long         time = 0;                  //!< event time in µs
    };

    /**
     * @brief Client event callback function type.
     *
     * @param service_id_  The service id struct of the connection that triggered the event.
     * @param data_        Event callback data structure with the event specific information.
    **/
    using ClientEventCallbackT = std::function<void(const Registration::SServiceId& service_id_, const SClientEventCallbackData& data_)>;

    /**
     * @brief eCAL server event callback struct.
    **/
    struct SServerEventCallbackData
    {
      eServerEvent      type{ eServerEvent::none };  //!< event type
      long long         time = 0;                  //!< event time in µs
    };

    /**
     * @brief Server event callback function type.
     *
     * @param service_id_  The service id struct of the connection that triggered the event.
     * @param data_        Event callback data structure with the event specific information.
    **/
    using ServerEventCallbackT = std::function<void(const Registration::SServiceId& service_id_, const struct SServerEventCallbackData& data_)>;
  }
    
}
