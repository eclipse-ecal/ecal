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
 * @brief  common data service client based on eCAL, adapted for Nanobind
**/

#include <ecal/ecal.h>
#include <wrappers/wrapper_client.h>

#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>
#include <wrappers/wrapper_datatypeinfo.h>

namespace eCAL
{
    CNBSrvClient::CNBSrvClient() : CServiceClient() { }

    CNBSrvClient::CNBSrvClient(const std::string& service_name) : CServiceClient(service_name) { }

    bool CNBSrvClient::WrapAddRespCB(nanobind::callable callback_)
    {
        assert(IsConnected());
        {
            std::lock_guard<std::mutex> callback_lock(m_python_resp_callback_mutex);
            m_python_resp_callback = callback_;
        }
        auto callback = std::bind(&CNBSrvClient::ResponseCallback, this, std::placeholders::_1);
        return(CServiceClient::AddResponseCallback(callback));
    }

    bool CNBSrvClient::WrapAddClientEventCB(eCAL_Client_Event type, nanobind::callable callback_)
    {
        assert(IsConnected());
        {
            std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
            m_python_event_callback = callback_;
        }
        auto callback = std::bind(&CNBSrvClient::AddCltEventCB, this, std::placeholders::_1, std::placeholders::_2);
        return(CNBSrvClient::AddEventCallback(type, callback));
    }

    void CNBSrvClient::ResponseCallback(const struct SServiceResponse& data_)
    {
        nanobind::callable fn_callback;
        {
            std::lock_guard<std::mutex> callback_lock(m_python_resp_callback_mutex);
            fn_callback = m_python_resp_callback;
        }

        try {
            nanobind::gil_scoped_acquire g2;
            fn_callback(data_.service_name, data_.response);
        }
        catch (const nanobind::python_error& e) {
            std::cout << e.what();
        }
    }
    void CNBSrvClient::AddCltEventCB(const char* client_name_, const struct SClientEventCallbackData* data_)
    {
        nanobind::callable fn_callback;
        {
            std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
            fn_callback = m_python_event_callback;
        }

        try {
            nanobind::gil_scoped_acquire g2;
            fn_callback(client_name_, data_->type, nanobind::int_(data_->time), data_->attr.hname);
        }
        catch (const nanobind::python_error& e) {
            std::cout << e.what();
        }
    }

}