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
 * @brief  eCAL logging receiver class
**/

#pragma once

#include "config/attributes/ecal_log_receiver_attributes.h"
#include "io/udp/ecal_udp_sample_receiver.h"

#include <ecal/types/logging.h>

#include <atomic>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Logging
  {
    class CLogReceiver
    {
      public:
        /**
         * @brief Constructor.
        **/
        CLogReceiver(const SReceiverAttributes& attr_);

        /**
         * @brief Destructor.
        **/
        ~CLogReceiver();

        /**
         * @brief Start logging.
        **/
        void Start();

        /**
         * @brief Stop logging.
        **/
        void Stop();

        /**
         * @brief Get the log messages.
         *
         * @param log_msg_list_string_ The log messages.
         * 
         * @return True if enabled and the message receiving got called.
        **/
        bool GetLogging(std::string& log_msg_list_string_);

        /**
         * @brief Get the log messages.
         *
         * @param log_ The log messages.
        **/
        void GetLogging(Logging::SLogging& log_);

      private:
        bool HasSample(const std::string& sample_name_);
        bool ApplySample(const char* serialized_sample_data_, size_t serialized_sample_size_);

        std::atomic<bool>                           m_created;
        
        std::mutex                                  m_log_mtx;

        SReceiverAttributes                         m_attributes;

        // log message list and log message serialization buffer
        Logging::SLogging                           m_log_msglist;
        std::vector<char>                           m_log_message_vec;

        std::shared_ptr<eCAL::UDP::CSampleReceiver> m_log_receiver;
    };
  }
}