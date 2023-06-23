/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
 * @file   ecal_service_info.h
 * @brief  eCAL service info
**/

#pragma once

#include <ecal/cimpl/ecal_service_info_cimpl.h>

#include <functional>
#include <string>
#include <vector>

namespace eCAL
{
  /**
   * @brief Service response struct containing the (responding) server informations and the response itself.
  **/
  struct SServiceResponse
  {
    SServiceResponse()
    {
      ret_state  = 0;
      call_state = call_state_none;
    };
    std::string  host_name;      //!< service host name
    std::string  service_name;   //!< name of the service
    std::string  service_id;     //!< id of the service
    std::string  method_name;    //!< name of the service method
    std::string  error_msg;      //!< human readable error message
    int          ret_state;      //!< return state of the called service method
    eCallState   call_state;     //!< call state (see eCallState)
    std::string  response;       //!< service response
  };
  typedef std::vector<SServiceResponse> ServiceResponseVecT; //!< vector of multiple service responses

  /**
   * @brief Service method callback function type (low level server interface).
   *
   * @param method_     The method name.
   * @param req_type_   The type of the method request.
   * @param resp_type_  The type of the method response.
   * @param request_    The request.
   * @param response_   The response returned from the method call.
  **/
  typedef std::function<int(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request_, std::string& response_)> MethodCallbackT;

  /**
   * @brief Service response callback function type (low level client interface).
   *
   * @param service_response_  Service response struct containing the (responding) server informations and the response itself.
  **/
  typedef std::function<void(const struct SServiceResponse& service_response_)> ResponseCallbackT;
};
