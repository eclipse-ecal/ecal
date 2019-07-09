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
 * @brief  Common message type for eCAL samples
**/

#pragma once

#include <stdint.h>

enum eUDPMessageType
{
  msg_type_unknown             = 0,
  msg_type_header              = 1,
  msg_type_content             = 2,
  msg_type_header_with_content = 3
};

struct alignas(4) SUDPMessageHead
{
  SUDPMessageHead()
  {
    head[0] = 'E';
    head[1] = 'C';
    head[2] = 'A';
    head[3] = 'L';
    version = 5;
    type    = msg_type_unknown;
    id      = 0;
    num     = 0;
    len     = 0;
  }

  char     head[4];   //-V112
  int32_t  version;
  int32_t  type;
  int32_t  id;        // unique id for all message parts
  int32_t  num;       // header: number of all parts,      data: current number of that part
  int32_t  len;       // header: complete size of message, data: current size of that part
};

#define MSG_BUFFER_SIZE   (64*1024 - 20 /* IP header */ - 8 /* UDP header */ - 1 /* don't ask */)
#define MSG_PAYLOAD_SIZE  (MSG_BUFFER_SIZE-sizeof(struct SUDPMessageHead))
struct SUDPMessage
{
  struct SUDPMessageHead header;
  char                   payload[MSG_PAYLOAD_SIZE];
};
