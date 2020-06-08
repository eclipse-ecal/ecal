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
 * @brief  eCAL sender base class
**/

#pragma once

#include <stddef.h>

namespace eCAL
{
  ////////////////////////////////////////////////////////
  // sender base class
  ////////////////////////////////////////////////////////
  struct SSenderAttr
  {
    SSenderAttr() :
      port(0),
      ttl(0),
      broadcast(false),
      unicast(false),
      loopback(true),
      sndbuf(1024 * 1024)
    {};

    std::string ipaddr;
    int         port;
    int         ttl;
    bool        broadcast;
    bool        unicast;
    bool        loopback;
    int         sndbuf;
  };

  class CSender
  {
  public:
    enum eSocketType
    {
      SType_Unknown = 0,
      SType_SenderUDP,
    };

    CSender() : m_skt_type(SType_Unknown)
    {
    };

    explicit CSender(eSocketType skt_type_) : m_skt_type(skt_type_)
    {
    };

    virtual ~CSender()
    {
    };

    virtual bool Create(const SSenderAttr& attr_) = 0;
    virtual bool Destroy() = 0;

    virtual size_t Send(const void* buf_, const size_t len_, const char* ipaddr_ = nullptr) = 0;

    inline eSocketType GetType() const {return m_skt_type;};

  protected:
    eSocketType m_skt_type;
  };
}
