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
 * @file   monitoring.h
 * @brief  eCAL monitoring struct interface
**/

#pragma once

#include <ecal/ecal_types.h>

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
    
    enum eTLayerType
    {
      tl_none        = 0,
      tl_ecal_udp_mc = 1,
      tl_ecal_shm    = 4,
      tl_ecal_tcp    = 5,
    };

    struct TLayer
    {
      eTLayerType  type      = tl_none;                         //<! transport layer type
      int32_t      version   = 0;                               //<! transport layer version
      bool         confirmed = false;                           //<! transport layer used?
    };

    struct STopicMon                                            //<! eCAL Topic struct
    {
      int32_t                             rclock{0};            //!< registration clock (heart beat)
      std::string                         hname;                //!< host name
      std::string                         hgname;               //!< host group name
      int32_t                             pid{0};               //!< process id
      std::string                         pname;                //!< process name
      std::string                         uname;                //!< unit name
      std::string                         tid;                  //!< topic id
      std::string                         tname;                //!< topic name
      std::string                         direction;            //!< direction (publisher, subscriber)
      SDataTypeInformation                tdatatype;            //!< topic datatype information (name, encoding, descriptor)
	  
      std::vector<TLayer>                 tlayer;               //!< transport layer details
      int32_t                             tsize{0};             //!< topic size

      int32_t                             connections_loc{0};   //!< number of local connected entities
      int32_t                             connections_ext{0};   //!< number of external connected entities
      int32_t                             message_drops{0};     //!< dropped messages

      int64_t                             did{0};               //!< data send id (publisher setid)
      int64_t                             dclock{0};            //!< data clock (send / receive action)
      int32_t                             dfreq{0};             //!< data frequency (send / receive samples per second) [mHz]

      std::map<std::string, std::string>  attr;                 //!< generic topic description
    };

    struct SProcessMon                                          //<! eCAL Process struct
    {
      int32_t        rclock{0};                                 //!< registration clock
      std::string    hname;                                     //!< host name
      std::string    hgname;                                    //!< host group name
      int32_t        pid{0};                                    //!< process id
      std::string    pname;                                     //!< process name
      std::string    uname;                                     //!< unit name
      std::string    pparam;                                    //!< process parameter

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

      int32_t        tsync_state{0};                            //!< time synchronization state
      std::string    tsync_mod_name;                            //!< time synchronization module name

      int32_t        component_init_state{0};                   //!< eCAL component initialization state (eCAL::Initialize(..))
      std::string    component_init_info;                       //!< like comp_init_state as human readable string (pub|sub|srv|mon|log|time|proc)

      std::string    ecal_runtime_version;                      //!< loaded / runtime eCAL version of a component
    };

    struct SMethodMon                                           //<! eCAL Server Method struct
    {
      std::string  mname;                                       //<! method name
      std::string  req_type;                                    //<! request type
      std::string  req_desc;                                    //<! request descriptor
      std::string  resp_type;                                   //<! response type
      std::string  resp_desc;                                   //<! response descriptor
      long long    call_count{0};                               //<! call counter
    };

    struct SServerMon                                           //<! eCAL Server struct
    {
      int32_t                  rclock{0};                       //<! registration clock
      std::string              hname;                           //<! host name
      std::string              pname;                           //<! process name
      std::string              uname;                           //<! unit name
      int32_t                  pid{0};                          //<! process id

      std::string              sname;                           //<! service name
      std::string              sid;                             //<! service id

      uint32_t                 version{0};                      //<! service protocol version
      uint32_t                 tcp_port_v0{0};                  //<! the tcp port protocol version 0 used for that service
      uint32_t                 tcp_port_v1{0};                  //<! the tcp port protocol version 1 used for that service

      std::vector<SMethodMon>  methods;                         //<! list of methods
    };

    struct SClientMon                                           //<! eCAL Client struct
    {
      int32_t      rclock{0};                                   //<! registration clock
      std::string  hname;                                       //<! host name
      std::string  pname;                                       //<! process name
      std::string  uname;                                       //<! unit name
      int32_t      pid{0};                                      //<! process id

      std::string  sname;                                       //<! service name
      std::string  sid;                                         //<! service id

      uint32_t     version{0};                                  //<! client protocol version
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
