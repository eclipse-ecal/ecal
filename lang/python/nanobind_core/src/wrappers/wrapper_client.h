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
 * @file   wrapper_client.h
 * @brief  Nanobind wrapper client interface
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
    class CNBSrvClient : public CServiceClient
    {
    public:
        /**
         * @brief Constructor.
        **/
        CNBSrvClient();

        /**
         * @brief Constructor.
         *
         * @param service_name   Unique service name.
        **/
        CNBSrvClient(const std::string& service_name);

        /**
         * @brief  Wrapper for Add callback function for incoming responses.
         *
         * @param callback_  The callback function to add.
         *
         * @return  True if succeeded, false if not.
        **/
        bool WrapAddRespCB(nanobind::callable callback_);

        /**
         * @brief Wrapper for Add callback function for service client events.
         *
         * @param type       The event type to react on.
         * @param callback_  The callback function to add.
         *
         * @return  True if succeeded, false if not.
        **/
        bool WrapAddClientEventCB(eCAL_Client_Event type, nanobind::callable callback_);

    private:
        /**
         * @brief  Private function to receive python parameters for incoming response.
         *
         * @param data_         eCAL Service response callback struct.
        **/
        void ResponseCallback(const struct SServiceResponse& data_);

        /**
         * @brief  Private function to receive python parameters for subscriber events.
         *
         * @param client_name_   The client name
         * @param data_          eCAL client event callback struct.
        **/
        void AddCltEventCB(const char* client_name_, const struct SClientEventCallbackData* data_);

        // class members
        nanobind::callable m_python_event_callback;
        std::mutex m_python_event_callback_mutex;

        nanobind::callable m_python_resp_callback;
        std::mutex m_python_resp_callback_mutex;
    };
}