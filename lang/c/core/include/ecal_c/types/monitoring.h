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
 * @file   types/monitoring.h
 * @brief  eCAL monitoring types
**/

#ifndef ecal_c_types_monitoring_h_included
#define ecal_c_types_monitoring_h_included

#include <ecal_c/types.h>

#define ECAL_MONITORING_ENTITY_PUBLISHER  0x001
#define ECAL_MONITORING_ENTITY_SUBSCRIBER 0x002
#define ECAL_MONITORING_ENTITY_SERVER     0x004
#define ECAL_MONITORING_ENTITY_CLIENT     0x008
#define ECAL_MONITORING_ENTITY_PROCESS    0x010
#define ECAL_MONITORING_ENTITY_HOST       0x020
#define ECAL_MONITORING_ENTITY_NONE       0x000

static const unsigned int eCAL_Monitoring_Entity_Publisher =  ECAL_MONITORING_ENTITY_PUBLISHER;
static const unsigned int eCAL_Monitoring_Entity_Subscriber = ECAL_MONITORING_ENTITY_SUBSCRIBER;
static const unsigned int eCAL_Monitoring_Entity_Server =     ECAL_MONITORING_ENTITY_SERVER;
static const unsigned int eCAL_Monitoring_Entity_Client =     ECAL_MONITORING_ENTITY_CLIENT;
static const unsigned int eCAL_Monitoring_Entity_Process =    ECAL_MONITORING_ENTITY_PROCESS;
static const unsigned int eCAL_Monitoring_Entity_Host =       ECAL_MONITORING_ENTITY_HOST;

static const unsigned int eCAL_Monitoring_Entity_All = ECAL_MONITORING_ENTITY_PUBLISHER
| ECAL_MONITORING_ENTITY_SUBSCRIBER
| ECAL_MONITORING_ENTITY_SERVER
| ECAL_MONITORING_ENTITY_CLIENT
| ECAL_MONITORING_ENTITY_PROCESS
| ECAL_MONITORING_ENTITY_HOST;

static const unsigned int eCAL_Monitoring_Entity_None = ECAL_MONITORING_ENTITY_NONE;

enum eCAL_Monitoring_eTransportLayerType
{
  eCAL_Monitoring_eTransportLayerType_none = 0,
  eCAL_Monitoring_eTransportLayerType_udp_mc = 1,
  eCAL_Monitoring_eTransportLayerType_shm = 4,
  eCAL_Monitoring_eTransportLayerType_tcp = 5
};

struct eCAL_Monitoring_STransportLayer
{
  enum eCAL_Monitoring_eTransportLayerType type; // transport layer type
  int32_t version;                              // transport layer version
  int active;                                   // transport layer used?
};

struct eCAL_Monitoring_STopic
{
  int32_t registration_clock;                   // registration clock (heart beat)
  const char* host_name;                        // host name
  const char* shm_transport_domain;             // shm transport domain
  int32_t process_id;                           // process id
  const char* process_name;                     // process name
  const char* unit_name;                        // unit name
  int64_t topic_id;                             // topic id
  const char* topic_name;                       // topic name
  const char* direction;                        // direction (publisher, subscriber)
  struct eCAL_SDataTypeInformation datatype_information; // topic datatype information (name, encoding, descriptor)
  struct eCAL_Monitoring_STransportLayer* transport_layer; // transport layer details
  size_t transport_layer_length;                // array of transport layer details
  int32_t topic_size;                           // topic size
  int32_t connections_local;                    // number of local connected entities
  int32_t connections_external;                 // number of external connected entities
  int32_t message_drops;                        // dropped messages
  int64_t data_id;                              // data send id (publisher setid)
  int64_t data_clock;                           // data clock (send / receive action)
  int32_t data_frequency;                       // data frequency (send / receive samples per second) [mHz]
};

struct eCAL_Monitoring_SProcess
{
  int32_t registration_clock;                   // registration clock
  const char* host_name;                        // host name
  const char* shm_transport_domain;             // shm transport domain
  int32_t process_id;                           // process id
  const char* process_name;                     // process name
  const char* unit_name;                        // unit name
  const char* process_parameter;                // process parameter
  int32_t state_severity;                       // process state info severity
  int32_t state_severity_level;                 // process state info severity level
  const char* state_info;                       // process state info as human readable string
  int32_t time_sync_state;                      // time synchronization state
  const char* time_sync_module_name;            // time synchronization module name
  int32_t component_init_state;                 // eCAL component initialization state (eCAL::Initialize(..))
  const char* component_init_info;              // like comp_init_state as human readable string (pub/sub/srv/mon/log/time/proc)
  const char* ecal_runtime_version;             // loaded / runtime eCAL version of a component
  const char* config_file_path;                 // Filepath of the configuration filepath that was loaded
};

struct eCAL_Monitoring_SMethod
{
  const char* method_name;                      // method name
  struct eCAL_SDataTypeInformation request_datatype_information; // request datatype information (encoding & type & description)
  struct eCAL_SDataTypeInformation response_datatype_information; // response datatype information (encoding & type & description)
  long long call_count;                         // call counter
};

struct eCAL_Monitoring_SServer
{
  int32_t registration_clock;                   // registration clock
  const char* host_name;                        // host name
  const char* process_name;                     // process name
  const char* unit_name;                        // unit name
  int32_t process_id;                           // process id
  const char* service_name;                     // service name
  int64_t service_id;                           // service id
  uint32_t version;                             // service protocol version
  uint32_t tcp_port_v0;                         // the tcp port protocol version 0 used for that service
  uint32_t tcp_port_v1;                         // the tcp port protocol version 1 used for that service
  struct eCAL_Monitoring_SMethod* methods;      // list of methods
  size_t methods_length;                        // array of methods
};

struct eCAL_Monitoring_SClient
{
  int32_t registration_clock;                   // registration clock
  const char* host_name;                        // host name
  const char* process_name;                     // process name
  const char* unit_name;                        // unit name
  int32_t process_id;                           // process id
  const char* service_name;                     // service name
  int64_t service_id;                           // service id
  struct eCAL_Monitoring_SMethod* methods;      // list of methods
  size_t methods_length;                        // array of methods
  uint32_t version;                             // client protocol version
};

struct eCAL_Monitoring_SMonitoring
{
  struct eCAL_Monitoring_SProcess* processes; // process info
  size_t processes_length;                      // array of process info
  struct eCAL_Monitoring_STopic* publishers;   // publisher info array
  size_t publishers_length;                      // array of publisher info
  struct eCAL_Monitoring_STopic* subscribers;  // subscriber info array
  size_t subscribers_length;                     // array of subscriber info
  struct eCAL_Monitoring_SServer* servers;     // server info array
  size_t servers_length;                         // array of server info
  struct eCAL_Monitoring_SClient* clients;    // clients info array
  size_t clients_length;                        // array of clients info
};

#endif /*ecal_c_types_monitoring_h_included*/
