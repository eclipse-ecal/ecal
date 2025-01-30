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
 * @file   ecal_c/tlayer.h
 * @brief  eCAL transport layer
**/

#ifndef ecal_c_tlayer_h_included
#define ecal_c_tlayer_h_included

/**
 * @brief eCAL transport layer types.
**/
enum eTransportLayerC
{
  tlayer_none       = 0,
  tlayer_udp_mc     = 1,
  tlayer_shm        = 4,
  tlayer_tcp        = 5,
  tlayer_all        = 255
};

/**
 * @brief eCAL transport layer modes.
**/
enum eSendModeC
{
  smode_none = -1,
  smode_off  = 0,
  smode_on,
  smode_auto
};

#endif /*ecal_c_tlayer_h_included*/
