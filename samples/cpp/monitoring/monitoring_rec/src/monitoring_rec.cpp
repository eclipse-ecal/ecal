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

#include <ecal/ecal.h>

#include <iostream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4800) // disable proto warnings
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

static int          g_mon_timing           = 1000;   // refresh time in ms
static bool         g_do_monitor_hosts     = true;   // monitor host infos
static bool         g_do_monitor_procs     = true;   // monitor process infos
static bool         g_do_monitor_services  = true;   // monitor service infos
static bool         g_do_monitor_topics    = true;   // monitor topic infos
static std::string  g_host_filt_string     = "";     // specify host name to monitor ("" == all hosts)
static std::string  g_procs_filt_string    = "";     // specify process name to monitor ("" == all processes)
static std::string  g_topics_filt_string   = "";     // specify topic name to monitor ("" == all topics)
static std::string  g_services_filt_string = "";     // specify service name to monitor ("" == all services)

int main(int argc, char **argv)
{
  // initialize eCAL core API
  eCAL::Initialize(argc, argv, "monitoring", eCAL::Init::All);

  // set filtering exclusive regex
  eCAL::Monitoring::SetExclFilter("ecal.*");

  // set filtering inclusive regex
  eCAL::Monitoring::SetInclFilter("");

  // set filtering on
  eCAL::Monitoring::SetFilterState(true);

  // activate monitoring publisher
  eCAL::Monitoring::PubMonitoring(true);

  // activate logging publisher
  eCAL::Monitoring::PubLogging(true);

  // monitoring instance to store complete snapshot
  eCAL::pb::Monitoring monitoring;
  std::string          monitoring_s;

  // monitor for ever
  while(eCAL::Ok())
  {
    // take snapshot :-)
    eCAL::Monitoring::GetMonitoring(monitoring_s);
    monitoring.ParseFromString(monitoring_s);

    // monitor hosts ?
    if(g_do_monitor_hosts)
    {
      // collect host infos
      std::cout << "-------- HOSTS ----------" << std::endl;

      // for all hosts
      for(auto host : monitoring.hosts())
      {
        // check filtering
        if(!g_host_filt_string.empty() && (g_host_filt_string != host.hname())) continue;

        // print host details
        std::cout << "hname           : " << host.hname()         << std::endl;   // host name
        std::cout << "os.osname       : " << host.os().osname()   << std::endl;   // operating system details
        std::cout << std::endl;
      }
    }

    // monitor processes ?
    if(g_do_monitor_procs)
    {
      // collect process infos
      std::cout << "------- PROCESSES -------" << std::endl;

      // for all processes
      for(auto process : monitoring.processes())
      {
        // check filtering
        if(!g_procs_filt_string.empty() && (g_procs_filt_string != process.uname())) continue;

        // print process details
        std::cout << "hname           : " << process.hname()             << std::endl;   // host name
        std::cout << "pid             : " << process.pid()               << std::endl;   // process id
        std::cout << "pname           : " << process.pname()             << std::endl;   // process name
        std::cout << "uname           : " << process.uname()             << std::endl;   // unit name
        std::cout << "pparam          : " << process.pparam()            << std::endl;   // process parameter
        std::cout << "pmemory         : " << process.pmemory()           << std::endl;   // process memory
        std::cout << "pcpu            : " << process.pcpu()              << std::endl;   // process cpu usage
        std::cout << "usrptime        : " << process.usrptime()          << std::endl;   // process user time
        std::cout << "datawrite       : " << process.datawrite()         << std::endl;   // data write bytes per sec
        std::cout << "dataread        : " << process.dataread()          << std::endl;   // date read bytes per sec
        std::cout << "severity        : " << process.state().severity()  << std::endl;   // process state severity
        std::cout << "info            : " << process.state().info()      << std::endl;   // process state info
        std::cout << "tsync state     : " << process.tsync_state()       << std::endl;   // time sync state
        std::cout << "tsync mod name  : " << process.tsync_mod_name()    << std::endl;   // time sync mod name
        std::cout << std::endl;
      }
    }

    // monitor services ?
    if(g_do_monitor_services)
    {
      // collect process infos
      std::cout << "------- SERVICES -------" << std::endl;

      // for all processes
      for(auto service : monitoring.services())
      {
        // check filtering
        if(!g_services_filt_string.empty() && (g_services_filt_string != service.sname())) continue;

        // print process details
        std::cout << "hname           : " << service.hname()      << std::endl;   // host name
        std::cout << "sname           : " << service.sname()      << std::endl;   // service name
        std::cout << "pid             : " << service.pid()        << std::endl;   // process id
        std::cout << std::endl;

        for(int i = 0; i < service.methods_size(); ++i)
        {
          auto method = service.methods(i);
          std::cout << "  mname           : " << method.mname()      << std::endl;   // method name
          std::cout << "  req_type        : " << method.req_type()   << std::endl;   // input type
          std::cout << "  resp_type       : " << method.resp_type()  << std::endl;   // output type
          std::cout << "  call_count      : " << method.call_count() << std::endl;   // call count
          std::cout << std::endl;
        }
        std::cout << std::endl;
      }
    }

    // monitor topics ?
    if(g_do_monitor_topics)
    {
      // collect topic infos
      std::cout << "-------- TOPICS ---------" << std::endl;

      // for all topics
      for(auto topic : monitoring.topics())
      {
        // check filtering
        if(!g_topics_filt_string.empty() && (g_topics_filt_string != topic.tname())) continue;

        // print topic details
        std::cout << "rclock          : " << topic.rclock()          << std::endl;   // registration clock
        std::cout << "hname           : " << topic.hname()           << std::endl;   // host name
        std::cout << "pid             : " << topic.pid()             << std::endl;   // process id
        std::cout << "pname           : " << topic.pname()           << std::endl;   // process name
        std::cout << "uname           : " << topic.uname()           << std::endl;   // unit name
        std::cout << "tid             : " << topic.tid()             << std::endl;   // topic id
        std::cout << "tname           : " << topic.tname()           << std::endl;   // topic name
        std::cout << "direction       : " << topic.direction()       << std::endl;   // direction (publisher, subscriber)
        std::cout << "ttype           : " << topic.ttype()           << std::endl;   // topic type
//        std::cout << "tdesc           : " << topic.tdesc()           << std::endl;   // topic description
//        std::cout << "tqos            : " << topic.tqos()            << std::endl;   // topic quality of service
        for (auto layer : topic.tlayer())
        {
          std::string layer_type("unknown");
          switch (layer.type())
          {
          case eCAL::pb::eTLayerType::tl_none:
            layer_type = "tlayer_none";
            break;
          case eCAL::pb::eTLayerType::tl_ecal_udp_mc:
            layer_type = "tlayer_udp_mc";
            break;
          case eCAL::pb::eTLayerType::tl_ecal_shm:
            layer_type = "tlayer_shm";
            break;
          case eCAL::pb::eTLayerType::tl_inproc:
            layer_type = "tlayer_inproc";
            break;
          case eCAL::pb::eTLayerType::tl_all:
            layer_type = "tlayer_all";
            break;
          default:
            break;
          }
          std::cout << "  tlayer.type     : " << layer_type              << std::endl;   // transport layers type
          std::cout << "  tlayer.confirmed: " << layer.confirmed()       << std::endl;   // transport layers confirmation
          if (!layer.par().empty())
          {
            std::cout << "  tlayer.par      : " << layer.par() << std::endl;   // transport layers parameter
          }
        }
        std::cout << "tsize           : " << topic.tsize()           << std::endl;   // topic size

        std::cout << "connections_loc : " << topic.connections_loc() << std::endl;   // number of local connected entities
        std::cout << "connections_ext : " << topic.connections_ext() << std::endl;   // number of external connected entities
        std::cout << "message_drops   : " << topic.message_drops()   << std::endl;   // dropped messages

        std::cout << "did             : " << topic.did()             << std::endl;   // data send id (publisher setid)
        std::cout << "dclock          : " << topic.dclock()          << std::endl;   // data clock (send / receive action)
        std::cout << "dfreq           : " << topic.dfreq()           << std::endl;   // data frequency (send / receive samples per second * 1000)

        std::cout << std::endl;
      }
    }

    // sleep few milliseconds
    eCAL::Process::SleepMS(g_mon_timing);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
