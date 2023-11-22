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
 * @brief  eCAL payload writer class wrapping classic (buffer, size) payload
**/

#pragma once

#include <ecal/ecal_payload_writer.h>

#include <cstring>

namespace eCAL
{
  /**
   * @brief Payload writer class that wraps a classic (void*, size_t) interface.
   *
   * This class is a payload writer that wraps a classic interface using `void*` and `size_t`
   * arguments. It inherits from the base class CPayloadWriter, allowing zero-copy memory
   * operations.
   */
  class CBufferPayloadWriter : public CPayloadWriter
  {
  public:
    /**
     * @brief Constructor for CBufferPayloadWriter.
     *
     * @param buffer_ Pointer to the buffer containing the data to be written.
     * @param size_   Size of the data to be written.
     */
    CBufferPayloadWriter(const void* const buffer_, size_t size_) : m_buffer(buffer_), m_size(size_) {};

    /**
     * @brief Make a dump memory copy of the stored buffer.
     *
     * This function performs a dump memory copy of the stored buffer to the provided
     * memory location (buffer_) with the specified size (size_). The size of the provided
     * memory buffer should be equal to or greater than the stored buffer size to avoid
     * memory corruption.
     *
     * @param buffer_  Pointer to the target buffer where the data will be copied.
     * @param size_    Size of the target buffer.
     *
     * @return True if the copy operation is successful, false otherwise.
     */
    bool WriteFull(void* buffer_, size_t size_) override
    {
      if (buffer_ == nullptr)  return false;
      if (size_ < m_size)      return false;
      if (m_buffer == nullptr) return false;
      if (m_size == 0)         return false;
      memcpy(buffer_, m_buffer, m_size);
      return true;
    }

    /**
     * @brief Get the size of the memory that needs to be copied.
     *
     * This function returns the size of the memory buffer that needs to be copied during
     * the write operation. It is used by the base class CPayloadWriter to allocate the
     * required memory for eCAL.
     *
     * @return The size of the memory that needs to be copied.
     */
    size_t GetSize() override { return m_size; };

  private:
    const void* m_buffer = nullptr;  ///< Pointer to the buffer containing the data to be written.
    size_t      m_size   = 0;        ///< Size of the data to be written.
  };

} // namespace eCAL
