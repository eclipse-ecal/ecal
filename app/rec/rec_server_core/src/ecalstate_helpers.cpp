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

#include <rec_server_core/ecalstate_helpers.h>

#include <algorithm>

namespace eCAL
{
  namespace rec_server
  {
    std::pair<eCAL_Process_eSeverity, std::string> GetProcessSeverity(const eCAL::rec_server::RecServerStatus& rec_server_status, const std::map<std::string, ClientConfig>& enabled_rec_clients)
    {
      eCAL_Process_eSeverity severity(eCAL_Process_eSeverity::proc_sev_healthy);
      std::string            state_string;

      if(!rec_server_status.activated_)
      {
        severity     = std::max(severity, eCAL_Process_eSeverity::proc_sev_warning);
        state_string = "Not activated";
      }
      else
      {
        if (rec_server_status.recording_meas_id_ == 0)
          state_string = "Activated";
        else
          state_string = "Recording";

        size_t connected_hosts(0);
        size_t connected_addons(0);

        size_t not_connected_hosts(0);
        size_t not_connected_addons(0);

        // Diff config against statuses
        for (const auto& configured_host : enabled_rec_clients)
        {

          auto client_status_it = rec_server_status.client_statuses_.find(configured_host.first);
          if (client_status_it == rec_server_status.client_statuses_.end())
          {
            not_connected_hosts++;
            not_connected_addons += configured_host.second.enabled_addons_.size();
          }
          else
          {
            connected_hosts++;

            for (const auto& configured_addon : configured_host.second.enabled_addons_)
            {
              auto addon_status_it = std::find_if(client_status_it->second.first.addon_statuses_.begin(), client_status_it->second.first.addon_statuses_.end(),
                [&configured_addon] (const eCAL::rec::RecorderAddonStatus& addon_status) { return addon_status.addon_id_ == configured_addon; });

              if (addon_status_it == client_status_it->second.first.addon_statuses_.end())
                not_connected_addons++;
              else
                connected_addons++;
            }
          }
        }

        if ((not_connected_hosts > 0) || (not_connected_addons > 0))
        {
          severity = std::max(severity, eCAL_Process_eSeverity::proc_sev_critical);
          state_string += ", Failed connecting to ";
          if (not_connected_hosts > 0)
          {
            state_string += std::to_string(not_connected_hosts) + " Host" + (not_connected_hosts > 1 ? "s" : "");
            if (not_connected_addons > 0)
              state_string += " & ";
          }
          if (not_connected_addons > 0)
          {
            state_string += std::to_string(not_connected_addons) + " Addon" + (not_connected_addons > 1 ? "s" : "");
          }
        }
        else if (connected_hosts == 0)
        {
          severity = std::max(severity, eCAL_Process_eSeverity::proc_sev_warning);
          state_string += ", No clients";
        }
        else
        {
          state_string += ", Connected to " + std::to_string(connected_hosts) + " Host" + (connected_hosts > 1 ? "s" : "");
          if (connected_addons > 0)
            state_string += " & " + std::to_string(connected_addons) + " Addon" + (connected_addons > 1 ? "s" : "");
        }
      }

      return {severity, state_string};
    }
  }
}