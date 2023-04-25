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
 * @file   ecal_payload_writer.h
 * @brief  eCAL payload writer base class
**/

#pragma once

#include <cstddef>
#include <cstring>

namespace eCAL
{
  // base payload writer class to allow zero copy memory operations
  // 
  // the `Write`and `Update` calls may operate on the target memory file directly (zero copy mode)
  class CPayloadWriter
  {
  public:
    CPayloadWriter() = default;
    virtual ~CPayloadWriter() = default;

    CPayloadWriter(const CPayloadWriter &) = default;
    CPayloadWriter(CPayloadWriter &&) = default;

    CPayloadWriter& operator=(const CPayloadWriter &) = default;
    CPayloadWriter& operator=(CPayloadWriter &&) = default;

    // the provisioned memory is uninitialized ->
    // perform a full write operation
    virtual bool Write(void* buffer_, size_t size_) = 0;

    // the provisioned memory is initialized and contains the data from the last write operation ->
    // perform a partial write operation or just modify a few bytes here
    //
    // by default this operation will just call `Write`
    virtual bool Update(void* buffer_, size_t size_) { return Write(buffer_, size_); };

    // provide the size of the required memory (eCAL needs to allocate for you).
    virtual size_t GetSize() = 0;
  };

  // payload writer class that wraps classic (void*, size_t) interface
  class CBufferPayloadWriter : public CPayloadWriter
  {
  public:
    CBufferPayloadWriter(const void* const buffer_, size_t size_) : m_buffer(buffer_), m_size(size_) {};

    // make a dump memory copy
    bool Write (void* buffer_, size_t size_) override {
      if (buffer_ == nullptr)  return false;
      if (size_ < m_size)      return false;
      if (m_buffer == nullptr) return false;
      if (m_size == 0)         return false;
      memcpy(buffer_, m_buffer, m_size);
      return true;
    }

    // size of the memory that needs to be copied
    size_t GetSize() override { return m_size; };
  
  private:
    const void* m_buffer = nullptr;
    size_t      m_size   = 0;
  };
}
