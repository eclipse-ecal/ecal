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
 * @file   wrapper_subscriber.h
 * @brief  Nanobind wrapper subscriber interface
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
#include <wrappers/wrapper_datatypeinfo.h>

namespace eCAL
{
    class CNBSubscriber : public CSubscriber
    {
    public:
        /**
         * @brief Constructor.
        **/
        CNBSubscriber();

        /**
         * @brief Constructor.
         *
         * @param topic_name   Unique topic name.
        **/
        CNBSubscriber(const std::string& topic_name); 
        /**
         * @brief Constructor.
         *
         * @param topic_name    Unique topic name.
         * @param datainfo      Topic information (encoding, type, descriptor)
        **/
        CNBSubscriber(const std::string& topic_name, const CNBDataTypeInformation& datainfo); 

        /**
         * @brief Receive a message from the publisher (able to process zero length buffer).
         *
         * @param nb_timeout  Maximum time before receive operation returns (in milliseconds, -1 means infinite).
         *
         * @return  Standard string for containing message content.
        **/
        std::string Receive(int nb_timeout);

        /**
         * @brief  Wrapper for Add callback function for incoming receives.
         *
         * @param callback_  The callback function to add.
         *
         * @return  True if succeeded, false if not.
        **/
        bool WrapAddRecCB(nanobind::callable callback_);

        /**
         * @brief Wrapper for Add callback function for subscriber events.
         *
         * @param event      The event type to react on.
         * @param callback_  The callback function to add.
         *
         * @return  True if succeeded, false if not.
        **/
        bool WrapAddSubEventCB(eCAL_Subscriber_Event event, nanobind::callable callback_);

    private:
        /**
         * @brief  Private function to receive python parameters for incoming receives.
         *
         * @param topic_name_   The topic name of the received message.
         * @param data_         eCAL subscriber receive callback struct.
        **/
        void ReceiveCallback(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_);

        /**
         * @brief  Private function to receive python parameters for subscriber events.
         *
         * @param event_name_   The event type to react on.
         * @param data_         eCAL subscriber event callback struct.
        **/
        void AddSubEventCB(const char* event_name_, const struct eCAL::SSubEventCallbackData* data_);

        // class members
        nanobind::callable m_python_event_callback;
        std::mutex m_python_event_callback_mutex;

        nanobind::callable m_python_rec_callback;
        std::mutex m_python_rec_callback_mutex;
    };
}