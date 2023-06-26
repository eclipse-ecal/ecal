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
 * @brief  UDP sender class
**/

#pragma once

#include <memory>
#include <string>

namespace eCAL
{
  struct SSenderAttr
  {
    std::string ipaddr;
    int         port      = 0;
    int         ttl       = 0;
    bool        broadcast = false;
    bool        unicast   = false;
    bool        loopback  = true;
    int         sndbuf    = 1024 * 1024;
  };

  class CUDPSenderImpl;

  class CUDPSender
  {
  public:
    CUDPSender(const SSenderAttr& attr_);

    size_t Send     (const void* buf_, size_t len_, const char* ipaddr_ = nullptr);
    void   SendAsync(const void* buf_, size_t len_, const char* ipaddr_ = nullptr);

  protected:
    std::shared_ptr<CUDPSenderImpl> m_socket_impl;
  };
}
