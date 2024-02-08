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
 * @brief  UDP sample receiver to receive messages of type eCAL::Sample
**/

#pragma once

#include "io/udp/sendreceive/udp_receiver.h"
#include "io/udp/fragmentation/rcv_fragments.h"
#include "util/ecal_thread.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace eCAL
{
  namespace UDP
  {
    class CSampleReceiver
    {
    public:
      using HasSampleCallbackT   = std::function<bool(const std::string& sample_name_)>;
      using ApplySampleCallbackT = std::function<void(const char* serialized_sample_data_, size_t serialized_sample_size_)>;

      CSampleReceiver(const IO::UDP::SReceiverAttr& attr_, HasSampleCallbackT has_sample_callback_, ApplySampleCallbackT apply_sample_callback_);
      virtual ~CSampleReceiver();

      bool AddMultiCastGroup(const char* ipaddr_);
      bool RemMultiCastGroup(const char* ipaddr_);

    protected:
      void ReceiveThread();
      void Process(const char* sample_buffer_, size_t sample_buffer_len_);

      HasSampleCallbackT                      m_has_sample_callback;
      ApplySampleCallbackT                    m_apply_sample_callback;

      IO::UDP::CUDPReceiver                   m_udp_receiver;
      std::shared_ptr<eCAL::CCallbackThread>  m_udp_receiver_thread;

      std::vector<char>                       m_msg_buffer;

      std::chrono::steady_clock::time_point   m_cleanup_start;

      class CSampleDefragmentation : public IO::UDP::CMsgDefragmentation
      {
      public:
        explicit CSampleDefragmentation(CSampleReceiver* sample_receiver_);
        ~CSampleDefragmentation() override;

        int OnMessageCompleted(std::vector<char>&& msg_buffer_) override;

      protected:
        CSampleReceiver* m_sample_receiver;
      };

      using SampleDefragmentationMapT = std::unordered_map<int32_t, std::shared_ptr<CSampleDefragmentation>>;
      SampleDefragmentationMapT m_defrag_sample_map;
    };
  }
}
