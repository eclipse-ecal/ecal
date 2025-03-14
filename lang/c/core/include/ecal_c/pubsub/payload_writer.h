/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
 * @file   pubsub/payload_writer.h
 * @brief  eCAL payload writer c interface
**/

#ifndef ecal_c_pubsub_payload_writer_h_included
#define ecal_c_pubsub_payload_writer_h_included

#include <stddef.h>

typedef struct eCAL_PayloadWriter eCAL_PayloadWriter;

/**
   * @brief Payload writer interface to allow zero copy memory operations.
   *
   * This interface struct can be used for enabling  zero-copy memory
   * operations. The `WriteFull` and `WriteModified` calls may operate on the target
   * memory file directly in zero-copy mode.
   *
   * A partial writing / modification of the memory file is only possible when zero-copy mode
   * is activated. If zero-copy is not enabled, the `WriteModified` method is ignored and the
   * `WriteFull` method is always executed.
   *
  **/
struct eCAL_PayloadWriter
{
  /**
     * @brief Perform a full write operation on uninitialized memory.
     *
     * This function pointer must be set to perform a full write operation
     * when the provisioned memory is uninitialized. Typically, this is the case when a
     * memory file had to be recreated or its size had to be changed.
     *
     * @param buffer_ Pointer to the buffer containing the data to be written.
     * @param size_   Size of the data to be written.
     *
     * @return zero if the write operation is successful, non-zero otherwise.
    **/
  int (*WriteFull)(void*, size_t);

  /**
     * @brief Perform a partial write operation to modify existing data.
     *
     * This function pointer can be set to modify existing data when the provisioned
     * memory is already initialized by a WriteFull call (i.e. contains the data from that full write operation).
     *
     * The memory can be partially modified and does not have to be completely rewritten, which leads to significantly
     * higher performance (lower latency).
     *
     * If not implemented (by default), this operation will just call the `WriteFull` function.
     *
     * @param buffer_ Pointer to the buffer containing the data to be modified.
     * @param size_   Size of the data to be modified.
     *
     * @return Zero if the write/update operation is successful, non-zero otherwise.
    **/
  int (*WriteModified)(void*, size_t);

  /**
     * @brief Get the size of the required memory.
     *
     * This function pointer must be set to provide the size of the memory
     * that eCAL needs to allocate.
     *
     * @return The size of the required memory.
    **/
  size_t(*GetSize)();
};

#endif /* ecal_c_pubsub_payload_writer_h_included */