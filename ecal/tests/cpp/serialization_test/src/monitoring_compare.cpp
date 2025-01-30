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
      if (monitoring1.publisher.size() != monitoring2.publisher.size())
      {
        return false;
      }

      for (size_t i = 0; i < monitoring1.publisher.size(); ++i)
      {
        if (monitoring1.publisher[i].registration_clock != monitoring2.publisher[i].registration_clock ||
          monitoring1.publisher[i].host_name != monitoring2.publisher[i].host_name ||
          monitoring1.publisher[i].shm_transport_domain != monitoring2.publisher[i].shm_transport_domain ||
          monitoring1.publisher[i].process_id != monitoring2.publisher[i].process_id ||
          monitoring1.publisher[i].process_name != monitoring2.publisher[i].process_name ||
          monitoring1.publisher[i].unit_name != monitoring2.publisher[i].unit_name ||
          monitoring1.publisher[i].topic_id != monitoring2.publisher[i].topic_id ||
          monitoring1.publisher[i].topic_name != monitoring2.publisher[i].topic_name ||
          monitoring1.publisher[i].direction != monitoring2.publisher[i].direction ||
          monitoring1.publisher[i].datatype_information != monitoring2.publisher[i].datatype_information ||
          monitoring1.publisher[i].transport_layer.size() != monitoring2.publisher[i].transport_layer.size() ||
          monitoring1.publisher[i].topic_size != monitoring2.publisher[i].topic_size ||
          monitoring1.publisher[i].connections_local != monitoring2.publisher[i].connections_local ||
          monitoring1.publisher[i].connections_external != monitoring2.publisher[i].connections_external ||
          monitoring1.publisher[i].message_drops != monitoring2.publisher[i].message_drops ||
          monitoring1.publisher[i].data_id != monitoring2.publisher[i].data_id ||
          monitoring1.publisher[i].data_clock != monitoring2.publisher[i].data_clock ||
          monitoring1.publisher[i].data_frequency != monitoring2.publisher[i].data_frequency)
        {
          return false;
        }
      }

      // compare subscriber info
      if (monitoring1.subscriber.size() != monitoring2.subscriber.size())
      {
        return false;
      }

      for (size_t i = 0; i < monitoring1.subscriber.size(); ++i)
      {
        if (monitoring1.subscriber[i].registration_clock != monitoring2.subscriber[i].registration_clock ||
          monitoring1.subscriber[i].host_name != monitoring2.subscriber[i].host_name ||
          monitoring1.subscriber[i].shm_transport_domain != monitoring2.subscriber[i].shm_transport_domain ||
          monitoring1.subscriber[i].process_id != monitoring2.subscriber[i].process_id ||
          monitoring1.subscriber[i].process_name != monitoring2.subscriber[i].process_name ||
          monitoring1.subscriber[i].unit_name != monitoring2.subscriber[i].unit_name ||
          monitoring1.subscriber[i].topic_id != monitoring2.subscriber[i].topic_id ||
          monitoring1.subscriber[i].topic_name != monitoring2.subscriber[i].topic_name ||
          monitoring1.subscriber[i].direction != monitoring2.subscriber[i].direction ||
          monitoring1.subscriber[i].datatype_information != monitoring2.subscriber[i].datatype_information ||
          monitoring1.subscriber[i].transport_layer.size() != monitoring2.subscriber[i].transport_layer.size() ||
          monitoring1.subscriber[i].topic_size != monitoring2.subscriber[i].topic_size ||
          monitoring1.subscriber[i].connections_local != monitoring2.subscriber[i].connections_local ||
          monitoring1.subscriber[i].connections_external != monitoring2.subscriber[i].connections_external ||
          monitoring1.subscriber[i].message_drops != monitoring2.subscriber[i].message_drops ||
          monitoring1.subscriber[i].data_id != monitoring2.subscriber[i].data_id ||
          monitoring1.subscriber[i].data_clock != monitoring2.subscriber[i].data_clock ||
          monitoring1.subscriber[i].data_frequency != monitoring2.subscriber[i].data_frequency)
        {
          return false;
        }
      }

      // compare server info
      if (monitoring1.server.size() != monitoring2.server.size())
      {
        return false;
      }

      for (size_t i = 0; i < monitoring1.server.size(); ++i)
      {
        if (monitoring1.server[i].registration_clock != monitoring2.server[i].registration_clock ||
          monitoring1.server[i].host_name != monitoring2.server[i].host_name ||
          monitoring1.server[i].process_name != monitoring2.server[i].process_name ||
          monitoring1.server[i].unit_name != monitoring2.server[i].unit_name ||
          monitoring1.server[i].process_id != monitoring2.server[i].process_id ||
          monitoring1.server[i].service_name != monitoring2.server[i].service_name ||
          monitoring1.server[i].service_id != monitoring2.server[i].service_id ||
          monitoring1.server[i].version != monitoring2.server[i].version ||
          monitoring1.server[i].tcp_port_v0 != monitoring2.server[i].tcp_port_v0 ||
          monitoring1.server[i].tcp_port_v1 != monitoring2.server[i].tcp_port_v1 ||
          monitoring1.server[i].methods.size() != monitoring2.server[i].methods.size())
        {
          return false;
        }

        for (size_t j = 0; j < monitoring1.server[i].methods.size(); ++j)
        {
          if (monitoring1.server[i].methods[j].method_name != monitoring2.server[i].methods[j].method_name ||
            monitoring1.server[i].methods[j].request_datatype_information != monitoring2.server[i].methods[j].request_datatype_information ||
            monitoring1.server[i].methods[j].response_datatype_information != monitoring2.server[i].methods[j].response_datatype_information ||
            monitoring1.server[i].methods[j].call_count != monitoring2.server[i].methods[j].call_count)
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
