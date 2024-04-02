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
 * @file   ecal_struct_sample_registration.h
 * @brief  eCAL registration as struct
**/

#pragma once

#include "ecal_struct_sample_common.h"
#include "ecal_struct_service.h"

#include <ecal/ecal_types.h>

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace eCAL
{
  namespace Registration
  {
    enum eProcessSeverity
    {
      proc_sev_unknown  = 0,
      proc_sev_healthy  = 1,
      proc_sev_warning  = 2,
      proc_sev_critical = 3,
      proc_sev_failed   = 4
    };

    enum eProcessSeverityLevel
    {
      proc_sev_level_unknown = 0,
      proc_sev_level1        = 1,
      proc_sev_level2        = 2,
      proc_sev_level3        = 3,
      proc_sev_level4        = 4,
      proc_sev_level5        = 5
    };

    enum eTSyncState
    {
      tsync_none     = 0,
      tsync_realtime = 1,
      tsync_replay   = 2
    };

    // Operating system details
    struct OSInfo
    {
      std::string                         osname;                       // name
    };

    // eCAL host
    struct Host
    {
      std::string                         hname;                        // host name
      OSInfo                              os;                           // operating system details
    };

    // Process severity information
    struct ProcessState
    {
      eProcessSeverity                    severity       = proc_sev_unknown;        // severity
      eProcessSeverityLevel               severity_level = proc_sev_level_unknown;  // severity level
      std::string                         info;                                     // info string
    };

    // Transport layer parameters for ecal udp multicast
    struct LayerParUdpMC
    {
    };

    // Transport layer parameters for ecal tcp
    struct LayerParTcp
    {
      int32_t                             port = 0;                     // tcp writers port number
    };

    // Transport layer parameters for ecal shm
    struct LayerParShm
    {
      std::list<std::string>              memory_file_list;             // list of memory file names
    };

    // Connection parameter for reader/writer
    struct ConnectionPar
    {
      LayerParUdpMC                       layer_par_udpmc;              // parameter for ecal udp multicast
      LayerParTcp                         layer_par_tcp;                // parameter for ecal tcp
      LayerParShm                         layer_par_shm;                // parameter for ecal shm
    };

    // Transport layer information
    struct TLayer
    {
      eTLayerType                         type = tl_none;               // transport layer type
      int32_t                             version = 0;                  // transport layer version
      bool                                confirmed = false;            // transport layer used?
      ConnectionPar                       par_layer;                    // transport layer parameter
    };

    // Process information
    struct Process
    {
      int32_t                             rclock = 0;                   // registration clock
      std::string                         hname;                        // host name
      std::string                         hgname;                       // host group name
      int32_t                             pid = 0;                      // process id
      std::string                         pname;                        // process name
      std::string                         uname;                        // unit name
      std::string                         pparam;                       // process parameter
      ProcessState                        state;                        // process state info
      eTSyncState                         tsync_state = tsync_none;     // time synchronization state
      std::string                         tsync_mod_name;               // time synchronization module name
      int32_t                             component_init_state = 0;     // eCAL component initialization state (eCAL::Initialize(..))
      std::string                         component_init_info;          // like comp_init_state as a human-readable string (pub|sub|srv|mon|log|time|proc)
      std::string                         ecal_runtime_version;         // loaded/runtime eCAL version of a component
    };

    // eCAL topic information
    struct Topic
    {
      int32_t                             rclock = 0;                   // registration clock (heart beat)
      std::string                         hname;                        // host name
      std::string                         hgname;                       // host group name
      int32_t                             pid    = 0;                   // process id
      std::string                         pname;                        // process name
      std::string                         uname;                        // unit name
      std::string                         tid;                          // topic id
      std::string                         tname;                        // topic name
      std::string                         direction;                    // direction (publisher, subscriber)
      SDataTypeInformation                tdatatype;                    // topic datatype information (encoding & type & description)

      std::vector<TLayer>                 tlayer;                       // active topic transport layers and its specific parameter
      int32_t                             tsize = 0;                    // topic size

      int32_t                             connections_loc = 0;          // number of local connected entities
      int32_t                             connections_ext = 0;          // number of external connected entities
      int32_t                             message_drops   = 0;          // dropped messages

      int64_t                             did    = 0;                   // data send id (publisher setid)
      int64_t                             dclock = 0;                   // data clock (send / receive action)
      int32_t                             dfreq  = 0;                   // data frequency (send / receive registrations per second) [mHz]

      std::map<std::string, std::string>  attr;                         // generic topic description
    };

    // Registration sample
    struct Sample
    {
      eCmdType                            cmd_type = bct_none;          // registration command type
      Host                                host;                         // host information
      Process                             process;                      // process information
      Service::Service                    service;                      // service information
      Service::Client                     client ;                      // client information
      Topic                               topic;                        // topic information
    };

    // Registration sample list
    struct SampleList
    {
      std::list<Sample>                   samples;                      // list of Samples used currently by SHM registration
    };
  }
}
