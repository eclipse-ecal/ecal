/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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
 * @brief  UDP logging receiver to receive messages of type eCAL::pb::LogMessage
**/

#pragma once

#include "io/udp/sendreceive/udp_receiver.h"
#include "util/ecal_thread.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <ecal/core/pb/logging.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  namespace UDP
  {
    class CLoggingReceiver
    {
    public:
      using LogMessageCallbackT = std::function<void(const eCAL::pb::LogMessage&)>;

      CLoggingReceiver(const IO::UDP::SReceiverAttr& attr_, LogMessageCallbackT log_message_callback_);
      virtual ~CLoggingReceiver();

    protected:
      void ReceiveThread();

      bool                             m_network_mode;

      LogMessageCallbackT              m_log_message_callback;

      IO::UDP::CUDPReceiver            m_udp_receiver;
      std::shared_ptr<CCallbackThread> m_udp_receiver_thread;

      std::vector<char>                m_msg_buffer;
      eCAL::pb::LogMessage             m_log_message;
    };
  }
}