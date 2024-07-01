/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @file   wrapper_server.h
 * @brief  Nanobind wrapper server interface
**/

#pragma once

#include <ecal/ecal.h>
#include <stdint.h>
#include <string>
#include <functional>
#include <cstddef>
#include <mutex>
#include <ecal/ecal_types.h>

#include <nanobind/nanobind.h>

namespace eCAL
{
    class CNBSrvServer : public CServiceServer
    {
    public:
        /**
         * @brief Constructor.
        **/
        CNBSrvServer();

        /**
         * @brief Constructor.
         *
         * @param service_name   Unique service name.
        **/
        CNBSrvServer(const std::string& service_name);

        /**
         * @brief Wrapper for Add callback function for service client events.
         *
         * @param nb_method     Service method name
         * @param nb_req_type   Service method request type.
         * @param nb_resp_type  Service method response type.
         * @param callback_     The callback function to add.
         *
         * @return  True if succeeded, false if not.
        **/
        bool WrapAddMethodCB(const std::string& nb_method, const std::string& nb_req_type, const std::string& nb_resp_type, nanobind::callable callback_);

        /**
         * @brief Wrapper for Add callback function for service client events.
         *
         * @param type       The event type to react on.
         * @param callback_  The callback function to add.
         *
         * @return  True if succeeded, false if not.
        **/
        bool WrapAddSrvEventCB(eCAL_Server_Event type, nanobind::callable callback_);

    private:
        /**
         * @brief  Private function to receive python parameters for method.
         *
         * @param nb_method     Service method name
         * @param nb_req_type   Service method request type.
         * @param nb_resp_type  Service method response type.
         * @param request       Server request 
         * @param response      Server response
         * @param callback_     The callback function to add.
         *
         * @return  Result
        **/
        int MethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request, std::string& response);

        /**
         * @brief  Private function to receive python parameters for server events.
         *
         * @param client_name_   The client name
         * @param data_          eCAL client event callback struct.
        **/
        void AddSrvEventCB(const char* client_name_, const struct SServerEventCallbackData* data_);

        // class members
        nanobind::callable m_python_event_callback;
        std::mutex m_python_event_callback_mutex;

        nanobind::callable m_python_method_callback;
        std::mutex m_python_method_callback_mutex;
    };
}
