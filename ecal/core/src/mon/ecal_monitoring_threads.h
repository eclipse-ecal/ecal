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
 * @brief  Monitoring worker threads
**/

#pragma once

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>

#include "ecal_thread.h"
#include "io/udp_receiver.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <functional>
#include <string>

namespace eCAL
{
  class CRegistrationReceiveThread
  {
  public:
    using RegMessageCallbackT = std::function<int(eCAL::CUDPReceiver* sample_receiver_)>;

    CRegistrationReceiveThread(RegMessageCallbackT reg_cb_);
    virtual ~CRegistrationReceiveThread();

  protected:
    int ThreadFun();

    CUDPReceiver         m_reg_rcv;
    class CThread        m_reg_rcv_thread;
    RegMessageCallbackT  m_reg_cb;
  };

  class CLoggingReceiveThread
  {
  public:
    using LogMessageCallbackT = std::function<void(const eCAL::pb::LogMessage&)>;

    CLoggingReceiveThread(LogMessageCallbackT log_cb_);
    virtual ~CLoggingReceiveThread();

    void SetNetworkMode(bool network_mode_);
  
  protected:
    int ThreadFun();

    CUDPReceiver         m_log_rcv;
    class CThread        m_log_rcv_thread;

    bool                 m_network_mode;
    std::vector<char>    m_msg_buffer;
    eCAL::pb::LogMessage   m_log_ecal_msg;
    LogMessageCallbackT  m_log_cb;
  };

  class CMonLogPublishingThread
  {
  public:
    using MonitoringCallbackT = std::function<void(eCAL::pb::Monitoring&)>;
    using LoggingCallbackT    = std::function<void(eCAL::pb::Logging&)>;

    CMonLogPublishingThread(MonitoringCallbackT mon_cb_, LoggingCallbackT log_cb_);
    virtual ~CMonLogPublishingThread();

    void SetMonState(bool state_, const std::string& name_);
    void SetLogState(bool state_, const std::string& name_);

  protected:
    int ThreadFun();

    template <typename T>
    struct SProtoPub
    {
      SProtoPub() : state(false) {}
      bool                state;
      std::string         name;

      protobuf::CPublisher<T>  pub;
    };

    class CThread                         m_pub_thread;
    struct SProtoPub<eCAL::pb::Monitoring>  m_mon_pub;
    struct SProtoPub<eCAL::pb::Logging>     m_log_pub;

    MonitoringCallbackT                   m_mon_cb;
    LoggingCallbackT                      m_log_cb;
  };
}
