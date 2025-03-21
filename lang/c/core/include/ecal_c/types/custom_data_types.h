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

 /**
  * @file   types/custom_types.h
  * @brief  eCAL custom types for configuration declarations
 **/

#ifndef ecal_c_types_custom_data_types_h_included
#define ecal_c_types_custom_data_types_h_included


 /**
  * @brief eCAL udp protocol version
 **/
enum eCAL_Types_UdpConfigVersion
{
  eCAL_Types_UdpConfigVersion_V1, //!< protocol version v1 (deprecated)
  eCAL_Types_UdpConfigVersion_V2  //!< protocol version v2
};

#endif /*ecal_c_types_custom_data_types_h_included*/