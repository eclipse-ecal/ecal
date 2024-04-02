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
 * @brief  udp data writer
**/

#pragma once

#include "io/udp/ecal_udp_sample_sender.h"
#include "readwrite/ecal_writer_base.h"

#include <memory>
#include <string>
#include <vector>

namespace eCAL
{
  class CDataWriterUdpMC : public CDataWriterBase
  {
  public:
    ~CDataWriterUdpMC() override;

    SWriterInfo GetInfo() override;

    bool Create(const std::string& host_name_, const std::string& topic_name_, const std::string & topic_id_) override;
    // this virtual function is called during construction/destruction,
    // so, mark it as final to ensure that no derived classes override it.
    bool Destroy() final;

    bool Write(const void* buf_, const SWriterAttr& attr_) override;

  protected:
    std::vector<char>                   m_sample_buffer;
    std::shared_ptr<UDP::CSampleSender> m_sample_sender_loopback;
    std::shared_ptr<UDP::CSampleSender> m_sample_sender_no_loopback;
  };
}
