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
 * @file   ecal_c/init.h
 * @brief  eCAL initialize components
**/

#ifndef ecal_c_init_h_included
#define ecal_c_init_h_included

#define eCAL_Init_Publisher   0x01                          /*!< Initialize Publisher API            */
#define eCAL_Init_Subscriber  0x02                          /*!< Initialize Subscriber API           */
#define eCAL_Init_Service     0x04                          /*!< Initialize Service API              */
#define eCAL_Init_Monitoring  0x08                          /*!< Initialize Monitoring API           */
#define eCAL_Init_Logging     0x10                          /*!< Initialize Logging API              */
#define eCAL_Init_TimeSync    0x20                          /*!< Initialize Time API                 */

#define eCAL_Init_All        (eCAL_Init_Publisher  \
                            | eCAL_Init_Subscriber \
                            | eCAL_Init_Service    \
                            | eCAL_Init_Monitoring \
                            | eCAL_Init_Logging    \
                            | eCAL_Init_TimeSync)           /*!< Initialize complete eCAL API        */

#define eCAL_Init_Default    (eCAL_Init_Publisher  \
                            | eCAL_Init_Subscriber \
                            | eCAL_Init_Service    \
                            | eCAL_Init_Logging    \
                            | eCAL_Init_TimeSync)           /*!< Initialize default eCAL API          */

#endif /*ecal_c_init_h_included*/
