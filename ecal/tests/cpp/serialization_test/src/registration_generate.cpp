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

#include "../../serialization/ecal_struct_sample_registration.h"

#include <cstdlib>

namespace eCAL
{
  std::string GenerateString(size_t length);

  namespace Registration
  {
    // generate Method
    Service::Method GenerateMethod()
    {
      Service::Method method;
      method.mname      = GenerateString(6);
      method.req_type   = GenerateString(8);
      method.req_desc   = GenerateString(10);
      method.resp_type  = GenerateString(8);
      method.resp_desc  = GenerateString(10);
      method.call_count = rand() % 100;

      return method;
    }

    // generate Service
    Service::Service GenerateService()
    {
      Service::Service service;
      service.rclock      = rand() % 1000;
      service.hname       = GenerateString(8);
      service.pname       = GenerateString(10);
      service.uname       = GenerateString(5);
      service.pid         = rand() % 100;
      service.sname       = GenerateString(8);
      service.sid         = GenerateString(7);
      service.methods.push_back(GenerateMethod());
      service.methods.push_back(GenerateMethod());
      service.version     = rand() % 10;
      service.tcp_port_v0 = rand() % 1000;
      service.tcp_port_v1 = rand() % 1000;

      return service;
    }

    // generate Client
    Service::Client GenerateClient()
    {
      Service::Client client;
      client.rclock  = rand() % 1000;
      client.hname   = GenerateString(8);
      client.pname   = GenerateString(10);
      client.uname   = GenerateString(5);
      client.pid     = rand() % 100;
      client.sname   = GenerateString(8);
      client.sid     = GenerateString(7);
      client.version = rand() % 10;

      return client;
    }

    // generate SDataTypeInformation
    SDataTypeInformation GenerateDataTypeInformation()
    {
      SDataTypeInformation dt;
      dt.name       = GenerateString(8);
      dt.encoding   = GenerateString(6);
      dt.descriptor = GenerateString(10);
      return dt;
    }

    // generate TLayer
    TLayer GenerateTLayer()
    {
      TLayer layer;
      layer.type      = static_cast<eTLayerType>(rand() % (tl_all + 1));
      layer.version   = rand() % 100;
      layer.confirmed = rand() % 2 == 1;
      return layer;
    }

    // generate Topic
    Topic GenerateTopic()
    {
      Topic topic;
      topic.rclock          = rand() % 1000;
      topic.hname           = GenerateString(8);
      topic.hgname          = GenerateString(6);
      topic.pid             = rand() % 100;
      topic.pname           = GenerateString(10);
      topic.uname           = GenerateString(5);
      topic.tid             = GenerateString(7);
      topic.tname           = GenerateString(8);
      topic.direction       = GenerateString(5);
      topic.tdatatype       = GenerateDataTypeInformation();
      topic.tlayer.push_back(GenerateTLayer());
      topic.tlayer.push_back(GenerateTLayer());
      topic.tsize           = rand() % 1000;
      topic.connections_loc = rand() % 50;
      topic.connections_ext = rand() % 50;
      topic.message_drops   = rand() % 10;
      topic.did             = rand();
      topic.dclock          = rand();
      topic.dfreq           = rand() % 100;
      return topic;
    }

    // generate Registration Sample
    Sample GenerateRegistrationSample()
    {
      Sample sample;
      sample.cmd_type                     = static_cast<eCmdType>(rand() % (bct_unreg_client + 1));
      sample.host.hname                   = GenerateString(8);
      sample.process.rclock               = rand() % 1000;
      sample.process.hname                = GenerateString(8);
      sample.process.hgname               = GenerateString(6);
      sample.process.pid                  = rand() % 100;
      sample.process.pname                = GenerateString(10);
      sample.process.uname                = GenerateString(5);
      sample.process.pparam               = GenerateString(12);
      sample.process.state.severity       = static_cast<eProcessSeverity>(rand() % (proc_sev_failed + 1));
      sample.process.state.severity_level = static_cast<eProcessSeverityLevel>(rand() % (proc_sev_level5 + 1));
      sample.process.state.info           = GenerateString(10);
      sample.process.tsync_state          = static_cast<eTSyncState>(rand() % (tsync_replay + 1));
      sample.process.tsync_mod_name       = GenerateString(6);
      sample.process.component_init_state = rand() % 5;
      sample.process.component_init_info  = GenerateString(8);
      sample.process.ecal_runtime_version = GenerateString(5);
      sample.service                      = GenerateService();
      sample.client                       = GenerateClient();
      sample.topic                        = GenerateTopic();

      return sample;
    }
  }
}
