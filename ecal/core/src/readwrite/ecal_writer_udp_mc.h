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

#include "ecal_def.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/ecal.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "io/udp_sender.h"
#include "readwrite/ecal_writer_base.h"

#include <string>

namespace eCAL
{
  class CDataWriterUdpMC : public CDataWriterBase
  {
  public:
    ~CDataWriterUdpMC();

    void GetInfo(SWriterInfo info_) override;

    bool Create(const std::string& host_name_, const std::string& topic_name_, const std::string & topic_id_) override;
    bool Destroy() override;

    bool Write(const SWriterData& data_) override;

  protected:
    std::string     m_udp_ipaddr;
    eCAL::pb::Sample  m_ecal_sample;

    CUDPSender      m_sample_snd_loopback;
    CUDPSender      m_sample_snd_no_loopback;
  };
}
