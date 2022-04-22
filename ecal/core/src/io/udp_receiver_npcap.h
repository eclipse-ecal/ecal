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


#pragma once

#include <io/udp_receiver_base.h>

#include "ecal_config_reader_hlp.h"
#include <udpcap/npcap_helpers.h>
#include <udpcap/udpcap_socket.h>

namespace eCAL
{
  ////////////////////////////////////////////////////////
  // Npcap based receiver class implementation
  ////////////////////////////////////////////////////////
  class CUDPReceiverPcap : public CUDPReceiverBase
  {
  public:
    CUDPReceiverPcap(const SReceiverAttr& attr_);

    bool AddMultiCastGroup(const char* ipaddr_) override;
    bool RemMultiCastGroup(const char* ipaddr_) override;

    size_t Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ = nullptr) override;

  protected:
    bool                 m_created;
    bool                 m_unicast;
    Udpcap::UdpcapSocket m_socket;
  };

}