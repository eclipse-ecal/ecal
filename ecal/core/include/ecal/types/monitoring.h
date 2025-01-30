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
 * @brief  eCAL monitoring struct interface
**/

#pragma once

#include <ecal/types.h>

#include <map>
#include <string>
#include <vector>
#include <cstdint>

namespace eCAL
{
  namespace Monitoring
  {
    namespace Entity
    {
      constexpr unsigned int Publisher  = 0x001;
      constexpr unsigned int Subscriber = 0x002;
      constexpr unsigned int Server     = 0x004;
      constexpr unsigned int Client     = 0x008;
      constexpr unsigned int Process    = 0x010;
      constexpr unsigned int Host       = 0x020;

      constexpr unsigned int All = Publisher
        | Subscriber
        | Server
        | Client
        | Process
        | Host;

      constexpr unsigned int None = 0x000;
    }
    
    enum class eTransportLayerType
    {
      none   = 0,
      udp_mc = 1,
      shm    = 4,
      tcp    = 5,
    };

    struct STransportLayer
    {
      eTransportLayerType  type    = eTransportLayerType::none;    //<! transport layer type
      int32_t      version = 0;                                    //<! transport layer version
      bool         active  = false;                                //<! transport layer used?
    };

    struct STopicMon                                               //<! eCAL Topic struct
    {
      int32_t                             registration_clock{0};   //!< registration clock (heart beat)
      std::string                         host_name;               //!< host name
      std::string                         shm_transport_domain;    //!< shm transport domain
      int32_t                             process_id{0};           //!< process id
      std::string                         process_name;            //!< process name
      std::string                         unit_name;               //!< unit name
      EntityIdT                           topic_id{0};             //!< topic id
      std::string                         topic_name;              //!< topic name
      std::string                         direction;               //!< direction (publisher, subscriber)
      SDataTypeInformation                datatype_information;    //!< topic datatype information (name, encoding, descriptor)
	  
      std::vector<STransportLayer>        transport_layer;         //!< transport layer details
      int32_t                             topic_size{0};           //!< topic size

      int32_t                             connections_local{0};    //!< number of local connected entities
      int32_t                             connections_external{0}; //!< number of external connected entities
      int32_t                             message_drops{0};        //!< dropped messages

      int64_t                             data_id{0};              //!< data send id (publisher setid)
      int64_t                             data_clock{0};           //!< data clock (send / receive action)
      int32_t                             data_frequency{0};       //!< data frequency (send / receive samples per second) [mHz]
    };

    struct SProcessMon                                             //<! eCAL Process struct
    {
      int32_t        registration_clock{0};                        //!< registration clock
      std::string    host_name;                                    //!< host name
      std::string    shm_transport_domain;                         //!< shm transport domain
      int32_t        process_id{0};                                //!< process id
      std::string    process_name;                                 //!< process name
      std::string    unit_name;                                    //!< unit name
      std::string    process_parameter;                            //!< process parameter

      int32_t        state_severity{0};                         //!< process state info severity:
                                                                //!<   proc_sev_unknown       = 0 (condition unknown)
                                                                //!<   proc_sev_healthy       = 1 (process healthy)
                                                                //!<   proc_sev_warning       = 2 (process warning level)
                                                                //!<   proc_sev_critical      = 3 (process critical)
                                                                //!<   proc_sev_failed        = 4 (process failed)
      int32_t        state_severity_level{0};                   //!< process state info severity level:
                                                                //!<   proc_sev_level_unknown = 0 (condition unknown)
                                                                //!<   proc_sev_level1        = 1 (default severity level 1)
                                                                //!<   proc_sev_level2        = 2 (severity level 2)
                                                                //!<   proc_sev_level3        = 3 (severity level 3)
                                                                //!<   proc_sev_level4        = 4 (severity level 4)
                                                                //!<   proc_sev_level5        = 5 (severity level 5)

      std::string    state_info;                                //!< process state info as human readable string

      int32_t        time_sync_state{0};                        //!< time synchronization state
      std::string    time_sync_module_name;                     //!< time synchronization module name

      int32_t        component_init_state{0};                   //!< eCAL component initialization state (eCAL::Initialize(..))
      std::string    component_init_info;                       //!< like comp_init_state as human readable string (pub|sub|srv|mon|log|time|proc)

      std::string    ecal_runtime_version;                      //!< loaded / runtime eCAL version of a component
      std::string    config_file_path;                          //!< Filepath of the configuration filepath that was loaded
    };

    struct SMethodMon                                           //<! eCAL Server Method struct
    {
      std::string           method_name;                        //<! method name

      SDataTypeInformation  request_datatype_information;       //<! request  datatype information (encoding & type & description)
      SDataTypeInformation  response_datatype_information;      //<! response datatype information (encoding & type & description)

      long long             call_count{0};                      //<! call counter
    };

    struct SServerMon                                           //<! eCAL Server struct
    {
      int32_t                  registration_clock{0};           //<! registration clock
      std::string              host_name;                       //<! host name
      std::string              process_name;                    //<! process name
      std::string              unit_name;                       //<! unit name
      int32_t                  process_id{0};                   //<! process id

      std::string              service_name;                    //<! service name
      EntityIdT                service_id{0};                          //<! service id

      uint32_t                 version{0};                      //<! service protocol version
      uint32_t                 tcp_port_v0{0};                  //<! the tcp port protocol version 0 used for that service
      uint32_t                 tcp_port_v1{0};                  //<! the tcp port protocol version 1 used for that service

      std::vector<SMethodMon>  methods;                         //<! list of methods
    };

    struct SClientMon                                           //<! eCAL Client struct
    {
      int32_t                  registration_clock{0};           //<! registration clock
      std::string              host_name;                       //<! host name
      std::string              process_name;                    //<! process name
      std::string              unit_name;                       //<! unit name
      int32_t                  process_id{0};                   //<! process id

      std::string              service_name;                    //<! service name
      EntityIdT                service_id{0};                          //<! service id

      std::vector<SMethodMon>  methods;                         //<! list of methods

      uint32_t                 version{0};                      //<! client protocol version
    };

    struct SMonitoring                                          //<! eCAL Monitoring struct
    {
      std::vector<SProcessMon>  processes;                      //<! process info
      std::vector<STopicMon>    publisher;                      //<! publisher info vector
      std::vector<STopicMon>    subscriber;                     //<! subscriber info vector
      std::vector<SServerMon>   server;                         //<! server info vector
      std::vector<SClientMon>   clients;                        //<! clients info vector
    };
  }
}
