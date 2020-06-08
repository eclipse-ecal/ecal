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
 * @brief  eCAL receiver base class
**/

#pragma once

#include <ecal/ecal.h>
#include "ecal_def.h"

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_socket.h"
#endif

#ifdef ECAL_OS_LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <stddef.h>

namespace eCAL
{
  ////////////////////////////////////////////////////////
  // receiver base class
  ////////////////////////////////////////////////////////
  struct SReceiverAttr
  {
    SReceiverAttr() :
      port(0),
      broadcast(false),
      unicast(false),
      loopback(true),
      rcvbuf(1024 * 1024)
    {};

    std::string ipaddr;
    int         port;
    bool        broadcast;
    bool        unicast;
    bool        loopback;
    int         rcvbuf;
  };

  class CReceiver
  {
  public:
    enum eSocketType
    {
      SType_Unknown = 0,
      SType_ReceiverUDP,
      SType_ReceiverTCP
    };

  public:
    CReceiver() : m_skt_type(SType_Unknown)
    {
    };

    explicit CReceiver(eSocketType skt_type_) : m_skt_type(skt_type_)
    {
    };

    virtual ~CReceiver()
    {
    };

    virtual bool Create(const SReceiverAttr& attr_) = 0;
    virtual bool Destroy() = 0;

    virtual size_t Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ = nullptr) = 0;

    inline eSocketType GetType() const {return m_skt_type;};

  protected:
    eSocketType m_skt_type;
  };
}
