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
#include "registration_generate.h"

#include <string>
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
      service.pname       = GenerateString(10);
      service.uname       = GenerateString(5);
      service.sname       = GenerateString(8);
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
      client.pname   = GenerateString(10);
      client.uname   = GenerateString(5);
      client.sname   = GenerateString(8);
      client.methods.push_back(GenerateMethod());
      client.methods.push_back(GenerateMethod());
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
      layer.enabled   = rand() % 2 == 1;
      layer.active = rand() % 2 == 1;
      return layer;
    }

    // generate Topic
    Topic GenerateTopic()
    {
      Topic topic;
      topic.rclock          = rand() % 1000;
      topic.hgname          = GenerateString(6);
      topic.pname           = GenerateString(10);
      topic.uname           = GenerateString(5);
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

    Process GenerateProcess()
    {
      Process process;
      process.rclock = rand() % 1000;
      process.hgname = GenerateString(6);
      process.pname = GenerateString(10);
      process.uname = GenerateString(5);
      process.pparam = GenerateString(12);
      process.state.severity = static_cast<eProcessSeverity>(rand() % (proc_sev_failed + 1));
      process.state.severity_level = static_cast<eProcessSeverityLevel>(rand() % (proc_sev_level5 + 1));
      process.state.info = GenerateString(10);
      process.tsync_state = static_cast<eTSyncState>(rand() % (tsync_replay + 1));
      process.tsync_mod_name = GenerateString(6);
      process.component_init_state = rand() % 5;
      process.component_init_info = GenerateString(8);
      process.ecal_runtime_version = GenerateString(5);
      return process;
    }

    SampleIdentifier GenerateIdentifier()
    {
      SampleIdentifier identifier;
      identifier.entity_id = GenerateString(7);
      identifier.process_id = rand() % 100;
      identifier.host_name = GenerateString(8);
      return identifier;
    }

    Sample GenerateProcessSample()
    {
      Sample sample;
      sample.cmd_type = bct_reg_process;
      sample.host.hname = GenerateString(8);
      sample.identifier = GenerateIdentifier();
      // Process samples don't have an id internally, hence it must be 0.
      sample.identifier.entity_id = "";
      sample.process = GenerateProcess();
      return sample;
    }

    Sample GenerateTopicSample()
    {
      Sample sample;
      sample.cmd_type = bct_reg_publisher;
      sample.host.hname = GenerateString(8);
      sample.identifier = GenerateIdentifier();
      sample.topic = GenerateTopic();
      return sample;
    }

    Sample GenerateServiceSample()
    {
      Sample sample;
      sample.cmd_type = bct_reg_service;
      sample.host.hname = GenerateString(8);
      sample.identifier = GenerateIdentifier();
      sample.service = GenerateService();
      return sample;
    }

    Sample GenerateClientSample()
    {
      Sample sample;
      sample.cmd_type = bct_reg_client;
      sample.host.hname = GenerateString(8);
      sample.identifier = GenerateIdentifier();
      sample.client = GenerateClient();
      return sample;
    }
  }
}
