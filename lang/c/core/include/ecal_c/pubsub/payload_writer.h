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

#ifndef ecal_c_pubsub_payload_writer_h_included
#define ecal_c_pubsub_payload_writer_h_included

#include <stdint.h>

typedef struct eCAL_PayloadWriter eCAL_PayloadWriter;

struct eCAL_PayloadWriter
{
  int (*WriteFull)(void*, size_t);
  int (*WriteModified)(void*, size_t);
  size_t(*GetSize)();
};

#endif /* ecal_c_pubsub_payload_writer_h_included */