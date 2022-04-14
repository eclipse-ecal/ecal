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

#include <io/udp_receiver.h>

namespace eCAL
{
  ////////////////////////////////////////////////////////
  // udp receiver class implementation
  ////////////////////////////////////////////////////////
  class CUDPReceiverBase
  {
  public:
    CUDPReceiverBase(const SReceiverAttr& /*attr_*/) {};
    // We don't technically need a virtual destructor, if we are working with shared_ptrs...
    virtual ~CUDPReceiverBase() = default;
    // Delete copy / move operations to prevent slicing
    CUDPReceiverBase(CUDPReceiverBase&&) = delete;
    CUDPReceiverBase& operator=(CUDPReceiverBase&&) = delete;
    CUDPReceiverBase(const CUDPReceiverBase&) = delete;
    CUDPReceiverBase& operator=(const CUDPReceiverBase&) = delete;

    virtual bool AddMultiCastGroup(const char* ipaddr_) = 0;
    virtual bool RemMultiCastGroup(const char* ipaddr_) = 0;

    virtual size_t Receive(char* buf_, size_t len_, int timeout_, ::sockaddr_in* address_ = nullptr) = 0;
  };
}