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

#include <stddef.h>

namespace eCAL
{
  class payload
  {
  public:
    virtual ~payload() {};

    virtual void write_complete(void* buf_, size_t len_) = 0;
    virtual void write_partial (void* buf_, size_t len_) = 0;

    virtual const void* data() = 0;
    virtual size_t      size() = 0;
  };
};
