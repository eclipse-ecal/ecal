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
 * @file   ecal_payload.h
 * @brief  eCAL payload
**/

#pragma once

#include <cstddef>

namespace eCAL
{
  // base payload class to allow zero memory operation
  // 
  // the `WriteComplete`and `WritePartial` calls perform 
  // write/copy operations on the target memory directly
  class CPayload
  {
  public:
    //CPayload(const CPayload &) = delete;
    //CPayload(CPayload &&) noexcept = default;

    //CPayload& operator=(const CPayload &) = delete;
    //CPayload& operator=(CPayload &&) noexcept = default;

    virtual ~CPayload() = default;

    // the provisioned memory is uninitialized ->
    // perform a full write operation
    virtual bool WriteComplete (void* buf_, size_t len_) = 0;

    // the provisioned memory is initialized and contains the data from the last write operation ->
    // perform a partial write operation or just modify a few bytes here
    //
    // by default this operation will just call `WriteComplete`
    virtual bool WritePartial  (void* buf_, size_t len_) { return WriteComplete(buf_, len_); };

    // provide the size of the required memory (eCAL needs to allocate for you).
    virtual size_t GetSize() = 0;
  };
}
