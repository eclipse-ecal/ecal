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
 * @brief  UDP sample receiver base class
**/

#pragma once

#include "io/udp/ecal_udp_receiver_attr.h"

namespace eCAL
{
  namespace UDP
  {
    class CSampleReceiverBase
    {
    public:
      virtual ~CSampleReceiverBase() = default;

      virtual bool AddMultiCastGroup(const char* ipaddr_) = 0;
      virtual bool RemMultiCastGroup(const char* ipaddr_) = 0;

      // prevent copying and moving
      CSampleReceiverBase(const CSampleReceiverBase&) = delete;
      CSampleReceiverBase& operator=(const CSampleReceiverBase&) = delete;
      CSampleReceiverBase(CSampleReceiverBase&&) = delete;
      CSampleReceiverBase& operator=(CSampleReceiverBase&&) = delete;

    protected:
      CSampleReceiverBase(const SReceiverAttr& attr_, const HasSampleCallbackT& has_sample_callback_, const ApplySampleCallbackT& apply_sample_callback_)
        : m_has_sample_callback(has_sample_callback_), m_apply_sample_callback(apply_sample_callback_), m_broadcast(attr_.broadcast)
      {
      }

      HasSampleCallbackT   m_has_sample_callback;
      ApplySampleCallbackT m_apply_sample_callback;
      bool                 m_broadcast = false;
    };
  }
}
