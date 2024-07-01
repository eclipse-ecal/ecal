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
#include <wrappers/wrapper_server.h>

#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>
#include <wrappers/wrapper_datatypeinfo.h>

namespace eCAL
{
    CNBSrvServer::CNBSrvServer() : CServiceServer() { }

    CNBSrvServer::CNBSrvServer(const std::string& service_name) : CServiceServer(service_name) { }

    bool CNBSrvServer::WrapAddMethodCB(const std::string& nb_method, const std::string& nb_req_type, const std::string& nb_resp_type, nanobind::callable callback_)
    {
        assert(IsConnected());
        {
            std::lock_guard<std::mutex> callback_lock(m_python_method_callback_mutex);
            m_python_method_callback = callback_;
        }
        auto Servercallback = std::bind(&CNBSrvServer::MethodCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        return(CNBSrvServer::AddMethodCallback(nb_method, nb_req_type, nb_resp_type, Servercallback));
    }

    bool CNBSrvServer::WrapAddSrvEventCB(eCAL_Server_Event type, nanobind::callable callback_)
    {
        assert(IsConnected());
        {
            std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
            m_python_event_callback = callback_;
        }
        auto callback = std::bind(&CNBSrvServer::AddSrvEventCB, this, std::placeholders::_1, std::placeholders::_2);
        return(CNBSrvServer::AddEventCallback(type, callback));
    }

    int CNBSrvServer::MethodCallback(const std::string& method_, const std::string& req_type_, const std::string& resp_type_, const std::string& request, std::string& response)
    {
        nanobind::callable fn_callback;
        {
            std::lock_guard<std::mutex> callback_lock(m_python_method_callback_mutex);
            fn_callback = m_python_method_callback;
        }

        try {
            nanobind::gil_scoped_acquire g2;
            auto result = fn_callback(method_, req_type_, resp_type_, request);
            // do some check if object holds a tuple if (!result.is_type<nanobind::tuple>)
            nanobind::tuple result_tuple = nanobind::cast<nanobind::tuple>(result);
            response = nanobind::cast<std::string>(result[1]);
            int nb_int = nanobind::cast<int>(result[0]);
            return nb_int;
        }
        catch (const nanobind::python_error& e) {
            std::cout << e.what();
        }
    }

    void CNBSrvServer::AddSrvEventCB(const char* client_name_, const struct SServerEventCallbackData* data_)
    {
        nanobind::callable fn_callback;
        {
            std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
            fn_callback = m_python_event_callback;
        }

        try {
            nanobind::gil_scoped_acquire g2;
            fn_callback(client_name_, data_->type, nanobind::int_(data_->time));
        }
        catch (const nanobind::python_error& e) {
            std::cout << e.what();
        }
    }

}
