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

#include "ecal_log_receiver.h"

#include "config/builder/udp_attribute_builder.h"
#include "serialization/ecal_serialize_logging.h"

#include <iostream>

namespace
{
  void logWarningToConsole(const std::string& msg_)
  {
    std::cout << "[eCAL][Logging-Receiver][Warning] " << msg_ << "\n";
  }
}

namespace eCAL
{
  namespace Logging
  {
    CLogReceiver::CLogReceiver(const SReceiverAttributes& attr_)
    : m_created(false)
    , m_attributes(attr_)
    {
    }

    CLogReceiver::~CLogReceiver()
    {
      Stop();
    }

    void CLogReceiver::Start()
    {
      if (m_attributes.receive_enabled)
      {
        // set logging receive network attributes
        const eCAL::UDP::SReceiverAttr attr = Logging::UDP::ConvertToIOUDPReceiverAttributes(m_attributes.udp_receiver);

        // start logging receiver
        m_log_receiver = std::make_shared<eCAL::UDP::CSampleReceiver>(attr, std::bind(&CLogReceiver::HasSample, this, std::placeholders::_1), std::bind(&CLogReceiver::ApplySample, this, std::placeholders::_1, std::placeholders::_2));

        if(m_log_receiver == nullptr)
        {
          logWarningToConsole("Logging receiver could not be created.");
        }
      }

      m_created = true;
    }

    void CLogReceiver::Stop()
    {
      if(!m_created) return;

      m_log_receiver.reset();

      m_created = false;
    }

    bool CLogReceiver::GetLogging(std::string& log_msg_list_string_)
    {
      if (!m_attributes.receive_enabled) return false;
      // clear target list string
      log_msg_list_string_.clear();

      // serialize message list
      {
        const std::lock_guard<std::mutex> lock(m_log_mtx);
        // serialize message list to target list string
        SerializeToBuffer(m_log_msglist, log_msg_list_string_);
        // clear message list
        m_log_msglist.log_messages.clear();
      }

      return true;
    }

    void CLogReceiver::GetLogging(Logging::SLogging& log_)
    {
      // copy message list
      {
        const std::lock_guard<std::mutex> lock(m_log_mtx);
        log_ = m_log_msglist;
        // clear message list
        m_log_msglist.log_messages.clear();
      }
    }

    bool CLogReceiver::HasSample(const std::string& sample_name_)
    {
      return (sample_name_ == "_log_message_");
    }

    bool CLogReceiver::ApplySample(const char* serialized_sample_data_, size_t serialized_sample_size_)
    {
      // TODO: Limit maximum size of collected log messages !
      Logging::SLogMessage log_message;
      if (DeserializeFromBuffer(serialized_sample_data_, serialized_sample_size_, log_message))
      {
        // in "network mode" we accept all log messages
        // in "local mode" we accept log messages from this host only
        if ((m_attributes.host_name == log_message.host_name) || m_attributes.network_enabled)
        {
          const std::lock_guard<std::mutex> lock(m_log_mtx);
          m_log_msglist.log_messages.emplace_back(log_message);
        }
        return true;
      }
      return false;
    }
  }
}