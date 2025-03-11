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

#ifndef ecal_c_types_monitoring_h_included
#define ecal_c_types_monitoring_h_included

#define ECAL_MONITORING_ENTITY_PUBLISHER 0x001
#define ECAL_MONITORING_ENTITY_SUBSCRIBER 0x002
#define ECAL_MONITORING_ENTITY_SERVER 0x004
#define ECAL_MONITORING_ENTITY_CLIENT 0x008
#define ECAL_MONITORING_ENTITY_PROCESS 0x010
#define ECAL_MONITORING_ENTITY_HOST 0x020
#define ECAL_MONITORING_ENTITY_NONE 0x000

static const unsigned int eCAL_Monitoring_Entity_Publisher = ECAL_MONITORING_ENTITY_PUBLISHER;
static const unsigned int eCAL_Monitoring_Entity_Subscriber = ECAL_MONITORING_ENTITY_SUBSCRIBER;
static const unsigned int eCAL_Monitoring_Entity_Server = ECAL_MONITORING_ENTITY_SERVER;
static const unsigned int eCAL_Monitoring_Entity_Client = ECAL_MONITORING_ENTITY_CLIENT;
static const unsigned int eCAL_Monitoring_Entity_Process = ECAL_MONITORING_ENTITY_PROCESS;
static const unsigned int eCAL_Monitoring_Entity_Host = ECAL_MONITORING_ENTITY_HOST;

static const unsigned int eCAL_Monitoring_Entity_All = ECAL_MONITORING_ENTITY_PUBLISHER
| ECAL_MONITORING_ENTITY_SUBSCRIBER
| ECAL_MONITORING_ENTITY_SERVER
| ECAL_MONITORING_ENTITY_CLIENT
| ECAL_MONITORING_ENTITY_PROCESS
| ECAL_MONITORING_ENTITY_HOST;

static const unsigned int eCAL_Monitoring_Entity_None = ECAL_MONITORING_ENTITY_NONE;

#endif /*ecal_c_types_monitoring_h_included*/
