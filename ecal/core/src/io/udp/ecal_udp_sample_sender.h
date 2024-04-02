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
 * @brief  UDP sample sender to send messages of type eCAL::Sample
**/

#pragma once

#include "io/udp/sendreceive/udp_sender.h"

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace eCAL
{
  namespace UDP
  {
    class CSampleSender
    {
    public:
      explicit CSampleSender(const IO::UDP::SSenderAttr& attr_);
      size_t Send(const std::string& sample_name_, const std::vector<char>& serialized_sample_);

    private:
      IO::UDP::SSenderAttr                 m_attr;
      std::shared_ptr<IO::UDP::CUDPSender> m_udp_sender;

      std::mutex                           m_payload_mutex;
      std::vector<char>                    m_payload;
    };
  }
}
