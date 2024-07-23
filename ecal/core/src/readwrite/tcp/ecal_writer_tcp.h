/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  tcp writer
**/

#pragma once

#include <ecal/config/publisher.h>

#include "readwrite/ecal_writer_base.h"

#include <tcp_pubsub/executor.h>
#include <tcp_pubsub/publisher.h>

#include <mutex>
#include <string>
#include <vector>

namespace eCAL
{
  // ecal tcp writer
  class CDataWriterTCP : public CDataWriterBase
  {
  public:
    CDataWriterTCP(const std::string& host_name_, const std::string& topic_name_, const std::string& topic_id_, const Publisher::Layer::TCP::Configuration& tcp_config_);

    SWriterInfo GetInfo() override;

    bool Write(const void* buf_, const SWriterAttr& attr_) override;

    Registration::ConnectionPar GetConnectionParameter() override;

  private:
    Publisher::Layer::TCP::Configuration         m_config;

    std::vector<char>                            m_header_buffer;

    static std::mutex                            g_tcp_writer_executor_mtx;
    static std::shared_ptr<tcp_pubsub::Executor> g_tcp_writer_executor;

    std::shared_ptr<tcp_pubsub::Publisher>       m_publisher;
    uint16_t                                     m_port = 0;
  };
}
