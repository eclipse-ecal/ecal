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
 * @file   ecal_struct_sample_registration.h
 * @brief  eCAL registration as struct
**/

#pragma once

#include "ecal_struct_sample_common.h"
#include "ecal_struct_service.h"

#include <ecal/types.h>
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

    enum eTimeSyncState
    {
      tsync_none     = 0,
      tsync_realtime = 1,
      tsync_replay   = 2
    };

    // Operating system details
    struct OSInfo
    {
      std::string                         name;                       // name

      bool operator==(const OSInfo& other) const {
        return name == other.name;
      }

      void clear()
      {
        name.clear();
      }
    };

    // eCAL host
    struct Host
    {
      std::string                         name;                         // host name
      OSInfo                              os;                           // operating system details

      bool operator==(const Host& other) const {
        return name == other.name && os == other.os;
      }

      void clear()
      {
        name.clear();
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
      int32_t                             registration_clock = 0;       // registration clock
      std::string                         shm_transport_domain;         // shm transport domain
      std::string                         process_name;                 // process name
      std::string                         unit_name;                    // unit name
      std::string                         process_parameter;                       // process parameter
      ProcessState                        state;                        // process state info
      eTimeSyncState                      time_sync_state = tsync_none; // time synchronization state
      std::string                         time_sync_module_name;        // time synchronization module name
      int32_t                             component_init_state = 0;     // eCAL component initialization state (eCAL::Initialize(..))
      std::string                         component_init_info;          // like comp_init_state as a human-readable string (pub|sub|srv|mon|log|time|proc)
      std::string                         ecal_runtime_version;         // loaded/runtime eCAL version of a component
      std::string                         config_file_path;             // Path from where the eCAL configuration for this process was loadedloaded/runtime eCAL version of a component

      bool operator==(const Process& other) const {
        return registration_clock == other.registration_clock &&
          shm_transport_domain == other.shm_transport_domain &&
          process_name == other.process_name &&
          unit_name == other.unit_name &&
          process_parameter == other.process_parameter &&
          state == other.state &&
          time_sync_state == other.time_sync_state &&
          time_sync_module_name == other.time_sync_module_name &&
          component_init_state == other.component_init_state &&
          component_init_info == other.component_init_info &&
          ecal_runtime_version == other.ecal_runtime_version &&
          config_file_path == other.config_file_path;
      }

      void clear()
      {
        registration_clock = 0;
        shm_transport_domain.clear();
        process_name.clear();
        unit_name.clear();
        process_parameter.clear();
        state.clear();
        time_sync_state = tsync_none;
        time_sync_module_name.clear();
        component_init_state = 0;
        component_init_info.clear();
        ecal_runtime_version.clear();
        config_file_path.clear();
      }
    };

    // eCAL topic information
    struct Topic
    {
      int32_t                             registration_clock = 0;       // registration clock (heart beat)
      std::string                         shm_transport_domain;         // shm transport domain
      std::string                         process_name;                 // process name
      std::string                         unit_name;                    // unit name
      std::string                         topic_name;                   // topic name
      std::string                         direction;                    // direction (publisher, subscriber)
      SDataTypeInformation                datatype_information;         // topic datatype information (encoding & type & description)

      Util::CExpandingVector<TLayer>      transport_layer;              // active topic transport layers and its specific parameter
      int32_t                             topic_size = 0;               // topic size

      int32_t                             connections_local = 0;        // number of local connected entities
      int32_t                             connections_external = 0;     // number of external connected entities
      int32_t                             message_drops   = 0;          // dropped messages

      int64_t                             data_id    = 0;               // data send id (publisher setid)
      int64_t                             data_clock = 0;               // data clock (send / receive action)
      int32_t                             data_frequency  = 0;                   // data frequency (send / receive registrations per second) [mHz]


      bool operator==(const Topic& other) const {
        return registration_clock == other.registration_clock &&
          shm_transport_domain == other.shm_transport_domain &&
          process_name == other.process_name &&
          unit_name == other.unit_name &&
          topic_name == other.topic_name &&
          direction == other.direction &&
          datatype_information == other.datatype_information &&
          transport_layer == other.transport_layer &&
          topic_size == other.topic_size &&
          connections_local == other.connections_local &&
          connections_external == other.connections_external &&
          message_drops == other.message_drops &&
          data_id == other.data_id &&
          data_clock == other.data_clock &&
          data_frequency == other.data_frequency;
      }

      void clear()
      {
        registration_clock = 0;
        shm_transport_domain.clear();
        process_name.clear();
        unit_name.clear();
        topic_name.clear();
        direction.clear();
        datatype_information.clear();

        transport_layer.clear();
        topic_size = 0;

        connections_local = 0;
        connections_external = 0;
        message_drops = 0;

        data_id = 0;
        data_clock = 0;
        data_frequency = 0;
      }
    };

    struct SampleIdentifier
    {
      uint64_t                           entity_id = 0;                 // unique id within that process
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
        entity_id = 0;
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
