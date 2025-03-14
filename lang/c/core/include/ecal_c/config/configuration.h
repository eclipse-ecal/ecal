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
  * @file   config/configuration.h
  * @brief  eCAL configuration c interface
 **/

#ifndef ecal_c_config_configuration_h_included
#define ecal_c_config_configuration_h_included

#include <ecal_c/config/application.h>
#include <ecal_c/config/registration.h>
#include <ecal_c/config/logging.h>
#include <ecal_c/config/publisher.h>
#include <ecal_c/config/subscriber.h>
#include <ecal_c/config/time.h>
#include <ecal_c/config/transport_layer.h>

#include <ecal_c/export.h>

enum eCAL_eCommunicationMode
{
  eCAL_eCommunicationMode_local,
  eCAL_eCommunicationMode_network
};

typedef struct eCAL_Configuration_Impl eCAL_Configuration_Impl;

typedef struct 
{
  struct eCAL_TransportLayer_Configuration transport_layer;
  struct eCAL_Registration_Configuration registration;
  struct eCAL_Subscriber_Configuration subscriber;
  struct eCAL_Publisher_Configuration publisher;
  struct eCAL_Time_Configuration timesync;
  struct eCAL_Application_Configuration application;
  struct eCAL_Logging_Configuration logging;

  enum eCAL_eCommunicationMode communication_mode; /*!< eCAL components communication mode:
                                                       local: local host only communication (default)
                                                       cloud: communication across network boundaries */
  eCAL_Configuration_Impl* _impl;
}
eCAL_Configuration;

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

ECALC_API eCAL_Configuration* eCAL_Configuration_New();
ECALC_API void eCAL_Configuration_Delete(eCAL_Configuration* configuration_);

ECALC_API void eCAL_Configuration_InitFromConfig(eCAL_Configuration* configuration_);

ECALC_API void eCAL_Configuration_InitFromFile(eCAL_Configuration* configuration_, const char* yaml_path_);

ECALC_API const char* eCAL_Configuration_GetConfigurationFilePath(eCAL_Configuration* configuration_);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* ecal_c_config_configuration_h_included */