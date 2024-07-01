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
 * @brief  common data subscriber based on eCAL, adapted for Nanobind
**/

#include <ecal/ecal.h>
#include <wrappers/wrapper_subscriber.h>

#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>


namespace eCAL
{
    CNBSubscriber::CNBSubscriber() : CSubscriber() { }

    CNBSubscriber::CNBSubscriber(const std::string& topic_name) : CSubscriber(topic_name) { }

    CNBSubscriber::CNBSubscriber(const std::string& topic_name, const CNBDataTypeInformation& datainfo) : CSubscriber(topic_name, convert(datainfo)) { }

    std::string CNBSubscriber::Receive(int nb_timeout)
    {
        std::string Rec_Data = "";
        auto success = eCAL::CSubscriber::ReceiveBuffer(Rec_Data, nullptr, nb_timeout);
        return(Rec_Data);
    }

    bool CNBSubscriber::WrapAddRecCB(nanobind::callable callback_)
    {
        assert(IsCreated());
        RemReceiveCallback();

        {
            std::lock_guard<std::mutex> callback_lock(m_python_rec_callback_mutex);
            m_python_rec_callback = callback_;
        }
        auto callback = std::bind(&CNBSubscriber::ReceiveCallback, this, std::placeholders::_1, std::placeholders::_2);
        return(CSubscriber::AddReceiveCallback(callback));
    }

    bool CNBSubscriber::WrapAddSubEventCB(eCAL_Subscriber_Event event, nanobind::callable callback_)
    {
        assert(IsCreated());
        RemEventCallback(event);
        {
            std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
            m_python_event_callback = callback_;
        }
        auto callback = std::bind(&CNBSubscriber::AddSubEventCB, this, std::placeholders::_1, std::placeholders::_2);
        return(CSubscriber::AddEventCallback(event, callback));
    }

    void CNBSubscriber::ReceiveCallback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
    {
        nanobind::callable fn_callback;
        {
            std::lock_guard<std::mutex> callback_lock(m_python_rec_callback_mutex);
            fn_callback = m_python_rec_callback;
        }

        try {
            nanobind::gil_scoped_acquire g2;
            fn_callback(nanobind::str(topic_name_), nanobind::bytes((const char*)data_->buf, data_->size), nanobind::int_(data_->time));
        }
        catch (const nanobind::python_error& e) {
            std::cout << e.what();
        }
    }

    void CNBSubscriber::AddSubEventCB(const char* event_name_, const struct eCAL::SSubEventCallbackData* data_)
    {
        nanobind::callable fn_callback;
        {
            std::lock_guard<std::mutex> callback_lock(m_python_event_callback_mutex);
            fn_callback = m_python_event_callback;
        }

        try {
            nanobind::gil_scoped_acquire g2;
            fn_callback(event_name_, data_->type, nanobind::int_(data_->time), data_->tid);
        }
        catch (const nanobind::python_error& e) {
            std::cout << e.what();
        }
    }

}