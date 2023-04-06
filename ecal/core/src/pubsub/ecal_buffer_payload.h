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
 * @file   ecal_buffer_payload.h
 * @brief  eCAL payload for primitive binary data
**/

#pragma once

#include <ecal/ecal_payload.h>

#include <cstddef>
#include <cstring>

namespace eCAL
{
  // specific payload class to wrap classic (void*, size_t) interface
  class CBufferPayload : public CPayload
  {
  public:
    CBufferPayload(const void* const buf_, size_t len_) : m_buf(buf_), m_buf_len(len_) {};

    // make a dump memory copy
    bool WriteComplete (void* buf_, size_t len_) {
      if (len_ < m_buf_len) return false;
      memcpy(buf_, m_buf, m_buf_len);
      return true;
    }

    // size of the memory that needs to be copied
    size_t GetSize() { return m_buf_len; };
  
  private:
    const void* const m_buf     = nullptr;
    size_t            m_buf_len = 0;
  };
};
