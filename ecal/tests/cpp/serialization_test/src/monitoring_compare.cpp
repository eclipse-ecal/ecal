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
        if (monitoring1.processes[i].rclock != monitoring2.processes[i].rclock ||
          monitoring1.processes[i].hname != monitoring2.processes[i].hname ||
          monitoring1.processes[i].hgname != monitoring2.processes[i].hgname ||
          monitoring1.processes[i].pid != monitoring2.processes[i].pid ||
          monitoring1.processes[i].pname != monitoring2.processes[i].pname ||
          monitoring1.processes[i].uname != monitoring2.processes[i].uname ||
          monitoring1.processes[i].pparam != monitoring2.processes[i].pparam ||
          monitoring1.processes[i].state_severity != monitoring2.processes[i].state_severity ||
          monitoring1.processes[i].state_severity_level != monitoring2.processes[i].state_severity_level ||
          monitoring1.processes[i].state_info != monitoring2.processes[i].state_info ||
          monitoring1.processes[i].tsync_state != monitoring2.processes[i].tsync_state ||
          monitoring1.processes[i].tsync_mod_name != monitoring2.processes[i].tsync_mod_name ||
          monitoring1.processes[i].component_init_state != monitoring2.processes[i].component_init_state ||
          monitoring1.processes[i].component_init_info != monitoring2.processes[i].component_init_info ||
          monitoring1.processes[i].ecal_runtime_version != monitoring2.processes[i].ecal_runtime_version)
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
        if (monitoring1.publisher[i].rclock != monitoring2.publisher[i].rclock ||
          monitoring1.publisher[i].hname != monitoring2.publisher[i].hname ||
          monitoring1.publisher[i].hgname != monitoring2.publisher[i].hgname ||
          monitoring1.publisher[i].pid != monitoring2.publisher[i].pid ||
          monitoring1.publisher[i].pname != monitoring2.publisher[i].pname ||
          monitoring1.publisher[i].uname != monitoring2.publisher[i].uname ||
          monitoring1.publisher[i].tid != monitoring2.publisher[i].tid ||
          monitoring1.publisher[i].tname != monitoring2.publisher[i].tname ||
          monitoring1.publisher[i].direction != monitoring2.publisher[i].direction ||
          monitoring1.publisher[i].tdatatype.name != monitoring2.publisher[i].tdatatype.name ||
          monitoring1.publisher[i].tdatatype.encoding != monitoring2.publisher[i].tdatatype.encoding ||
          monitoring1.publisher[i].tdatatype.descriptor != monitoring2.publisher[i].tdatatype.descriptor ||
          monitoring1.publisher[i].tlayer.size() != monitoring2.publisher[i].tlayer.size() ||
          monitoring1.publisher[i].tsize != monitoring2.publisher[i].tsize ||
          monitoring1.publisher[i].connections_loc != monitoring2.publisher[i].connections_loc ||
          monitoring1.publisher[i].connections_ext != monitoring2.publisher[i].connections_ext ||
          monitoring1.publisher[i].message_drops != monitoring2.publisher[i].message_drops ||
          monitoring1.publisher[i].did != monitoring2.publisher[i].did ||
          monitoring1.publisher[i].dclock != monitoring2.publisher[i].dclock ||
          monitoring1.publisher[i].dfreq != monitoring2.publisher[i].dfreq ||
          monitoring1.publisher[i].attr != monitoring2.publisher[i].attr)
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
        if (monitoring1.subscriber[i].rclock != monitoring2.subscriber[i].rclock ||
          monitoring1.subscriber[i].hname != monitoring2.subscriber[i].hname ||
          monitoring1.subscriber[i].hgname != monitoring2.subscriber[i].hgname ||
          monitoring1.subscriber[i].pid != monitoring2.subscriber[i].pid ||
          monitoring1.subscriber[i].pname != monitoring2.subscriber[i].pname ||
          monitoring1.subscriber[i].uname != monitoring2.subscriber[i].uname ||
          monitoring1.subscriber[i].tid != monitoring2.subscriber[i].tid ||
          monitoring1.subscriber[i].tname != monitoring2.subscriber[i].tname ||
          monitoring1.subscriber[i].direction != monitoring2.subscriber[i].direction ||
          monitoring1.subscriber[i].tdatatype.name != monitoring2.subscriber[i].tdatatype.name ||
          monitoring1.subscriber[i].tdatatype.encoding != monitoring2.subscriber[i].tdatatype.encoding ||
          monitoring1.subscriber[i].tdatatype.descriptor != monitoring2.subscriber[i].tdatatype.descriptor ||
          monitoring1.subscriber[i].tlayer.size() != monitoring2.subscriber[i].tlayer.size() ||
          monitoring1.subscriber[i].tsize != monitoring2.subscriber[i].tsize ||
          monitoring1.subscriber[i].connections_loc != monitoring2.subscriber[i].connections_loc ||
          monitoring1.subscriber[i].connections_ext != monitoring2.subscriber[i].connections_ext ||
          monitoring1.subscriber[i].message_drops != monitoring2.subscriber[i].message_drops ||
          monitoring1.subscriber[i].did != monitoring2.subscriber[i].did ||
          monitoring1.subscriber[i].dclock != monitoring2.subscriber[i].dclock ||
          monitoring1.subscriber[i].dfreq != monitoring2.subscriber[i].dfreq ||
          monitoring1.subscriber[i].attr != monitoring2.subscriber[i].attr)
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
        if (monitoring1.server[i].rclock != monitoring2.server[i].rclock ||
          monitoring1.server[i].hname != monitoring2.server[i].hname ||
          monitoring1.server[i].pname != monitoring2.server[i].pname ||
          monitoring1.server[i].uname != monitoring2.server[i].uname ||
          monitoring1.server[i].pid != monitoring2.server[i].pid ||
          monitoring1.server[i].sname != monitoring2.server[i].sname ||
          monitoring1.server[i].sid != monitoring2.server[i].sid ||
          monitoring1.server[i].version != monitoring2.server[i].version ||
          monitoring1.server[i].tcp_port_v0 != monitoring2.server[i].tcp_port_v0 ||
          monitoring1.server[i].tcp_port_v1 != monitoring2.server[i].tcp_port_v1 ||
          monitoring1.server[i].methods.size() != monitoring2.server[i].methods.size())
        {
          return false;
        }

        for (size_t j = 0; j < monitoring1.server[i].methods.size(); ++j)
        {
          if (monitoring1.server[i].methods[j].mname != monitoring2.server[i].methods[j].mname ||
            monitoring1.server[i].methods[j].req_type != monitoring2.server[i].methods[j].req_type ||
            monitoring1.server[i].methods[j].req_desc != monitoring2.server[i].methods[j].req_desc ||
            monitoring1.server[i].methods[j].resp_type != monitoring2.server[i].methods[j].resp_type ||
            monitoring1.server[i].methods[j].resp_desc != monitoring2.server[i].methods[j].resp_desc ||
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
        if (monitoring1.clients[i].rclock != monitoring2.clients[i].rclock ||
          monitoring1.clients[i].hname != monitoring2.clients[i].hname ||
          monitoring1.clients[i].pname != monitoring2.clients[i].pname ||
          monitoring1.clients[i].uname != monitoring2.clients[i].uname ||
          monitoring1.clients[i].pid != monitoring2.clients[i].pid ||
          monitoring1.clients[i].sname != monitoring2.clients[i].sname ||
          monitoring1.clients[i].sid != monitoring2.clients[i].sid ||
          monitoring1.clients[i].version != monitoring2.clients[i].version)
        {
          return false;
        }
      }

      return true;
    }
  }
}
