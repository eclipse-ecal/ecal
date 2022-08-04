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

#include "udpcap/udpcap_socket.h"

#include "udpcap_socket_private.h"

namespace Udpcap
{
  UdpcapSocket::UdpcapSocket()
    : udpcap_socket_private_(std::make_unique<Udpcap::UdpcapSocketPrivate>())
  {}

  UdpcapSocket::~UdpcapSocket()
                                                                                                                     {}

  bool              UdpcapSocket::isValid                    () const                                                { return udpcap_socket_private_->isValid(); }

  bool              UdpcapSocket::bind                       (const HostAddress& local_address, uint16_t local_port) { return udpcap_socket_private_->bind(local_address, local_port); }

  bool              UdpcapSocket::isBound                    () const                                                { return udpcap_socket_private_->isBound(); }
  HostAddress       UdpcapSocket::localAddress               () const                                                { return udpcap_socket_private_->localAddress(); }
  uint16_t          UdpcapSocket::localPort                  () const                                                { return udpcap_socket_private_->localPort(); }

  bool              UdpcapSocket::setReceiveBufferSize       (int receive_buffer_size)                               { return udpcap_socket_private_->setReceiveBufferSize(receive_buffer_size); }

  bool              UdpcapSocket::hasPendingDatagrams        () const                                                { return udpcap_socket_private_->hasPendingDatagrams(); }

  std::vector<char> UdpcapSocket::receiveDatagram            (HostAddress* source_address, uint16_t* source_port)                                                       { return udpcap_socket_private_->receiveDatagram(source_address, source_port); }
  std::vector<char> UdpcapSocket::receiveDatagram            (unsigned long timeout_ms, HostAddress* source_address, uint16_t* source_port)                             { return udpcap_socket_private_->receiveDatagram(timeout_ms, source_address, source_port); }
  size_t            UdpcapSocket::receiveDatagram            (char* data, size_t max_len, HostAddress* source_address, uint16_t* source_port)                           { return udpcap_socket_private_->receiveDatagram(data, max_len, source_address, source_port); }
  size_t            UdpcapSocket::receiveDatagram            (char* data, size_t max_len, unsigned long timeout_ms, HostAddress* source_address, uint16_t* source_port) { return udpcap_socket_private_->receiveDatagram(data, max_len, timeout_ms, source_address, source_port); }

  bool              UdpcapSocket::joinMulticastGroup         (const HostAddress& group_address)                      { return udpcap_socket_private_->joinMulticastGroup(group_address); }
  bool              UdpcapSocket::leaveMulticastGroup        (const HostAddress& group_address)                      { return udpcap_socket_private_->leaveMulticastGroup(group_address); }

  void              UdpcapSocket::setMulticastLoopbackEnabled(bool enabled)                                          { udpcap_socket_private_->setMulticastLoopbackEnabled(enabled); }
  bool              UdpcapSocket::isMulticastLoopbackEnabled () const                                                { return udpcap_socket_private_->isMulticastLoopbackEnabled(); }

  void              UdpcapSocket::close                      ()                                                      { udpcap_socket_private_->close(); }

}