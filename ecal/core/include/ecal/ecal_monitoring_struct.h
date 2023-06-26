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

#include <ecal/ecal_os.h>
#include <ecal/ecal_monitoring_entity.h>
#include <ecal/types/topic_information.h>

#include <map>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Monitoring
  {
    struct STopicMon                                            //<! eCAL Topic struct
    {
      int                                 rclock             = 0;       //!< registration clock (heart beat)
      int                                 hid                = 0;       //!< host id (not used currently)
      std::string                         hname;                        //!< host name
      int                                 pid                = 0;       //!< process id
      std::string                         pname;                        //!< process name
      std::string                         uname;                        //!< unit name
      std::string                         tid;                          //!< topic id
      std::string                         tname;                        //!< topic name
      std::string                         direction;                    //!< direction (publisher, subscriber)
      STopicInformation                   tinfo;                        //!< topic information (encoding, type, descriptor)
      int                                 tsize              = 0;       //!< topic size

      bool                                tlayer_ecal_udp_mc = false;   //!< transport layer udp active
      bool                                tlayer_ecal_shm    = false;   //!< transport layer shm active
      bool                                tlayer_ecal_tcp    = false;   //!< transport layer tcp active
      bool                                tlayer_inproc      = false;   //!< transport layer inproc active

      int                                 connections_loc    = 0;        //!< number of local connected entities
      int                                 connections_ext    = 0;        //!< number of external connected entities
      long long                           message_drops      = 0;        //!< dropped messages

      long long                           did                = 0;        //!< data send id (publisher setid)
      long long                           dclock             = 0;        //!< data clock (send / receive action)
      long                                dfreq              = 0;        //!< data frequency (send / receive samples per second) [mHz]

      std::map<std::string, std::string>  attr;                 //!< generic topic description
    };

    struct SProcessMon                                                   //<! eCAL Process struct
    {
      int            rclock               = 0;                  //!< registration clock
      std::string    hname;                                     //!< host name
      int            pid                  = 0;                  //!< process id
      std::string    pname;                                     //!< process name
      std::string    uname;                                     //!< unit name
      std::string    pparam;                                    //!< process parameter

      long long      pmemory              = 0;                  //!< process memory
      float          pcpu                 = 0.0f;               //!< process cpu usage
      float          usrptime             = 0.0f;               //!< process user time

      long long      datawrite            = 0;                  //!< data write bytes per sec
      long long      dataread             = 0;                  //!< data read bytes per sec

      int            state_severity       = 0;                  //!< process state info severity:
                                                                //!<   proc_sev_unknown       = 0 (condition unknown)
                                                                //!<   proc_sev_healthy       = 1 (process healthy)
                                                                //!<   proc_sev_warning       = 2 (process warning level)
                                                                //!<   proc_sev_critical      = 3 (process critical)
                                                                //!<   proc_sev_failed        = 4 (process failed)
      int            state_severity_level = 0;                  //!< process state info severity level:
                                                                //!<   proc_sev_level_unknown = 0 (condition unknown)
                                                                //!<   proc_sev_level1        = 1 (default severity level 1)
                                                                //!<   proc_sev_level2        = 2 (severity level 2)
                                                                //!<   proc_sev_level3        = 3 (severity level 3)
                                                                //!<   proc_sev_level4        = 4 (severity level 4)
                                                                //!<   proc_sev_level5        = 5 (severity level 5)

      std::string    state_info;                                //!< process state info as human readable string

      int            tsync_state          = 0;                  //!< time synchronization state
      std::string    tsync_mod_name;                            //!< time synchronization module name

      int            component_init_state = 0;                  //!< eCAL component initialization state (eCAL::Initialize(..))
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
      long long    call_count = 0;                              //<! call counter
    };

    struct SServerMon                                           //<! eCAL Server struct
    {
      int                      rclock      = 0;                 //<! registration clock
      std::string              hname;                           //<! host name
      std::string              pname;                           //<! process name
      std::string              uname;                           //<! unit name
      int                      pid;                             //<! process id

      std::string              sname;                           //<! service name
      std::string              sid;                             //<! service id

      int                      tcp_port_v0 = 0;                 //<! the tcp port protocol version 0 used for that service
      int                      tcp_port_v1 = 0;                 //<! the tcp port protocol version 1 used for that service

      std::vector<SMethodMon>  methods;                         //<! list of methods
    };

    struct SClientMon                                           //<! eCAL Client struct
    {
      int          rclock = 0;                                  //<! registration clock
      std::string  hname;                                       //<! host name
      std::string  pname;                                       //<! process name
      std::string  uname;                                       //<! unit name
      int          pid    = 0;                                  //<! process id

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

    /**
     * @brief Get monitoring as a struct.
     *
     * @param [out] mon_       Target struct to store monitoring information.
     * @param       entities_  Entities definition.
     *
     * @return Number of struct elements if succeeded.
    **/
    ECAL_API int GetMonitoring(eCAL::Monitoring::SMonitoring& mon_, unsigned int entities_ = Entity::All);
  }
}
