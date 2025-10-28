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
 * @brief  eCAL logging provider class
**/

#pragma once

#include "config/attributes/ecal_log_provider_attributes.h"
#include "io/udp/ecal_udp_sample_sender.h"

#include <ecal/log_level.h>
#include <ecal/types/logging.h>

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Logging
  {
    class CLogProvider
    {
      public:
        /**
         * @brief Constructor.
        **/
        CLogProvider(const SProviderAttributes& attr_);

        /**
         * @brief Destructor.
        **/
        ~CLogProvider();

        /**
         * @brief Start logging.
        **/
        void Start();

        /**
         * @brief Stop logging.
        **/
        void Stop();

        /**
          * @brief Log a message.
          *
          * @param level_  The level.
          * @param msg_    The message.
        **/
        void Log(eLogLevel level_, const std::string& msg_);

      private:

        CLogProvider(const CLogProvider&);                 // prevent copy-construction
        CLogProvider& operator=(const CLogProvider&);      // prevent assignment

        bool StartFileLogging();
        bool StartUDPLogging();

        std::mutex                                m_log_mtx;

        std::atomic<bool>                         m_created;
        std::unique_ptr<eCAL::UDP::CSampleSender> m_udp_logging_sender;

        // log message list and log message serialization buffer
        Logging::SLogging                         m_log_msglist;
        std::vector<char>                         m_log_message_vec;

        std::string                               m_logfile_name;
        FILE*                                     m_logfile;

        SProviderAttributes                       m_attributes;
      
    };
  }
}