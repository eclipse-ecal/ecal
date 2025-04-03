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

/**
 * @brief Functions to generate Process Registration / Unregistration samples.
 *
**/

#include "ecal_process_registration.h"

#include <ecal/init.h>
#include <ecal/process.h>
#include "ecal_global_accessors.h"
#include "ecal_globals.h"
#include "time/ecal_timegate.h"

eCAL::Registration::Sample eCAL::Registration::GetProcessRegisterSample()
{
  Registration::Sample process_sample;
  process_sample.cmd_type = bct_reg_process;

  auto& process_sample_identifier = process_sample.identifier;
  process_sample_identifier.host_name  = eCAL::Process::GetHostName();
  process_sample_identifier.process_id = eCAL::Process::GetProcessID();
  // We need to set the process_id as entity_id.
  // However, we cannot send anything over the wire :(
  process_sample_identifier.entity_id = process_sample_identifier.process_id;

  auto& process_sample_process                = process_sample.process;
  process_sample_process.shm_transport_domain = eCAL::Process::GetShmTransportDomain();
  process_sample_process.process_name         = eCAL::Process::GetProcessName();
  process_sample_process.unit_name            = eCAL::Process::GetUnitName();
  process_sample_process.process_parameter               = eCAL::Process::GetProcessParameter();
  process_sample_process.state.severity       = static_cast<Registration::eProcessSeverity>(g_process_severity);
  process_sample_process.state.severity_level = static_cast<Registration::eProcessSeverityLevel>(g_process_severity_level);
  process_sample_process.state.info           = g_process_info;
#if ECAL_CORE_TIMEPLUGIN
  if (g_timegate() == nullptr)
  {
    process_sample_process.time_sync_state = Registration::eTimeSyncState::tsync_none;
  }
  else
  {
    if (!g_timegate()->IsSynchronized())
    {
      process_sample_process.time_sync_state = Registration::eTimeSyncState::tsync_none;
    }
    else
    {
      switch (g_timegate()->GetSyncMode())
      {
      case CTimeGate::eTimeSyncMode::realtime:
        process_sample_process.time_sync_state = Registration::eTimeSyncState::tsync_realtime;
        break;
      case CTimeGate::eTimeSyncMode::replay:
        process_sample_process.time_sync_state = Registration::eTimeSyncState::tsync_replay;
        break;
      default:
        process_sample_process.time_sync_state = Registration::eTimeSyncState::tsync_none;
        break;
      }
    }
    process_sample_process.time_sync_module_name = g_timegate()->GetName();
  }
#endif

  // eCAL initialization state
  const unsigned int comp_state(g_globals()->GetComponents());
  process_sample_process.component_init_state = static_cast<int32_t>(comp_state);
  std::string component_info;
  if ((comp_state & eCAL::Init::Publisher) != 0u) component_info += "|pub";
  if ((comp_state & eCAL::Init::Subscriber) != 0u) component_info += "|sub";
  if ((comp_state & eCAL::Init::Logging) != 0u) component_info += "|log";
  if ((comp_state & eCAL::Init::TimeSync) != 0u) component_info += "|time";
  if (!component_info.empty()) component_info = component_info.substr(1);
  process_sample_process.component_init_info = component_info;

  process_sample_process.ecal_runtime_version = eCAL::GetVersionString();
  process_sample_process.config_file_path = eCAL::GetConfiguration().GetConfigurationFilePath();

  return process_sample;
}

eCAL::Registration::Sample eCAL::Registration::GetProcessUnregisterSample()
{
  Registration::Sample process_sample;
  process_sample.cmd_type = bct_unreg_process;

  auto& process_sample_identifier = process_sample.identifier;
  process_sample_identifier.host_name  = eCAL::Process::GetHostName();
  process_sample_identifier.process_id = eCAL::Process::GetProcessID();

  auto& process_sample_process = process_sample.process;
  process_sample_process.process_name = eCAL::Process::GetProcessName();
  process_sample_process.unit_name    = eCAL::Process::GetUnitName();

  return process_sample;
}
