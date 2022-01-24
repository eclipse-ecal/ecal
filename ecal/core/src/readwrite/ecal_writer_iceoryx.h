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
 * @brief  shared memory (iceoryx) writer
**/

#pragma once

#include "readwrite/ecal_writer_base.h"

#include <iceoryx_posh/popo/untyped_publisher.hpp>

#include <memory>
#include <string>

namespace eCAL
{
  // ecal shared memory (Iceoryx) writer
  class CDataWriterSHM : public CDataWriterBase
  {
  public:
    CDataWriterSHM();
    ~CDataWriterSHM();

    void GetInfo(SWriterInfo info_) override;

    bool Create(const std::string& host_name_, const std::string& topic_name_, const std::string & topic_id_) override;
    bool Destroy() override;

    bool Write(const SWriterData& data_) override;

  private:
    std::shared_ptr<iox::popo::UntypedPublisher> m_publisher;
  };
}
