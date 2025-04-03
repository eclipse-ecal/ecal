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

#include "monitoring_generate.h"
#include "registration_generate.h"
#include "common_generate.h"

#include <cstdlib>
#include <string>

namespace eCAL
{
  namespace Monitoring
  {
    // generate process
    SProcessMon GenerateProcess()
    {
      SProcessMon process;
      process.registration_clock    = rand() % 1000;
      process.host_name             = GenerateString(10);
      process.shm_transport_domain  = GenerateString(8);
      process.process_id            = rand() % 1000;
      process.process_name          = GenerateString(8);
      process.unit_name             = GenerateString(6);
      process.process_parameter     = GenerateString(20);
      process.state_severity        = rand() % 5;
      process.state_severity_level  = rand() % 6;
      process.state_info            = GenerateString(15);
      process.time_sync_state       = rand() % 3;
      process.time_sync_module_name = GenerateString(12);
      process.component_init_state  = rand() % 10;
      process.component_init_info   = GenerateString(20);
      process.ecal_runtime_version  = GenerateString(8);
      process.config_file_path      = GenerateString(20);
      return process;
    }

    // generate topic
    STopicMon GenerateTopic(const std::string& direction)
    {
      STopicMon topic;
      topic.registration_clock   = rand() % 1000;
      topic.host_name            = GenerateString(10);
      topic.shm_transport_domain = GenerateString(8);
      topic.process_id           = rand() % 1000;
      topic.process_name         = GenerateString(8);
      topic.unit_name            = GenerateString(6);
      topic.topic_id             = rand();
      topic.topic_name           = GenerateString(10);
      topic.direction            = direction;
      topic.datatype_information = eCAL::Registration::GenerateDataTypeInformation();
      topic.transport_layer.push_back({ eTransportLayerType::shm, 1, true });
      topic.topic_size           = rand() % 5000;
      topic.connections_local    = rand() % 10;
      topic.connections_external = rand() % 10;
      topic.message_drops        = rand() % 100;
      topic.data_id              = rand() % 10000;
      topic.data_clock           = rand() % 10000;
      topic.data_frequency       = rand() % 100;
      return topic;
    }

    // generate servicemethod
    SMethodMon GenerateServiceMethod()
    {
      SMethodMon method;
      method.method_name = GenerateString(8);

      method.request_datatype_information  = eCAL::Registration::GenerateDataTypeInformation();
      method.response_datatype_information = eCAL::Registration::GenerateDataTypeInformation();

      method.call_count = rand() % 10000;
      return method;
    }

    // generate service
    SServerMon GenerateService()
    {
      SServerMon server;
      server.registration_clock = rand() % 1000;
      server.host_name          = GenerateString(10);
      server.process_name       = GenerateString(8);
      server.unit_name          = GenerateString(6);
      server.process_id         = rand() % 1000;
      server.service_name       = GenerateString(10);
      server.service_id         = rand();
      server.version            = rand() % 100;
      server.tcp_port_v0        = rand() % 65536;
      server.tcp_port_v1        = rand() % 65536;

      server.methods.push_back(GenerateServiceMethod());
      server.methods.push_back(GenerateServiceMethod());
      server.methods.push_back(GenerateServiceMethod());
      return server;
    }

    // generate client
    SClientMon GenerateClient()
    {
      SClientMon client;
      client.registration_clock = rand() % 1000;
      client.host_name          = GenerateString(10);
      client.process_name       = GenerateString(8);
      client.unit_name          = GenerateString(6);
      client.process_id         = rand() % 1000;
      client.service_name       = GenerateString(10);
      client.service_id         = rand();
      client.methods.push_back(GenerateServiceMethod());
      client.methods.push_back(GenerateServiceMethod());
      client.methods.push_back(GenerateServiceMethod());
      client.version            = rand() % 100;
      return client;
    }

    // generate monitoring
    SMonitoring GenerateMonitoring()
    {
      SMonitoring monitoring;

      // generate process
      monitoring.processes.push_back(GenerateProcess());
      monitoring.processes.push_back(GenerateProcess());
      monitoring.processes.push_back(GenerateProcess());

      // generate topics
      monitoring.publisher.push_back(GenerateTopic("publisher"));
      monitoring.publisher.push_back(GenerateTopic("publisher"));
      monitoring.publisher.push_back(GenerateTopic("publisher"));
      monitoring.subscriber.push_back(GenerateTopic("subscriber"));
      monitoring.subscriber.push_back(GenerateTopic("subscriber"));
      monitoring.subscriber.push_back(GenerateTopic("subscriber"));

      // generate services
      monitoring.server.push_back(GenerateService());
      monitoring.server.push_back(GenerateService());
      monitoring.server.push_back(GenerateService());

      // generate clients
      monitoring.clients.push_back(GenerateClient());
      monitoring.clients.push_back(GenerateClient());
      monitoring.clients.push_back(GenerateClient());

      return monitoring;
    }
  }
}
