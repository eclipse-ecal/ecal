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
 * @file   common.cpp
 * @brief  C API common helper functions
**/

#include "common.h"

#include <ecal_c/types.h>

#include <cstdlib>
#include <cstring>

int CopyBuffer(void* target_, int target_len_, const std::string& source_s_)
{
  if(target_ == nullptr) return(0);
  if(source_s_.empty())  return(0);
  if(target_len_ == ECAL_ALLOCATE_4ME)
  {
    void* buf_alloc = malloc(source_s_.size()); // NOLINT(*-owning-memory, *-no-malloc)
    if(buf_alloc == nullptr) return(0);
    const int copied = CopyBuffer(buf_alloc, static_cast<int>(source_s_.size()), source_s_);
    if(copied > 0)
    {
     *((void**)target_) = buf_alloc; // NOLINT(*-pro-type-cstyle-cast)
     return(copied);
    }
    else
    {
      // copying buffer failed, so free allocated memory.
      free(buf_alloc); // NOLINT(*-owning-memory, *-no-malloc)
    }
  }
  else
  {
    if(target_len_ < static_cast<int>(source_s_.size())) return(0);
    memcpy(target_, source_s_.data(), source_s_.size());
    return(static_cast<int>(source_s_.size()));
  }
  return(0);
}
