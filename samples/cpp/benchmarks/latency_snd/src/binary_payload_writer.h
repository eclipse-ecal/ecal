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

#include <ecal/ecal_payload_writer.h>
#include <cstring>

// a binary payload
class CBinaryPayload : public eCAL::CPayloadWriter
{
public:
  CBinaryPayload(size_t size_) : size(size_) {}

  bool WriteFull(void* buf_, size_t len_) override
  {
    // write complete content to the shared memory file
    if (len_ < size) return false;
    memset(buf_, 42, size);
    return true;
  };

  bool WriteModified(void* buf_, size_t len_) override
  {
    // update content of the shared memory file
    if (len_ < size) return false;
    const size_t write_idx((clock % 1024) % len_);
    const char write_chr(clock % 10 + 48);
    static_cast<char*>(buf_)[write_idx] = write_chr;
    clock++;
    return true;
  };

  size_t GetSize() override { return size; };

private:
  size_t size  = 0;
  int    clock = 0;
};
