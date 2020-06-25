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
 * @brief  Global database class
**/

#include <ecal/ecal.h>

#include "ecal_def.h"
#include "io/msg_type.h"

#include "ecal_config_hlp.h"
#include "ecal_monitoring_threads.h"
#include "ecal_global_accessors.h"

namespace eCAL
{
  static bool IsLocalHost(const eCAL::pb::LogMessage& ecal_message_)
  {
    std::string host_name = ecal_message_.hname();
    if (host_name.empty())                   return(false);
    if (host_name == Process::GetHostName()) return(true);
    return(false);
  }

  CRegistrationReceiveThread::CRegistrationReceiveThread(RegMessageCallbackT reg_cb_) :
    m_reg_cb(reg_cb_)
  {
    SReceiverAttr attr;
    bool local_only = !eCALPAR(NET, ENABLED);
    // for local only communication we switch to local broadcasting to bypass vpn's or firewalls
    if (local_only)
    {
      attr.ipaddr    = "127.255.255.255";
      attr.broadcast = true;
    }
    else
    {
      attr.ipaddr    = eCALPAR(NET, UDP_MULTICAST_GROUP);
      attr.broadcast = false;
    }
    attr.port     = eCALPAR(NET, UDP_MULTICAST_PORT) + NET_UDP_MULTICAST_PORT_REG_OFF;
    attr.loopback = true;
    attr.rcvbuf   = eCALPAR(NET, UDP_MULTICAST_RCVBUF);

    m_reg_rcv.Create(attr);
    m_reg_rcv_thread.Start(0, std::bind(&CRegistrationReceiveThread::ThreadFun, this));
  }

  CRegistrationReceiveThread::~CRegistrationReceiveThread()
  {
    m_reg_rcv_thread.Stop();
    m_reg_rcv.Destroy();
  }

  int CRegistrationReceiveThread::ThreadFun()
  {
    m_reg_cb(&m_reg_rcv);
    return 0;
  }

  CLoggingReceiveThread::CLoggingReceiveThread(LogMessageCallbackT log_cb_) :
    m_network_mode(false), m_log_cb(log_cb_)
  {
    SReceiverAttr attr;
    bool local_only = !eCALPAR(NET, ENABLED);
    // for local only communication we switch to local broadcasting to bypass vpn's or firewalls
    if (local_only)
    {
      attr.ipaddr    = "127.255.255.255";
      attr.broadcast = true;
    }
    else
    {
      attr.ipaddr    = eCALPAR(NET, UDP_MULTICAST_GROUP);
      attr.broadcast = false;
    }
    attr.port     = eCALPAR(NET, UDP_MULTICAST_PORT) + NET_UDP_MULTICAST_PORT_LOG_OFF;
    attr.loopback = true;
    attr.rcvbuf   = eCALPAR(NET, UDP_MULTICAST_RCVBUF);

    m_log_rcv.Create(attr);
    m_log_rcv_thread.Start(0, std::bind(&CLoggingReceiveThread::ThreadFun, this));
    m_msg_buffer.resize(MSG_BUFFER_SIZE);
  }

  CLoggingReceiveThread::~CLoggingReceiveThread()
  {
    m_log_rcv_thread.Stop();
    m_log_rcv.Destroy();
  }

  void CLoggingReceiveThread::SetNetworkMode(bool network_mode_)
  {
    m_network_mode = network_mode_;
  }

  int CLoggingReceiveThread::ThreadFun()
  {
    // wait for any incoming message
    size_t recv_len = m_log_rcv.Receive(m_msg_buffer.data(), m_msg_buffer.size(), 10);
    if (recv_len > 0)
    {
      m_log_ecal_msg.Clear();
      if (m_log_ecal_msg.ParseFromArray(m_msg_buffer.data(), static_cast<int>(recv_len)))
      {
        if (IsLocalHost(m_log_ecal_msg) || m_network_mode)
        {
          m_log_cb(m_log_ecal_msg);
        }
      }
    }

    return(0);
  };

  CMonLogPublishingThread::CMonLogPublishingThread(MonitoringCallbackT mon_cb_, LoggingCallbackT log_cb_) :
    m_mon_cb(mon_cb_), m_log_cb(log_cb_)
  {
    m_pub_thread.Start(CMN_REGISTRATION_REFRESH, std::bind(&CMonLogPublishingThread::ThreadFun, this));
  };

  CMonLogPublishingThread::~CMonLogPublishingThread()
  {
    m_pub_thread.Stop();
    m_mon_pub.pub.Destroy();
    m_log_pub.pub.Destroy();
  };

  void CMonLogPublishingThread::SetMonState(bool state_, const std::string& name_)
  {
    m_mon_pub.state = state_;
    m_mon_pub.name  = name_;
    if (state_)
    {
      m_mon_pub.pub.Create(name_);
    }
    else
    {
      m_mon_pub.pub.Destroy();
    }
  }

  void CMonLogPublishingThread::SetLogState(bool state_, const std::string& name_)
  {
    m_log_pub.state = state_;
    m_log_pub.name  = name_;
    if (state_)
    {
      m_log_pub.pub.Create(name_);
    }
    else
    {
      m_log_pub.pub.Destroy();
    }
  }

  int CMonLogPublishingThread::ThreadFun()
  {
    if (m_mon_pub.state)
    {
      // get monitoring
      eCAL::pb::Monitoring monitoring;
      m_mon_cb(monitoring);
      // publish monitoring
      m_mon_pub.pub.Send(monitoring);
    }

    if (m_log_pub.state)
    {
      // get logging
      eCAL::pb::Logging logging;
      m_log_cb(logging);
      // publish logging
      m_log_pub.pub.Send(logging);
    }
    return 0;
  }  
}
