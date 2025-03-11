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

#ifndef ecal_c_config_publisher_h_included
#define ecal_c_config_publisher_h_included

#include <stdint.h>

enum eCAL_TransportLayer_eType
{
  eCAL_TransportLayer_eType_none,
  eCAL_TransportLayer_eType_udp_mc,
  eCAL_TransportLayer_eType_shm,
  eCAL_TransportLayer_eType_tcp,
};

struct eCAL_Publisher_Layer_SHM_Configuration
{
  int enable;
  int zero_copy_mode;
  unsigned int acknowledge_timeout_ms;
  unsigned int memfile_buffer_count;
  unsigned int memfile_min_size_bytes;
  unsigned int memfile_reserve_percent;
};

struct eCAL_Publisher_Layer_UDP_Configuration
{
  int enable;
};

struct eCAL_Publisher_Layer_TCP_Configuration
{
  int enable;
};

struct eCAL_Publisher_Layer_Configuration
{
  struct eCAL_Publisher_Layer_SHM_Configuration shm;
  struct eCAL_Publisher_Layer_UDP_Configuration udp;
  struct eCAL_Publisher_Layer_TCP_Configuration tcp;
};

struct eCAL_Publisher_Configuration
{
  struct eCAL_Publisher_Layer_Configuration layer;

  enum eCAL_TransportLayer_eType* layer_priority_local;
  size_t layer_priority_local_length;
  enum eCAL_TransportLayer_eType* layer_priority_remote;
  size_t layer_priority_remote_length;
};

#endif /*ecal_c_config_publisher_h_included*/