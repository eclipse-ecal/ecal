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
 * @brief  eCAL common message format
**/

#pragma once

#include <stdint.h>

namespace eCAL
{
  struct SEcalMessage
  {
    SEcalMessage()
    {
      hdr_size  = sizeof(SEcalMessage);
      data_size = 0;
      id        = 0;
      clock     = 0;
      time      = 0;
      hash      = 0;
      ZeroMemory(reserved, 16);

    };
    uint16_t  hdr_size;
    uint64_t  data_size;
    uint64_t  id;
    uint64_t  clock;
    int64_t   time;
    uint64_t  hash;
    char reserverd[16];
  };
};
