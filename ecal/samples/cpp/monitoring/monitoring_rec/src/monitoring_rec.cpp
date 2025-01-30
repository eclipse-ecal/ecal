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

#include <ecal/ecal.h>

#include <iostream>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <ecal/core/pb/monitoring.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace
{
  int          g_mon_timing           = 1000;  // refresh time in ms
  bool         g_do_monitor_hosts     = true;  // monitor host infos
  bool         g_do_monitor_procs     = true;  // monitor process infos
  bool         g_do_monitor_services  = true;  // monitor service infos
  bool         g_do_monitor_topics    = true;  // monitor topic infos
  std::string  g_host_filt_string     = "";    // specify host name to monitor ("" == all hosts)
  std::string  g_procs_filt_string    = "";    // specify process name to monitor ("" == all processes)
  std::string  g_topics_filt_string   = "";    // specify topic name to monitor ("" == all topics)
  std::string  g_services_filt_string = "";    // specify service name to monitor ("" == all services)
}

int main()
{
  // initialize eCAL core API
  eCAL::Initialize("monitoring", eCAL::Init::All);

  // monitoring instance to store snapshot
  eCAL::pb::Monitoring monitoring;
  std::string          monitoring_s;

  // monitor for ever
  while (eCAL::Ok())
  {
    // monitor hosts ?
    if (g_do_monitor_hosts)
    {
      // take snapshot :-)
      eCAL::Monitoring::GetMonitoring(monitoring_s, eCAL::Monitoring::Entity::Host);
      monitoring.ParseFromString(monitoring_s);

      // collect host infos
      std::cout << "-------- HOSTS ----------" << "\n";

      // for all hosts
      for (const auto& host : monitoring.hosts())
      {
        // check filtering
        if (!g_host_filt_string.empty() && (g_host_filt_string != host.name())) continue;

        // print host details
        std::cout << "host.name       : " << host.name()      << "\n";   // host name
        std::cout << "os.name         : " << host.os().name() << "\n";   // operating system details
        std::cout << "\n";
      }
    }

    // monitor processes ?
    if (g_do_monitor_procs)
    {
      // take snapshot :-)
      eCAL::Monitoring::GetMonitoring(monitoring_s, eCAL::Monitoring::Entity::Process);
      monitoring.ParseFromString(monitoring_s);

      // collect process infos
      std::cout << "------- PROCESSES -------" << "\n";

      // for all processes
      for (const auto& process : monitoring.processes())
      {
        // check filtering
        if (!g_procs_filt_string.empty() && (g_procs_filt_string != process.unit_name())) continue;

        // print process details
        std::cout << "host name         : " << process.host_name()             << "\n";   // host name
        std::cout << "process id        : " << process.process_id()            << "\n";   // process id
        std::cout << "process name      : " << process.process_name()          << "\n";   // process name
        std::cout << "unit name         : " << process.unit_name()             << "\n";   // unit name
        std::cout << "process parameter : " << process.process_parameter()     << "\n";   // process parameter
        std::cout << "severity          : " << process.state().severity()      << "\n";   // process state severity
        std::cout << "info              : " << process.state().info()          << "\n";   // process state info
        std::cout << "tsync state       : " << process.time_sync_state()       << "\n";   // time sync state
        std::cout << "tsync mod name    : " << process.time_sync_module_name() << "\n";   // time sync mod name
        std::cout << "\n";
      }
    }

    // monitor services ?
    if (g_do_monitor_services)
    {
      // take snapshot :-)
      eCAL::Monitoring::GetMonitoring(monitoring_s, eCAL::Monitoring::Entity::Server | eCAL::Monitoring::Entity::Client);
      monitoring.ParseFromString(monitoring_s);

      // collect process infos
      std::cout << "------- SERVICES -------" << "\n";

      // for all processes
      for (const auto& service : monitoring.services())
      {
        // check filtering
        if (!g_services_filt_string.empty() && (g_services_filt_string != service.service_name())) continue;

        // print process details
        std::cout << "host name       : " << service.host_name()  << "\n";   // host name
        std::cout << "service name    : " << service.service_name()      << "\n";   // service name
        std::cout << "process id      : " << service.process_id() << "\n";   // process id
        std::cout << "\n";

        for (int i = 0; i < service.methods_size(); ++i)
        {
          auto method = service.methods(i);
          std::cout << "  method name     : " << method.method_name() << "\n";   // method name
          std::cout << "  req_type        : " << method.req_type()    << "\n";   // request type
          //std::cout << "  req_desc        : " << method.req_desc()   << "\n";   // request descriptor
          std::cout << "  resp_type       : " << method.resp_type()   << "\n";   // response type
          //std::cout << "  resp_desc       : " << method.resp_desc()  << "\n";   // response descriptor
          std::cout << "  call_count      : " << method.call_count()  << "\n";   // call count
          std::cout << "\n";
        }
        std::cout << "\n";
      }
    }

    // monitor topics ?
    if (g_do_monitor_topics)
    {
      // take snapshot :-)
      eCAL::Monitoring::GetMonitoring(monitoring_s, eCAL::Monitoring::Entity::Publisher | eCAL::Monitoring::Entity::Subscriber);
      monitoring.ParseFromString(monitoring_s);

      // collect topic infos
      std::cout << "-------- TOPICS ---------" << "\n";

      // for all topics
      for (const auto& topic : monitoring.topics())
      {
        // check filtering
        if (!g_topics_filt_string.empty() && (g_topics_filt_string != topic.topic_name())) continue;

        // print topic details
        std::cout << "registration clock : " << topic.registration_clock()              << "\n";   // registration clock
        std::cout << "host name          : " << topic.host_name()                       << "\n";   // host name
        std::cout << "process id         : " << topic.process_id()                      << "\n";   // process id
        std::cout << "process name       : " << topic.process_name()                    << "\n";   // process name
        std::cout << "unit name          : " << topic.unit_name()                       << "\n";   // unit name
        std::cout << "topic id           : " << topic.topic_id()                        << "\n";   // topic id
        std::cout << "topic name         : " << topic.topic_name()                      << "\n";   // topic name
        std::cout << "direction          : " << topic.direction()                       << "\n";   // direction (publisher, subscriber)
        std::cout << "ttype name         : " << topic.datatype_information().name()     << "\n";   // topic type name
        std::cout << "ttype encoding     : " << topic.datatype_information().encoding() << "\n";   // topic type encoding
        //std::cout << "tdesc           : " << topic.datatype_information().descriptor_information()     << "\n";   // topic description
        for (const auto& layer : topic.transport_layer())
        {
          std::string layer_type("unknown");
          switch (layer.type())
          {
          case eCAL::pb::eTransportLayerType::tl_none:
            layer_type = "tlayer_none";
            break;
          case eCAL::pb::eTransportLayerType::tl_ecal_udp_mc:
            layer_type = "tlayer_udp_mc";
            break;
          case eCAL::pb::eTransportLayerType::tl_ecal_shm:
            layer_type = "tlayer_shm";
            break;
          case eCAL::pb::eTransportLayerType::tl_ecal_tcp:
            layer_type = "tlayer_tcp";
            break;
          case eCAL::pb::eTransportLayerType::tl_all:
            layer_type = "tlayer_all";
            break;
          default:
            break;
          }
          std::cout << "  transport_layer.type    : " << layer_type                   << "\n";   // transport layers type
          std::cout << "  transport_layer.active  : " << layer.active()               << "\n";   // transport layers confirmation
        }           
        std::cout << "topic size                  : " << topic.topic_size()           << "\n";   // topic size

        std::cout << "connections local           : " << topic.connections_local()    << "\n";   // number of local connected entities
        std::cout << "connections external        : " << topic.connections_external() << "\n";   // number of external connected entities
        std::cout << "message drops               : " << topic.message_drops()        << "\n";   // dropped messages

        std::cout << "data id                     : " << topic.data_id()              << "\n";   // data send id (publisher setid)
        std::cout << "data clock                  : " << topic.data_clock()           << "\n";   // data clock (send / receive action)
        std::cout << "data frequency              : " << topic.data_frequency()       << "\n";   // data frequency (send / receive samples per second * 1000)

        std::cout << "\n";
      }
    }

    // sleep few milliseconds
    eCAL::Process::SleepMS(g_mon_timing);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
