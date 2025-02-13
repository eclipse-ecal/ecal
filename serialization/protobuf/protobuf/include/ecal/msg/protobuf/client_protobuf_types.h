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
 * @file   msg/protobuf/client_protobuf_types.h
 * @brief  eCAL client protobuf templated response type
**/

#pragma once

#include <ecal/service/types.h>

#include <string>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief Templated service response structure.
     *
     * This structure holds a typed response for a service call. The response is stored
     * as a smart pointer to the expected protobuf message type.
     *
     * @tparam ResponseT The expected protobuf response type.
     */
    template <typename ResponseT>
    struct SMsgServiceResponse
    {
      eCallState                 call_state = eCallState::none; //!< Call state (success/failure)
      SServiceId                 server_id;                     //!< Identifier of the server that executed the call
      SServiceMethodInformation  service_method_information;    //!< Information about the called method
      int                        ret_state = 0;                 //!< Return state of the service call
      ResponseT                  response;                      //!< The typed response message
      std::string                error_msg;                     //!< Error message if the call failed
    };

    /**
     * @brief Alias for a vector of typed service responses.
     *
     * @tparam ResponseT The expected protobuf response type.
     */
    template <typename ResponseT>
    using SMsgServiceResponseVecT = std::vector<SMsgServiceResponse<ResponseT>>;

    /**
     * @brief Typed callback for responses.
     *
     * @tparam ResponseT The expected protobuf response type.
     */
    template <typename ResponseT>
    using SMsgResponseCallbackT = std::function<void(const SMsgServiceResponse<ResponseT>&)>;
  }
}
