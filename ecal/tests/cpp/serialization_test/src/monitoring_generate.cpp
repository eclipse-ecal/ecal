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

#include <ecal/types/monitoring.h>

#include <cstdlib>

namespace eCAL
{
  std::string GenerateString(size_t length);

  namespace Monitoring
  {
    // generate process
    SProcessMon GenerateProcess()
    {
      SProcessMon process;
      process.rclock               = rand() % 1000;
      process.hname                = GenerateString(10);
      process.hgname               = GenerateString(8);
      process.pid                  = rand() % 1000;
      process.pname                = GenerateString(8);
      process.uname                = GenerateString(6);
      process.pparam               = GenerateString(20);
      process.state_severity       = rand() % 5;
      process.state_severity_level = rand() % 6;
      process.state_info           = GenerateString(15);
      process.tsync_state          = rand() % 3;
      process.tsync_mod_name       = GenerateString(12);
      process.component_init_state = rand() % 10;
      process.component_init_info  = GenerateString(20);
      process.ecal_runtime_version = GenerateString(8);
      return process;
    }

    // generate topic
    STopicMon GenerateTopic(const std::string& direction)
    {
      STopicMon publisher;
      publisher.rclock               = rand() % 1000;
      publisher.hname                = GenerateString(10);
      publisher.hgname               = GenerateString(8);
      publisher.pid                  = rand() % 1000;
      publisher.pname                = GenerateString(8);
      publisher.uname                = GenerateString(6);
      publisher.tid                  = GenerateString(8);
      publisher.tname                = GenerateString(10);
      publisher.direction            = direction;
      publisher.tdatatype.name       = GenerateString(8);
      publisher.tdatatype.encoding   = GenerateString(8);
      publisher.tdatatype.descriptor = GenerateString(10);
      publisher.tlayer.push_back({ tl_ecal_shm, 1, true });
      publisher.tsize                = rand() % 5000;
      publisher.connections_loc      = rand() % 10;
      publisher.connections_ext      = rand() % 10;
      publisher.message_drops        = rand() % 100;
      publisher.did                  = rand() % 10000;
      publisher.dclock               = rand() % 10000;
      publisher.dfreq                = rand() % 100;
      return publisher;
    }

    // generate servicemethod
    SMethodMon GenerateServiceMethod()
    {
      SMethodMon method;
      method.mname      = GenerateString(8);
      method.req_type   = GenerateString(8);
      method.req_desc   = GenerateString(10);
      method.resp_type  = GenerateString(8);
      method.resp_desc  = GenerateString(10);
      method.call_count = rand() % 10000;
      return method;
    }

    // generate service
    SServerMon GenerateService()
    {
      SServerMon server;
      server.rclock      = rand() % 1000;
      server.hname       = GenerateString(10);
      server.pname       = GenerateString(8);
      server.uname       = GenerateString(6);
      server.pid         = rand() % 1000;
      server.sname       = GenerateString(10);
      server.sid         = GenerateString(8);
      server.version     = rand() % 100;
      server.tcp_port_v0 = rand() % 65536;
      server.tcp_port_v1 = rand() % 65536;

      server.methods.push_back(GenerateServiceMethod());
      server.methods.push_back(GenerateServiceMethod());
      server.methods.push_back(GenerateServiceMethod());
      return server;
    }

    // generate client
    SClientMon GenerateClient()
    {
      SClientMon client;
      client.rclock  = rand() % 1000;
      client.hname   = GenerateString(10);
      client.pname   = GenerateString(8);
      client.uname   = GenerateString(6);
      client.pid     = rand() % 1000;
      client.sname   = GenerateString(10);
      client.sid     = GenerateString(8);
      client.version = rand() % 100;
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
