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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4834)
#endif
#include <asio.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  class CUDPReceiverAsio : public CUDPReceiverBase
  {
  public:
    CUDPReceiverAsio(const SReceiverAttr& attr_);

    // this virtual function is called during construction/destruction,
    // so, mark it as final to ensure that no derived classes override it.
    bool AddMultiCastGroup(const char* ipaddr_) final override;
    bool RemMultiCastGroup(const char* ipaddr_) override;

    size_t Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ = nullptr) override;

  protected:
    void RunIOContext(const asio::chrono::steady_clock::duration& timeout);

    bool                    m_created;
    bool                    m_broadcast;
    bool                    m_unicast;
    asio::io_context        m_iocontext;
    asio::ip::udp::socket   m_socket;
    asio::ip::udp::endpoint m_sender_endpoint;
  };
}