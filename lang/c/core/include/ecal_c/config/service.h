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

 /**
  * @file   config/service.h
  * @brief  eCAL configuration for service layer
 **/

#ifndef ecal_c_config_service_h_included
#define ecal_c_config_service_h_included

struct eCAL_Service_Configuration
{
  unsigned int server_client_id_timeout_ms; /*!< Timeout in ms to wait for the client to send its entity ID after
                                                 connecting. If the client does not send its entity ID within this
                                                 time, the server will use a placeholder.
                                                 Set to 0 to disable waiting (old behavior). Default: 1000 */
};

#endif /* ecal_c_config_service_h_included */
