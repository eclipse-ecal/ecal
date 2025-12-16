/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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

#include "monitoring_compare.h"

#include <cstddef>
#include <ecal/types/monitoring.h>

namespace eCAL
{
  namespace Monitoring
  {
    // compare two monitoring structs
    bool CompareMonitorings(const SMonitoring& monitoring1, const SMonitoring& monitoring2)
    {
      // compare process info
      if (monitoring1.processes.size() != monitoring2.processes.size())
      {
        return false;
      }

      for (size_t i = 0; i < monitoring1.processes.size(); ++i)
      {
        if (monitoring1.processes[i].registration_clock != monitoring2.processes[i].registration_clock ||
          monitoring1.processes[i].host_name != monitoring2.processes[i].host_name ||
          monitoring1.processes[i].shm_transport_domain != monitoring2.processes[i].shm_transport_domain ||
          monitoring1.processes[i].process_id != monitoring2.processes[i].process_id ||
          monitoring1.processes[i].process_name != monitoring2.processes[i].process_name ||
          monitoring1.processes[i].unit_name != monitoring2.processes[i].unit_name ||
          monitoring1.processes[i].process_parameter != monitoring2.processes[i].process_parameter ||
          monitoring1.processes[i].state_severity != monitoring2.processes[i].state_severity ||
          monitoring1.processes[i].state_severity_level != monitoring2.processes[i].state_severity_level ||
          monitoring1.processes[i].state_info != monitoring2.processes[i].state_info ||
          monitoring1.processes[i].time_sync_state != monitoring2.processes[i].time_sync_state ||
          monitoring1.processes[i].time_sync_module_name != monitoring2.processes[i].time_sync_module_name ||
          monitoring1.processes[i].component_init_state != monitoring2.processes[i].component_init_state ||
          monitoring1.processes[i].component_init_info != monitoring2.processes[i].component_init_info ||
          monitoring1.processes[i].ecal_runtime_version != monitoring2.processes[i].ecal_runtime_version ||
          monitoring1.processes[i].config_file_path != monitoring2.processes[i].config_file_path
          )
        {
          return false;
        }
      }

      // compare publisher info
      if (monitoring1.publishers.size() != monitoring2.publishers.size())
      {
        return false;
      }

      for (size_t i = 0; i < monitoring1.publishers.size(); ++i)
      {
        if (monitoring1.publishers[i].registration_clock != monitoring2.publishers[i].registration_clock ||
          monitoring1.publishers[i].host_name != monitoring2.publishers[i].host_name ||
          monitoring1.publishers[i].shm_transport_domain != monitoring2.publishers[i].shm_transport_domain ||
          monitoring1.publishers[i].process_id != monitoring2.publishers[i].process_id ||
          monitoring1.publishers[i].process_name != monitoring2.publishers[i].process_name ||
          monitoring1.publishers[i].unit_name != monitoring2.publishers[i].unit_name ||
          monitoring1.publishers[i].topic_id != monitoring2.publishers[i].topic_id ||
          monitoring1.publishers[i].topic_name != monitoring2.publishers[i].topic_name ||
          monitoring1.publishers[i].direction != monitoring2.publishers[i].direction ||
          monitoring1.publishers[i].datatype_information != monitoring2.publishers[i].datatype_information ||
          monitoring1.publishers[i].transport_layer.size() != monitoring2.publishers[i].transport_layer.size() ||
          monitoring1.publishers[i].topic_size != monitoring2.publishers[i].topic_size ||
          monitoring1.publishers[i].connections_local != monitoring2.publishers[i].connections_local ||
          monitoring1.publishers[i].connections_external != monitoring2.publishers[i].connections_external ||
          monitoring1.publishers[i].message_drops != monitoring2.publishers[i].message_drops ||
          monitoring1.publishers[i].data_id != monitoring2.publishers[i].data_id ||
          monitoring1.publishers[i].data_clock != monitoring2.publishers[i].data_clock ||
          monitoring1.publishers[i].data_frequency != monitoring2.publishers[i].data_frequency)
        {
          return false;
        }
      }

      // compare subscriber info
      if (monitoring1.subscribers.size() != monitoring2.subscribers.size())
      {
        return false;
      }

      for (size_t i = 0; i < monitoring1.subscribers.size(); ++i)
      {
        if (monitoring1.subscribers[i].registration_clock != monitoring2.subscribers[i].registration_clock ||
          monitoring1.subscribers[i].host_name != monitoring2.subscribers[i].host_name ||
          monitoring1.subscribers[i].shm_transport_domain != monitoring2.subscribers[i].shm_transport_domain ||
          monitoring1.subscribers[i].process_id != monitoring2.subscribers[i].process_id ||
          monitoring1.subscribers[i].process_name != monitoring2.subscribers[i].process_name ||
          monitoring1.subscribers[i].unit_name != monitoring2.subscribers[i].unit_name ||
          monitoring1.subscribers[i].topic_id != monitoring2.subscribers[i].topic_id ||
          monitoring1.subscribers[i].topic_name != monitoring2.subscribers[i].topic_name ||
          monitoring1.subscribers[i].direction != monitoring2.subscribers[i].direction ||
          monitoring1.subscribers[i].datatype_information != monitoring2.subscribers[i].datatype_information ||
          monitoring1.subscribers[i].transport_layer.size() != monitoring2.subscribers[i].transport_layer.size() ||
          monitoring1.subscribers[i].topic_size != monitoring2.subscribers[i].topic_size ||
          monitoring1.subscribers[i].connections_local != monitoring2.subscribers[i].connections_local ||
          monitoring1.subscribers[i].connections_external != monitoring2.subscribers[i].connections_external ||
          monitoring1.subscribers[i].message_drops != monitoring2.subscribers[i].message_drops ||
          monitoring1.subscribers[i].data_id != monitoring2.subscribers[i].data_id ||
          monitoring1.subscribers[i].data_clock != monitoring2.subscribers[i].data_clock ||
          monitoring1.subscribers[i].data_frequency != monitoring2.subscribers[i].data_frequency ||
          monitoring1.subscribers[i].data_frequency != monitoring2.subscribers[i].data_frequency ||
          monitoring1.subscribers[i].data_latency_us.count != monitoring2.subscribers[i].data_latency_us.count || 
          monitoring1.subscribers[i].data_latency_us.latest != monitoring2.subscribers[i].data_latency_us.latest ||
          monitoring1.subscribers[i].data_latency_us.min != monitoring2.subscribers[i].data_latency_us.min ||
          monitoring1.subscribers[i].data_latency_us.max != monitoring2.subscribers[i].data_latency_us.max ||
          monitoring1.subscribers[i].data_latency_us.mean != monitoring2.subscribers[i].data_latency_us.mean ||
          monitoring1.subscribers[i].data_latency_us.variance != monitoring2.subscribers[i].data_latency_us.variance
          )
        {
          return false;
        }
      }

      // compare server info
      if (monitoring1.servers.size() != monitoring2.servers.size())
      {
        return false;
      }

      for (size_t i = 0; i < monitoring1.servers.size(); ++i)
      {
        if (monitoring1.servers[i].registration_clock != monitoring2.servers[i].registration_clock ||
          monitoring1.servers[i].host_name != monitoring2.servers[i].host_name ||
          monitoring1.servers[i].process_name != monitoring2.servers[i].process_name ||
          monitoring1.servers[i].unit_name != monitoring2.servers[i].unit_name ||
          monitoring1.servers[i].process_id != monitoring2.servers[i].process_id ||
          monitoring1.servers[i].service_name != monitoring2.servers[i].service_name ||
          monitoring1.servers[i].service_id != monitoring2.servers[i].service_id ||
          monitoring1.servers[i].version != monitoring2.servers[i].version ||
          monitoring1.servers[i].tcp_port_v0 != monitoring2.servers[i].tcp_port_v0 ||
          monitoring1.servers[i].tcp_port_v1 != monitoring2.servers[i].tcp_port_v1 ||
          monitoring1.servers[i].methods.size() != monitoring2.servers[i].methods.size())
        {
          return false;
        }

        for (size_t j = 0; j < monitoring1.servers[i].methods.size(); ++j)
        {
          if (monitoring1.servers[i].methods[j].method_name != monitoring2.servers[i].methods[j].method_name ||
            monitoring1.servers[i].methods[j].request_datatype_information != monitoring2.servers[i].methods[j].request_datatype_information ||
            monitoring1.servers[i].methods[j].response_datatype_information != monitoring2.servers[i].methods[j].response_datatype_information ||
            monitoring1.servers[i].methods[j].call_count != monitoring2.servers[i].methods[j].call_count)
          {
            return false;
          }
        }
      }

      // compare client info
      if (monitoring1.clients.size() != monitoring2.clients.size())
      {
        return false;
      }

      for (size_t i = 0; i < monitoring1.clients.size(); ++i)
      {
        if (monitoring1.clients[i].registration_clock != monitoring2.clients[i].registration_clock ||
          monitoring1.clients[i].host_name != monitoring2.clients[i].host_name ||
          monitoring1.clients[i].process_name != monitoring2.clients[i].process_name ||
          monitoring1.clients[i].unit_name != monitoring2.clients[i].unit_name ||
          monitoring1.clients[i].process_id != monitoring2.clients[i].process_id ||
          monitoring1.clients[i].service_name != monitoring2.clients[i].service_name ||
          monitoring1.clients[i].service_id != monitoring2.clients[i].service_id ||
          monitoring1.clients[i].methods.size() != monitoring2.clients[i].methods.size() ||
          monitoring1.clients[i].version != monitoring2.clients[i].version)
        {
          return false;
        }

        for (size_t j = 0; j < monitoring1.clients[i].methods.size(); ++j)
        {
          if (monitoring1.clients[i].methods[j].method_name != monitoring2.clients[i].methods[j].method_name ||
            monitoring1.clients[i].methods[j].request_datatype_information != monitoring2.clients[i].methods[j].request_datatype_information ||
            monitoring1.clients[i].methods[j].response_datatype_information != monitoring2.clients[i].methods[j].response_datatype_information ||
            monitoring1.clients[i].methods[j].call_count != monitoring2.clients[i].methods[j].call_count)
          {
            return false;
          }
        }
      }

      return true;
    }
  }
}
