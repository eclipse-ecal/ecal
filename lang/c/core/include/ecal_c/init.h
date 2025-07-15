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

#include <ecal_c/config/configuration.h>

#define ECAL_INIT_PUBLISHER  0x001
#define ECAL_INIT_SUBSCRIBER 0x002
#define ECAL_INIT_SERVICE    0x004
#define ECAL_INIT_MONITORING 0x008
#define ECAL_INIT_LOGGING    0x010
#define ECAL_INIT_TIMESYNC   0x020
#define ECAL_INIT_NONE       0x000

static const unsigned int eCAL_Init_Publisher =  ECAL_INIT_PUBLISHER;
static const unsigned int eCAL_Init_Subscriber = ECAL_INIT_SUBSCRIBER;
static const unsigned int eCAL_Init_Service =    ECAL_INIT_SERVICE;
static const unsigned int eCAL_Init_Monitoring = ECAL_INIT_MONITORING;
static const unsigned int eCAL_Init_Logging =    ECAL_INIT_LOGGING;
static const unsigned int eCAL_Init_TimeSync =   ECAL_INIT_TIMESYNC;

static const unsigned int eCAL_Init_All = ECAL_INIT_PUBLISHER
| ECAL_INIT_SUBSCRIBER
| ECAL_INIT_SERVICE
| ECAL_INIT_MONITORING
| ECAL_INIT_LOGGING
| ECAL_INIT_TIMESYNC;

static const unsigned int eCAL_Init_Default = ECAL_INIT_PUBLISHER
| ECAL_INIT_SUBSCRIBER
| ECAL_INIT_SERVICE
| ECAL_INIT_LOGGING
| ECAL_INIT_TIMESYNC;

static const unsigned int eCAL_Init_None = ECAL_INIT_NONE;

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Convenience function for initializing a new configuration
   *
   * @return Handle to configuration instance if succeeded, NULL otherwise. The handle needs to be released by eCAL_Coniguration_Delete().
  **/
  static inline eCAL_Configuration* eCAL_Init_Configuration()
  {
    eCAL_Configuration* config;
    config = eCAL_Configuration_New();
    eCAL_Configuration_InitFromConfig(config);
    return config;
  }
#ifdef __cplusplus
}
#endif /*__cplusplus*/


#endif /*ecal_c_init_h_included*/
