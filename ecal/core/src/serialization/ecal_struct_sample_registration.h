/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
#include "util/expanding_vector.h"

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <tuple>

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

      bool operator==(const OSInfo& other) const {
        return osname == other.osname;
      }

      void clear()
      {
        osname.clear();
      }
    };

    // eCAL host
    struct Host
    {
      std::string                         hname;                        // host name
      OSInfo                              os;                           // operating system details

      bool operator==(const Host& other) const {
        return hname == other.hname && os == other.os;
      }

      void clear()
      {
        hname.clear();
        os.clear();
      }
    };

    // Process severity information
    struct ProcessState
    {
      eProcessSeverity                    severity       = proc_sev_unknown;        // severity
      eProcessSeverityLevel               severity_level = proc_sev_level_unknown;  // severity level
      std::string                         info;                                     // info string

      bool operator==(const ProcessState& other) const {
        return severity == other.severity && severity_level == other.severity_level && info == other.info;
      }

      void clear()
      {
        severity = proc_sev_unknown;
        severity_level = proc_sev_level_unknown;
        info.clear();
      }
    };

    // Transport layer parameters for ecal udp multicast
    struct LayerParUdpMC
    {
      bool operator==(const LayerParUdpMC& /*other*/) const {
        // Assuming there are no member variables to compare
        return true;
      }

      void clear()
      {}
    };

    // Transport layer parameters for ecal tcp
    struct LayerParTcp
    {
      int32_t                             port = 0;                     // tcp writers port number

      bool operator==(const LayerParTcp& other) const {
        return port == other.port;
      }

      void clear()
      {
        port = 0;
      }
    };

    // Transport layer parameters for ecal shm
    struct LayerParShm
    {
      Util::CExpandingVector<std::string> memory_file_list;             // list of memory file names

      bool operator==(const LayerParShm& other) const {
        return memory_file_list == other.memory_file_list;
      }

      void clear()
      {
        memory_file_list.clear();
      }
    };

    // Connection parameter for reader/writer
    struct ConnectionPar
    {
      LayerParUdpMC                       layer_par_udpmc;              // parameter for ecal udp multicast
      LayerParTcp                         layer_par_tcp;                // parameter for ecal tcp
      LayerParShm                         layer_par_shm;                // parameter for ecal shm

      bool operator==(const ConnectionPar& other) const {
        return layer_par_udpmc == other.layer_par_udpmc &&
          layer_par_tcp == other.layer_par_tcp &&
          layer_par_shm == other.layer_par_shm;
      }

      void clear()
      {
        layer_par_udpmc.clear();
        layer_par_tcp.clear();
        layer_par_shm.clear();
      }
    };

    // Transport layer information
    struct TLayer
    {
      eTLayerType                         type = tl_none;               // transport layer type
      int32_t                             version = 0;                  // transport layer version
      bool                                enabled = false;              // transport layer enabled ?
      bool                                active = false;               // transport layer in use ?
      ConnectionPar                       par_layer;                    // transport layer parameter

      bool operator==(const TLayer& other) const {
        return type == other.type &&
          version == other.version &&
          enabled == other.enabled &&
          active == other.active &&
          par_layer == other.par_layer;
      }

      void clear()
      {
        type = tl_none;
        version = 0;
        enabled = false;
        active = false;
        par_layer.clear();
      }
    };

    // Process information
    struct Process
    {
      int32_t                             rclock = 0;                   // registration clock
      std::string                         hgname;                       // host group name
      std::string                         pname;                        // process name
      std::string                         uname;                        // unit name
      std::string                         pparam;                       // process parameter
      ProcessState                        state;                        // process state info
      eTSyncState                         tsync_state = tsync_none;     // time synchronization state
      std::string                         tsync_mod_name;               // time synchronization module name
      int32_t                             component_init_state = 0;     // eCAL component initialization state (eCAL::Initialize(..))
      std::string                         component_init_info;          // like comp_init_state as a human-readable string (pub|sub|srv|mon|log|time|proc)
      std::string                         ecal_runtime_version;         // loaded/runtime eCAL version of a component

      bool operator==(const Process& other) const {
        return rclock == other.rclock &&
          hgname == other.hgname &&
          pname == other.pname &&
          uname == other.uname &&
          pparam == other.pparam &&
          state == other.state &&
          tsync_state == other.tsync_state &&
          tsync_mod_name == other.tsync_mod_name &&
          component_init_state == other.component_init_state &&
          component_init_info == other.component_init_info &&
          ecal_runtime_version == other.ecal_runtime_version;
      }

      void clear()
      {
        rclock = 0;
        hgname.clear();
        pname.clear();
        uname.clear();
        pparam.clear();
        state.clear();
        tsync_state = tsync_none;
        tsync_mod_name.clear();
        component_init_state = 0;
        component_init_info.clear();
        ecal_runtime_version.clear();
      }
    };

    // eCAL topic information
    struct Topic
    {
      int32_t                             rclock = 0;                   // registration clock (heart beat)
      std::string                         hgname;                       // host group name
      std::string                         pname;                        // process name
      std::string                         uname;                        // unit name
      std::string                         tname;                        // topic name
      std::string                         direction;                    // direction (publisher, subscriber)
      SDataTypeInformation                tdatatype;                    // topic datatype information (encoding & type & description)

      Util::CExpandingVector<TLayer>      tlayer;                       // active topic transport layers and its specific parameter
      int32_t                             tsize = 0;                    // topic size

      int32_t                             connections_loc = 0;          // number of local connected entities
      int32_t                             connections_ext = 0;          // number of external connected entities
      int32_t                             message_drops   = 0;          // dropped messages

      int64_t                             did    = 0;                   // data send id (publisher setid)
      int64_t                             dclock = 0;                   // data clock (send / receive action)
      int32_t                             dfreq  = 0;                   // data frequency (send / receive registrations per second) [mHz]

      std::map<std::string, std::string>  attr;                         // generic topic description

      bool operator==(const Topic& other) const {
        return rclock == other.rclock &&
          hgname == other.hgname &&
          pname == other.pname &&
          uname == other.uname &&
          tname == other.tname &&
          direction == other.direction &&
          tdatatype == other.tdatatype &&
          tlayer == other.tlayer &&
          tsize == other.tsize &&
          connections_loc == other.connections_loc &&
          connections_ext == other.connections_ext &&
          message_drops == other.message_drops &&
          did == other.did &&
          dclock == other.dclock &&
          dfreq == other.dfreq &&
          attr == other.attr;
      }

      void clear()
      {
        rclock = 0;
        hgname.clear();
        pname.clear();
        uname.clear();
        tname.clear();
        direction.clear();
        tdatatype.clear();

        tlayer.clear();
        tsize = 0;

        connections_loc = 0;
        connections_ext = 0;
        message_drops = 0;

        did = 0;
        dclock = 0;
        dfreq = 0;

        attr.clear();
      }
    };

    struct SampleIdentifier
    {
      std::string                        entity_id;                     // unique id within that process
      int32_t                            process_id = 0;                // process id which produced the sample
      std::string                        host_name;                     // host which produced the sample

      // This is a hack that assumes the entity_id is unique within the whole ecal system (which it should be)
      bool operator==(const SampleIdentifier& other) const {
        return entity_id == other.entity_id;
      }

      bool operator<(const SampleIdentifier& other) const
      {
        return entity_id < other.entity_id;
      }

      void clear()
      {
        entity_id.clear();
        process_id = 0;
        host_name.clear();
      }
    };

    // Registration sample
    struct Sample
    {
      SampleIdentifier                    identifier;                   // Unique identifier to see who produced the sample (publisher / subscriber / ...)
      eCmdType                            cmd_type = bct_none;          // registration command type
      Host                                host;                         // host information
      Process                             process;                      // process information
      Service::Service                    service;                      // service information
      Service::Client                     client ;                      // client information
      Topic                               topic;                        // topic information

      bool operator==(const Sample& other) const {
        return identifier == other.identifier &&
          cmd_type == other.cmd_type &&
          host == other.host &&
          process == other.process &&
          service == other.service &&
          client == other.client &&
          topic == other.topic;
      }

      void clear()
      {
        identifier.clear();
        cmd_type = bct_none;
        host.clear();
        process.clear();
        service.clear();
        client.clear();
        topic.clear();
      }
    };

    // Registration sample list
    using SampleList = Util::CExpandingVector<Sample>;
  }
}
