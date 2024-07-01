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
 * @file   wrapper_publisher.h
 * @brief  Nanobind wrapper publisher interface
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
    class CNBPublisher : public CPublisher
    {
    public:
        /**
         * @brief Constructor.
        **/
        CNBPublisher();

        /**
         * @brief Constructor.
         *
         * @param topic_name   Unique topic name.
        **/
        CNBPublisher(const std::string& topic_name);

        /**
         * @brief Constructor.
         *
         * @param topic_name    Unique topic name.
         * @param datainfo      Topic information (encoding, type, descriptor)
        **/
        CNBPublisher(const std::string& topic_name, const CNBDataTypeInformation& datainfo);

        /**
         * @brief Wrapper for Add callback function for publisher events.
         *
         * @param event      The event type to react on.
         * @param callback_  The callback function to add.
         *
         * @return  True if succeeded, false if not.
        **/
        bool WrapAddPubEventCB(eCAL_Publisher_Event event, nanobind::callable callback_);

    private:
        /**
        * @brief  Private function to receive python parameters for publisher events.
        *
        * @param event_name_   The event type to react on.
        * @param data_         eCAL publisher event callback struct.
        **/
        void AddPubEventCB(const char* event_name_, const struct eCAL::SPubEventCallbackData* data_);

        // class members
        nanobind::callable m_python_event_callback;
        std::mutex m_python_event_callback_mutex;

    };
}