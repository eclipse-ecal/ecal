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

#include <ecal/ecal_qos.h>

#include <string>

namespace eCAL
{
  struct SWriterInfo
  {
    SWriterInfo() :
      has_mode_local(false),
      has_mode_cloud(false),
      has_qos_history_kind(false),
      has_qos_reliability(false),
      send_size_max(-1)
    {
    };

    std::string name;
    std::string description;

    bool has_mode_local;
    bool has_mode_cloud;

    bool has_qos_history_kind;
    bool has_qos_reliability;

    int  send_size_max;
  };
  
  class CDataWriterBase
  {
  public:
    CDataWriterBase() : m_created(false) {};
    virtual ~CDataWriterBase() {};

    virtual void GetInfo(SWriterInfo info_) = 0;

    virtual bool Create(const std::string& host_name_, const std::string& topic_name_, const std::string & topic_id_) = 0;
    virtual bool Destroy() = 0;

    virtual bool SetQOS(const QOS::SWriterQOS& qos_) { m_qos = qos_; return true; };
    QOS::SWriterQOS GetQOS() { return(m_qos); };

    virtual bool AddLocConnection(const std::string& /*process_id_*/, const std::string& /*conn_par_*/) { return false; };
    virtual bool RemLocConnection(const std::string& /*process_id_*/) { return false; };

    virtual bool AddExtConnection(const std::string& /*host_name_*/, const std::string& /*process_id_*/, const std::string& /*conn_par_*/) { return false; };
    virtual bool RemExtConnection(const std::string& /*host_name_*/, const std::string& /*process_id_*/) { return false; };

    virtual std::string GetConnectionParameter() { return ""; };

    struct SWriterData
    {
      const void*  buf       = nullptr;
      size_t       len       = 0;
      long long    id        = 0;
      long long    clock     = 0;
      size_t       hash      = 0;
      long long    time      = 0;
      size_t       buffering = 1;
      long         bandwidth = 0;
      bool         loopback  = false;
      bool         zero_copy = false;
    };

    virtual bool PrepareWrite(const SWriterData& /*data_*/) { return false; };
    virtual bool Write(const SWriterData& data_) = 0;

  protected:
    std::string      m_host_name;
    std::string      m_topic_name;
    std::string      m_topic_id;
    QOS::SWriterQOS  m_qos;

    bool             m_created;
  };
}
