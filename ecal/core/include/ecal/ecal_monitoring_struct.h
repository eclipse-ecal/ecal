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

#include <map>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Monitoring
  {
    struct STopicMon
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

      int                                 rclock;              //!< registration clock (heart beat)
      int                                 hid;                 //!< host id
      std::string                         hname;               //!< host name
      int                                 pid;                 //!< process id
      std::string                         pname;               //!< process name
      std::string                         uname;               //!< unit name
      std::string                         tid;                 //!< topic id
      std::string                         tname;               //!< topic name
      std::string                         direction;           //!< direction (publisher, subscriber)
      std::string                         ttype;               //!< topic type (protocol)
      std::string                         tdesc;               //!< topic description (protocol descriptor)
      int                                 tsize;               //!< topic size

      bool                                tlayer_ecal_udp_mc;  //!< transport layer udp active
      bool                                tlayer_ecal_shm;     //!< transport layer shm active
      bool                                tlayer_ecal_tcp;     //!< transport layer tcp active
      bool                                tlayer_inproc;       //!< transport layer inproc active

      int                                 connections_loc;     //!< number of local connected entities
      int                                 connections_ext;     //!< number of external connected entities
      long long                           message_drops;       //!< dropped messages

      long long                           did;                 //!< data send id (publisher setid)
      long long                           dclock;              //!< data clock (send / receive action)
      long                                dfreq;               //!< data frequency (send / receive samples per second) [mHz]

      std::map<std::string, std::string>  attr;                //!< generic topic description
    };

    struct SProcessMon
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

      int            rclock;
      std::string    hname;
      std::string    pname;
      std::string    uname;
      int            pid;
      std::string    pparam;
      long long      pmemory;
      float          pcpu;
      float          usrptime;
      long long      datawrite;
      long long      dataread;
      int            state_severity;
      int            state_severity_level;
      std::string    state_info;
      int            tsync_state;
      std::string    tsync_mod_name;
      int            component_init_state;
      std::string    component_init_info;
      std::string    ecal_runtime_version;
    };

    struct SMethodMon
    {
      SMethodMon()
      {
        call_count = 0;
      };
      std::string  mname;
      std::string  req_type;
      std::string  req_desc;
      std::string  resp_type;
      std::string  resp_desc;
      long long    call_count;
    };

    struct SServerMon
    {
      SServerMon()
      {
        rclock   = 0;
        pid      = 0;
        tcp_port = 0;
      };

      int                      rclock;
      std::string              hname;
      std::string              sname;
      std::string              sid;
      std::string              pname;
      std::string              uname;
      int                      pid;
      int                      tcp_port;
      std::vector<SMethodMon>  methods;
    };

    struct SClientMon
    {
      SClientMon()
      {
        rclock = 0;
        pid    = 0;
      };

      int          rclock;
      std::string  hname;
      std::string  sname;
      std::string  sid;
      std::string  pname;
      std::string  uname;
      int          pid;
    };

    struct SMonitoring
    {
      std::vector<SProcessMon>  process;
      std::vector<STopicMon>    publisher;
      std::vector<STopicMon>    subscriber;
      std::vector<SServerMon>   server;
      std::vector<SClientMon>   clients;
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
