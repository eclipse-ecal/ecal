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
 * @brief  UDP receiver class
**/

#pragma once

#include <memory>
#include "ecal_receiver.h"

namespace eCAL
{
  class CUDPReceiverImpl;
  class CUDPcapReceiverImpl;

  class CUDPReceiver : public CReceiver
  {
  public:
    CUDPReceiver();

    bool Create(const SReceiverAttr& attr_);
    bool Destroy();

    bool AddMultiCastGroup(const char* ipaddr_);
    bool RemMultiCastGroup(const char* ipaddr_);

    size_t Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ = nullptr);

  protected:
    bool m_use_npcap;
    std::shared_ptr<CUDPReceiverImpl>    m_socket_impl;
    std::shared_ptr<CUDPcapReceiverImpl> m_udpcap_socket_impl;
  };
}
