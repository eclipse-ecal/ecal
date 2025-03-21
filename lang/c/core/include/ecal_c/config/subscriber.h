/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
  * @file   config/subscriber.h
  * @brief  eCAL subscriber configuration
 **/

#ifndef ecal_c_config_subscriber_h_included
#define ecal_c_config_subscriber_h_included

struct eCAL_Subscriber_Layer_SHM_Configuration
{
  int enable;  //!< enable layer (Default: true)
};

struct eCAL_Subscriber_Layer_UDP_Configuration
{
  int enable;  //!< enable layer (Default: true)
};

struct eCAL_Subscriber_Layer_TCP_Configuration
{
  int enable;  //!< enable layer (Default: false)
};

struct eCAL_Subscriber_Layer_Configuration
{
  struct eCAL_Subscriber_Layer_SHM_Configuration shm;
  struct eCAL_Subscriber_Layer_UDP_Configuration udp;
  struct eCAL_Subscriber_Layer_TCP_Configuration tcp;
};

struct eCAL_Subscriber_Configuration
{
  struct eCAL_Subscriber_Layer_Configuration layer;

  int drop_out_of_order_messages;  //!< Enable dropping of payload messages that arrive out of order (Default: true)
};

#endif /* ecal_c_config_subscriber_h_included */