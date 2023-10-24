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
 * @brief  data writer base class
**/

#pragma once

#include <ecal/ecal_payload_writer.h>
#include <ecal/ecal_qos.h>

#include "ecal_writer_data.h"
#include "ecal_writer_info.h"

#include <atomic>
#include <string>

namespace eCAL
{
  class CDataWriterBase
  {
  public:
    CDataWriterBase() : m_created(false) {};
    virtual ~CDataWriterBase() = default;

    virtual SWriterInfo GetInfo() = 0;

    virtual bool Create(const std::string& host_name_, const std::string& topic_name_, const std::string & topic_id_) = 0;
    virtual bool Destroy() = 0;

    virtual bool SetQOS(const QOS::SWriterQOS& qos_) { m_qos = qos_; return true; };
    QOS::SWriterQOS GetQOS() { return(m_qos); };

    virtual void AddLocConnection(const std::string& /*process_id_*/, const std::string& /*topic_id_*/, const std::string& /*conn_par_*/) {};
    virtual void RemLocConnection(const std::string& /*process_id_*/, const std::string& /*topic_id_*/) {};

    virtual void AddExtConnection(const std::string& /*host_name_*/, const std::string& /*process_id_*/, const std::string& /*topic_id_*/, const std::string& /*conn_par_*/) {};
    virtual void RemExtConnection(const std::string& /*host_name_*/, const std::string& /*process_id_*/, const std::string& /*topic_id_*/) {};

    virtual std::string GetConnectionParameter() { return ""; };

    virtual bool PrepareWrite(const SWriterAttr& /*attr_*/) { return false; };
    virtual bool Write(CPayloadWriter& /*payload_*/, const SWriterAttr& /*attr_*/) { return false; };
    virtual bool Write(const void* /*buf_*/, const SWriterAttr& /*attr_*/) { return false; };

  protected:
    std::string        m_host_name;
    std::string        m_topic_name;
    std::string        m_topic_id;
    QOS::SWriterQOS    m_qos;

    std::atomic<bool>  m_created;
  };
}
