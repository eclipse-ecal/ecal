/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

#ifndef ecal_c_config_registration_h_included
#define ecal_c_config_registration_h_included

#include <stdint.h>

enum eCAL_Registration_Local_eTransportType
{
  eCAL_Registration_Local_eTransportType_shm,
  eCAL_Registration_Local_eTransportType_udp
};

struct eCAL_Registration_Local_SHM_Configuration
{
  const char* domain; //!< Domain name for shared memory based registration (Default: ecal_mon)
  size_t queue_size; //!< Queue size of registration events (Default: 1024)
};

struct eCAL_Registration_Local_UDP_Configuration
{
  unsigned int port; //!< UDP broadcast port number (Default: 14000)
};

struct eCAL_Registration_Local_Configuration
{
  enum eCAL_Registration_Local_eTransportType transport_type; //!< Transport type for registration (Default: udp)
  struct eCAL_Registration_Local_SHM_Configuration shm;
  struct eCAL_Registration_Local_UDP_Configuration udp;
};

enum eCAL_Registration_Network_eTransportType
{
  eCAL_Registration_Network_eTransportType_udp
};

struct eCAL_Registration_Network_UDP_Configuration
{
  unsigned int port; //!< UDP multicast port number (Default: 14000)
};

struct eCAL_Registration_Network_Configuration
{
  enum eCAL_Registration_Network_eTransportType transport_type; //!< Transport type for registration (Default: udp)
  struct eCAL_Registration_Network_UDP_Configuration udp;
};

struct eCAL_Registration_Configuration
{
  unsigned int registration_timeout; //!< Timeout for topic registration in ms (internal) (Default: 10000)
  unsigned int registration_refresh; //!< Topic registration refresh cycle (has to be smaller than registration timeout!) (Default: 1000)
  int loopback; //!< Enable to receive UDP messages on the same local machine (Default: true)
  const char* shm_transport_domain; //!< Common shm transport domain that enables interprocess mechanisms across (virtual) host borders (e.g., Docker); by default equivalent to local host name (Default: "")
  struct eCAL_Registration_Local_Configuration local;
  struct eCAL_Registration_Network_Configuration network;
};

#endif /* ecal_c_config_registration_h_included */