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
 * @file   ecal_monitoring_pb.h
 * @brief  eCAL monitoring interface using structs
**/

#pragma once

#include <ecal/ecal_types.h>

#include <map>
#include <string>
#include <vector>

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
    
    struct STopicMon                                            //<! eCAL Topic struct
    {
      STopicMon()
      {
        rclock             = 0;
        hid                = 0;
        pid                = 0;
        tsize              = 0;
        tlayer_ecal_udp_mc = false;
        tlayer_ecal_shm    = false;
        tlayer_ecal_tcp    = false;
        tlayer_inproc      = false;
        connections_loc    = 0;
        connections_ext    = 0;
        message_drops      = 0;
        did                = 0;
        dclock             = 0;
        dfreq              = 0;
      };

      int                                 rclock;               //!< registration clock (heart beat)
      int                                 hid;                  //!< host id
      std::string                         hname;                //!< host name
      std::string                         hgname;               //!< host group name
      int                                 pid;                  //!< process id
      std::string                         pname;                //!< process name
      std::string                         uname;                //!< unit name
      std::string                         tid;                  //!< topic id
      std::string                         tname;                //!< topic name
      std::string                         direction;            //!< direction (publisher, subscriber)
      SDataTypeInformation                tdatatype;            //!< topic datatype information (name, encoding, descriptor)
      int                                 tsize;                //!< topic size

      bool                                tlayer_ecal_udp_mc;   //!< transport layer udp active
      bool                                tlayer_ecal_shm;      //!< transport layer shm active
      bool                                tlayer_ecal_tcp;      //!< transport layer tcp active
      bool                                tlayer_inproc;        //!< transport layer inproc active

      int                                 connections_loc;      //!< number of local connected entities
      int                                 connections_ext;      //!< number of external connected entities
      long long                           message_drops;        //!< dropped messages

      long long                           did;                  //!< data send id (publisher setid)
      long long                           dclock;               //!< data clock (send / receive action)
      long                                dfreq;                //!< data frequency (send / receive samples per second) [mHz]

      std::map<std::string, std::string>  attr;                 //!< generic topic description
    };

    struct SProcessMon                                          //<! eCAL Process struct
    {
      SProcessMon()
      {
        rclock               = 0;
        pid                  = 0;
        pmemory              = 0;
        pcpu                 = 0.0f;
        usrptime             = 0.0f;
        datawrite            = 0;
        dataread             = 0;
        state_severity       = 0;
        state_severity_level = 0;
        tsync_state          = 0;
        component_init_state = 0;
      };

      int            rclock;                                    //!< registration clock
      std::string    hname;                                     //!< host name
      std::string    hgname;                                    //!< host group name
      int            pid;                                       //!< process id
      std::string    pname;                                     //!< process name
      std::string    uname;                                     //!< unit name
      std::string    pparam;                                    //!< process parameter

      long long      pmemory;                                   //!< process memory
      float          pcpu;                                      //!< process cpu usage
      float          usrptime;                                  //!< process user time

      long long      datawrite;                                 //!< data write bytes per sec
      long long      dataread;                                  //!< data read bytes per sec

      int            state_severity;                            //!< process state info severity:
                                                                //!<   proc_sev_unknown       = 0 (condition unknown)
                                                                //!<   proc_sev_healthy       = 1 (process healthy)
                                                                //!<   proc_sev_warning       = 2 (process warning level)
                                                                //!<   proc_sev_critical      = 3 (process critical)
                                                                //!<   proc_sev_failed        = 4 (process failed)
      int            state_severity_level;                      //!< process state info severity level:
                                                                //!<   proc_sev_level_unknown = 0 (condition unknown)
                                                                //!<   proc_sev_level1        = 1 (default severity level 1)
                                                                //!<   proc_sev_level2        = 2 (severity level 2)
                                                                //!<   proc_sev_level3        = 3 (severity level 3)
                                                                //!<   proc_sev_level4        = 4 (severity level 4)
                                                                //!<   proc_sev_level5        = 5 (severity level 5)

      std::string    state_info;                                //!< process state info as human readable string

      int            tsync_state;                               //!< time synchronization state
      std::string    tsync_mod_name;                            //!< time synchronization module name

      int            component_init_state;                      //!< eCAL component initialization state (eCAL::Initialize(..))
      std::string    component_init_info;                       //!< like comp_init_state as human readable string (pub|sub|srv|mon|log|time|proc)

      std::string    ecal_runtime_version;                      //!< loaded / runtime eCAL version of a component
    };

    struct SMethodMon                                           //<! eCAL Server Method struct
    {
      SMethodMon()
      {
        call_count = 0;
      };
      std::string  mname;                                       //<! method name
      std::string  req_type;                                    //<! request type
      std::string  req_desc;                                    //<! request descriptor
      std::string  resp_type;                                   //<! response type
      std::string  resp_desc;                                   //<! response descriptor
      long long    call_count;                                  //<! call counter
    };

    struct SServerMon                                           //<! eCAL Server struct
    {
      SServerMon()
      {
        rclock   = 0;
        pid      = 0;
        tcp_port = 0;
      };

      int                      rclock;                          //<! registration clock    
      std::string              hname;                           //<! host name
      std::string              pname;                           //<! process name
      std::string              uname;                           //<! unit name
      int                      pid;                             //<! process id

      std::string              sname;                           //<! service name
      std::string              sid;                             //<! service id

      int                      tcp_port;                        //<! the tcp port used for that service

      std::vector<SMethodMon>  methods;                         //<! list of methods
    };

    struct SClientMon                                           //<! eCAL Client struct
    {
      SClientMon()
      {
        rclock = 0;
        pid    = 0;
      };

      int          rclock;                                      //<! registration clock
      std::string  hname;                                       //<! host name
      std::string  pname;                                       //<! process name
      std::string  uname;                                       //<! unit name
      int          pid;                                         //<! process id

      std::string  sname;                                       //<! service name
      std::string  sid;                                         //<! service id
    };

    struct SMonitoring                                          //<! eCAL Monitoring struct
    {
      std::vector<SProcessMon>  process;                        //<! process info
      std::vector<STopicMon>    publisher;                      //<! publisher info vector
      std::vector<STopicMon>    subscriber;                     //<! subscriber info vector
      std::vector<SServerMon>   server;                         //<! server info vector
      std::vector<SClientMon>   clients;                        //<! clients info vector
    };

  }
}
