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
 * @brief  lcm data writer
**/

#pragma once

#include <string>
#include <memory>

#include "io/udp_sender.h"
#include "lcm/lcmlite.h"
#include "readwrite/ecal_writer_base.h"

namespace eCAL
{
  class CDataWriterLCM : public CDataWriterBase
  {
  public:
    CDataWriterLCM();
    ~CDataWriterLCM();

    void GetInfo(SWriterInfo info_) override;

    bool Create(const std::string& host_name_, const std::string& topic_name_, const std::string & topic_id_) override;
    bool Destroy() override;

    size_t Send(const SWriterData& data_) override;

  protected:
    CUDPSender                      m_udp_sender;
    std::shared_ptr<lcmlite_pub_t>  m_lcm;
    bool                            m_initialized;
  };
}
