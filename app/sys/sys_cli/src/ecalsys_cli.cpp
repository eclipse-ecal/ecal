/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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
 * eCALSys Console Application
**/

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <ecal/ecal.h>
#include <ecal/ecal_client.h>
#include <ecalsys/ecal_sys.h>
#include <ecalsys/esys_defs.h>

#include <ecal/msg/protobuf/client.h>
#include <ecal/msg/protobuf/server.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/pb/sys/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ecalsys_service.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505) // disable tclap warning (unreferenced local function has been removed)
#endif
#include "tclap/CmdLine.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <custom_tclap/advanced_tclap_output.h>
#include <custom_tclap/fuzzy_value_switch_arg_bool.h>

#include <command_executor.h>
#include <commands/load_config.h>
#include <commands/update_from_cloud.h>
#include <commands/start_tasks.h>
#include <commands/stop_tasks.h>
#include <commands/restart_tasks.h>

#include "ecalsys_util.h"


#include <ecal_utils/command_line.h>
#include <ecal_utils/str_convert.h>
#include <ecal_utils/win_cp_changer.h>

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <Windows.h>
#endif // WIN32


bool exit_command_received;

#ifdef WIN32
int main()
#else
int main(int argc, char** argv)
#endif
{
#ifdef WIN32
  EcalUtils::WinCpChanger win_cp_changer(CP_UTF8); // The WinCpChanger will set the Codepage back to the original, once destroyed
#endif // WIN32

  // Define the command line object.
  TCLAP::CmdLine cmd(ECAL_SYS_LIB_NAME, ' ', ECAL_SYS_VERSION_STRING);
  
  TCLAP::UnlabeledMultiArg<std::string> unlabled_config_arg            ("config_", "The Configuration file to load.", false, "Path");

  TCLAP::ValueArg<std::string>          config_arg                     ("c", "config", "The Configuration file to load. Not supported in remote-control mode.", false, "", "Path");
  
  TCLAP::SwitchArg                      remote_control_arg             ("",  "remote-control",   "Remote control another eCAL Sys instance.",                false);
  TCLAP::ValueArg<std::string>          remote_control_host_arg        ("",  "remote-host",      "Set the hostname for remote-controlling.", false, "", "Hostname");

  TCLAP::SwitchArg                      start_arg                      ("s", "start",   "Start all tasks.",                false);
  TCLAP::SwitchArg                      stop_arg                       ("x", "stop",    "Stop all tasks.",                 false);
  TCLAP::SwitchArg                      restart_arg                    ("r", "restart", "Restart all tasks.",              false);
  
  CustomTclap::FuzzyValueSwitchArgBool      local_tasks_only_arg           ("", "local-tasks-only",            "Only tasks on local host will be considered. Not supported in remote-control mode.",         false, false, "true|false");
  CustomTclap::FuzzyValueSwitchArgBool      use_localhost_for_all_tasks_arg("", "use-localhost-for-all-tasks", "All tasks will be considered as being on local host. Not supported in remote-control mode.", false, false, "true|false");
  CustomTclap::FuzzyValueSwitchArgBool      no_wait_for_clients_arg        ("", "no-wait-for-clients",         "Don't wait for eCAL Sys clients before starting / stopping tasks. Waiting is always disabled in interactive and remote-control mode.", false, false, "true|false"); // TODO: This arguments was a simple switch arg before.
  TCLAP::SwitchArg                      disable_update_from_cloud_arg  ("", "disable-update-from-cloud",   "Do not use the monitor to update the tasks for restarting/stopping/monitoring.", false);

  TCLAP::SwitchArg                      interactive_dont_exit_arg      ("",  "interactive-dont-exit",  "When in interactive mode, this option prevents eCAL Sys from exiting, when stdin is closed.", false);
  TCLAP::SwitchArg                      interactive_arg                ("i", "interactive",            "Start eCAL Sys and listen for commands on stdin. When not in remote-control mode itself, eCAL Sys will offer the eCAL Sys Service for being remote-controlled. Note that eCAL Sys will exit, when stdin is closed. To prevent that, combine this option with \"" + interactive_dont_exit_arg.getName() + "\". When using interactive mode, waiting for ecal_sys_clients is disabled.", false);

  std::vector<TCLAP::Arg*> arg_vector =
  {
    &config_arg,
    &remote_control_arg,
    &remote_control_host_arg,
    &start_arg,
    &stop_arg,
    &restart_arg,
    &local_tasks_only_arg,
    &use_localhost_for_all_tasks_arg,
    &no_wait_for_clients_arg,
    &disable_update_from_cloud_arg,
    &interactive_arg,
    &interactive_dont_exit_arg,
    &unlabled_config_arg,
  };

  std::stringstream tclap_output_stream;
  CustomTclap::AdvancedTclapOutput advanced_tclap_output(std::vector<std::ostream*>{ &std::cout }, 75);
  advanced_tclap_output.setArgumentHidden(&unlabled_config_arg, true);
  
  cmd.setOutput(&advanced_tclap_output);

  for (auto arg_iterator = arg_vector.rbegin(); arg_iterator != arg_vector.rend(); ++arg_iterator)
  {
    cmd.add(*arg_iterator);
  }

  try
  {
#ifdef WIN32
    auto utf8_args_vector = EcalUtils::CommandLine::GetUtf8Argv();
    cmd.parse(utf8_args_vector);
#else
    cmd.parse(argc, argv);
#endif // WIN32
  }
  catch (TCLAP::ArgException& e)
  {
    std::cerr << "Error parsing command line: " << e.what() << std::endl;
  }

  std::string cfg_file_name;
  if (config_arg.isSet())
  {
    cfg_file_name             = config_arg                     .getValue();
  }
  else if (unlabled_config_arg.isSet())
  {
    cfg_file_name             = unlabled_config_arg            .getValue()[0];
  }
  
  // Ecalsys instance and ecalsys service. We will only use one of those, depending on the remote-control setting
  std::shared_ptr<EcalSys>                                                ecalsys_instance;
  std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::sys::Service>> remote_ecalsys_service;
  
  // Ptrs for eCAL Sys Service (only used in non-remote control mode)
  std::shared_ptr<eCALSysServiceImpl>                                     ecalsys_service_impl;
  std::shared_ptr<eCAL::protobuf::CServiceServer<eCAL::pb::sys::Service>> ecalsys_service_server;

  /************************************************************************/
  /* Argument validation                                                  */
  /************************************************************************/

  // remote-host without remote-control
  if (remote_control_host_arg.isSet() && !remote_control_arg.isSet())
  {
    std::cerr << "Error: " << remote_control_host_arg.getName() << " can only be used in combination with " << remote_control_arg.getName() << "." << std::endl;
    return EXIT_FAILURE;
  }

  // multiple commands
  if (int(start_arg.isSet()) + int(stop_arg.isSet()) + int(restart_arg.isSet()) > 1)
  {
    std::cerr << "Error: Only one argument of " << start_arg.getName() << " / " << restart_arg.getName() << " / " << stop_arg.getName() << " can be used simultaneously." << std::endl;
    return EXIT_FAILURE;
  }

  // No config and not in remote-control or interactive mode 
  if (!config_arg.isSet()
    && unlabled_config_arg.getValue().empty()
    && !remote_control_arg.isSet()
    && !interactive_arg.isSet()
    && !interactive_dont_exit_arg.isSet())
  {
    std::cerr << "Error: Configuration file needed, when not running as " << remote_control_arg.getName() << " or " << interactive_arg.getName() << std::endl;
    return EXIT_FAILURE;
  }

  // local-tasks-only or use-localhost-for-all-tasks in remote-control mode
  if (local_tasks_only_arg.isSet() && remote_control_arg.isSet())
  {
    std::cerr << "Error: " << local_tasks_only_arg.getName() << " cannot be used in remote-control mode." << std::endl;
    return EXIT_FAILURE;
  }
  if (use_localhost_for_all_tasks_arg.isSet() && remote_control_arg.isSet())
  {
    std::cerr << "Error: " << use_localhost_for_all_tasks_arg.getName() << " cannot be used in remote-control mode." << std::endl;
    return EXIT_FAILURE;
  }

  // Both local-tasks-only and use-localhost-for-all-tasks is given
  if ((local_tasks_only_arg.isSet() && local_tasks_only_arg.getValue())
    && (use_localhost_for_all_tasks_arg.isSet() && use_localhost_for_all_tasks_arg.getValue()))
  {
    std::cerr << "Error: " << local_tasks_only_arg.getName() << " and " << use_localhost_for_all_tasks_arg.getName() << " are mutually excludsive (only one of them can be used)." << std::endl;
    return EXIT_FAILURE;
  }

  // There is nothing to do
  if (!interactive_arg.isSet()
    && !interactive_dont_exit_arg.isSet()
    && !start_arg.isSet()
    && !stop_arg.isSet()
    && !restart_arg.isSet())
  {
    std::cerr << "Error: There is nothing to do." << std::endl;
    return EXIT_FAILURE;
  }

  /************************************************************************/
  /*  Remote control mode                                                 */
  /************************************************************************/
  if (remote_control_arg.isSet()) // Remote-control-mode
  {
    eCAL::Initialize(0, nullptr, "eCALSys-Remote", eCAL::Init::All);
    eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "Running");

    remote_ecalsys_service = std::make_shared<eCAL::protobuf::CServiceClient<eCAL::pb::sys::Service>>();
  }
  else                            // Non-remote control mode
  {
    eCAL::Initialize(0, nullptr, "eCALSys", eCAL::Init::All);
    eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "Running");

    ecalsys_instance = std::make_shared<EcalSys>();

    // Create the eCALSys service
    ecalsys_service_impl   = std::make_shared<eCALSysServiceImpl>(ecalsys_instance);
    ecalsys_service_server = std::make_shared<eCAL::protobuf::CServiceServer<eCAL::pb::sys::Service>>(ecalsys_service_impl);
  }

  // Give the monitor some time to connect to eCAL and update
  std::this_thread::sleep_for(std::chrono::seconds(2));

  /************************************************************************/
  /* Load the configuration                                               */
  /************************************************************************/
  if (config_arg.isSet() || !unlabled_config_arg.getValue().empty())
  {
    eCAL::sys::Error error(eCAL::sys::Error::ErrorCode::GENERIC_ERROR);
    if (ecalsys_instance)
    {
      error = eCAL::sys::command::LoadConfig().Execute(ecalsys_instance, {cfg_file_name});
      if (error)
      {
        std::cerr << error.ToString() << std::endl;
        return EXIT_FAILURE;
      }
    }
    else
    {
      std::cerr << "Warning: Ignoring configuration file in remote control mode" << std::endl;
    }
  }

  /************************************************************************/
  /* eCAL Sys Options                                                     */
  /************************************************************************/
  if (ecalsys_instance)
  {
    auto options         = ecalsys_instance->GetOptions();
    bool options_changed = false;

    // --local-tasks-only
    if (local_tasks_only_arg.isSet())
    {
      options.local_tasks_only = local_tasks_only_arg.getValue();
      if (options.local_tasks_only)
        options.use_localhost_for_all_tasks = false;

      options_changed = true;
    }

    // --use-localhost-for-all-tasks
    if (use_localhost_for_all_tasks_arg.isSet())
    {
      options.use_localhost_for_all_tasks = use_localhost_for_all_tasks_arg.getValue();
      if (options.use_localhost_for_all_tasks)
        options.local_tasks_only = false;

      options_changed = true;
    }

    // --no-wait-for-clients
    if (no_wait_for_clients_arg.isSet())
    {
      options.check_target_reachability = !no_wait_for_clients_arg.getValue();
      options_changed = true;
    }

    if (options_changed)
      ecalsys_instance->SetOptions(options);
  }

  /************************************************************************/
  /* Wait for eCAL Sys Clients                                            */
  /************************************************************************/
  if (!interactive_arg.isSet() && !interactive_dont_exit_arg.isSet() && !no_wait_for_clients_arg.isSet())
  {
    if (ecalsys_instance)
    {
      WaitForClients(ecalsys_instance);
    }
  }

  /************************************************************************/
  /* Update from cloud                                                    */
  /************************************************************************/
  if (!disable_update_from_cloud_arg.isSet())
  {
    eCAL::sys::Error error(eCAL::sys::Error::ErrorCode::GENERIC_ERROR);
    if (ecalsys_instance && ecalsys_instance->IsConfigOpened())
    {
      error = eCAL::sys::command::UpdateFromCloud().Execute(ecalsys_instance, {});
      if (error)
      {
        std::cerr << "Error: " << error.ToString() << std::endl;
        return EXIT_FAILURE;
      }
      else
      {
        ecalsys_instance->WaitForTaskActions();
      }
    }
  }

  /************************************************************************/
  /* Start Tasks                                                          */
  /************************************************************************/
  if (start_arg.isSet())
  {
    eCAL::sys::Error error(eCAL::sys::Error::ErrorCode::GENERIC_ERROR);
    if (ecalsys_instance)
    {
      error = eCAL::sys::command::StartTask().Execute(ecalsys_instance, {});
      if (!error)
        ecalsys_instance->WaitForTaskActions();
    }
    else
    {
      error = eCAL::sys::command::StartTask().Execute(remote_control_host_arg.getValue(), remote_ecalsys_service, {});
    }

    if (error)
      std::cerr << "Error: " << error.ToString() << std::endl;
  }

  /************************************************************************/
  /* Restart Tasks                                                        */
  /************************************************************************/
  if (restart_arg.isSet())
  {
    eCAL::sys::Error error(eCAL::sys::Error::ErrorCode::GENERIC_ERROR);
    if (ecalsys_instance)
    {
      error = eCAL::sys::command::RestartTask().Execute(ecalsys_instance, {});
      if (!error)
        ecalsys_instance->WaitForTaskActions();
    }
    else
    {
      error = eCAL::sys::command::RestartTask().Execute(remote_control_host_arg.getValue(), remote_ecalsys_service, {});
    }

    if (error)
      std::cerr << "Error: " << error.ToString() << std::endl;
  }

  /************************************************************************/
  /* Stop Tasks                                                           */
  /************************************************************************/
  if (stop_arg.isSet())
  {
    eCAL::sys::Error error(eCAL::sys::Error::ErrorCode::GENERIC_ERROR);
    if (ecalsys_instance)
    {
      error = eCAL::sys::command::StopTask().Execute(ecalsys_instance, {});
      if (!error)
        ecalsys_instance->WaitForTaskActions();
    }
    else
    {
      error = eCAL::sys::command::StopTask().Execute(remote_control_host_arg.getValue(), remote_ecalsys_service, {});
    }

    if (error)
      std::cerr << "Error: " << error.ToString() << std::endl;
  }


  /************************************************************************/
  /* Interactive Mode                                                     */
  /************************************************************************/
  if(interactive_arg.isSet() || interactive_dont_exit_arg.isSet())
  {
#ifdef WIN32
    // Create buffer fo the manual ReadConsoleW call
    std::wstring w_buffer;
    w_buffer.reserve(4096);
#endif // WIN32

    std::cout << "Using interactive mode. Type \"help\" to view a list of all commands." << std::endl;
    eCAL::sys::CommandExecutor command_executor(ecalsys_instance, remote_control_host_arg.getValue(), remote_ecalsys_service);
    for(;;)
    {
      std::cout << ">> ";
      std::string line;
      bool success = false;

#ifdef WIN32
      HANDLE h_in = GetStdHandle(STD_INPUT_HANDLE);
      DWORD std_handle_type = GetFileType(h_in);
      if (std_handle_type == FILE_TYPE_CHAR)
      {
        // This is an (interactive) console => read console as UTF16
        DWORD chars_read(0);
        w_buffer.resize(4096);

        success = ReadConsoleW(h_in, (LPVOID)(w_buffer.data()), static_cast<DWORD>(w_buffer.size()), &chars_read, NULL) != 0;

        if (success)
        {
          w_buffer.resize(chars_read);
          line = EcalUtils::StrConvert::WideToUtf8(w_buffer);

          // Trim \r\n at the end
          for (int i = 0; i < 2; i++)
          {
            if (line.size() > 0 
              && ((line.back() == '\r') || (line.back() == '\n')))
            {
              line.pop_back();
            }
          }
        }
      }
      else
      {
        // This is a pipe => read binary data directly as UTF8
        success = bool(std::getline(std::cin, line));
      }
#else
      success = bool(std::getline(std::cin, line));
#endif // WIN32

      if (!success)
      {
        std::cout << "Stdin closed." << std::endl;
        break;
      }

      eCAL::sys::Error error = command_executor.ExecuteCommand(line);
      if (error)
      {
        std::cerr << "Error: " << error.ToString() << std::endl;
      }
    }
  }

  if(interactive_dont_exit_arg.isSet())
  {
    while(eCAL::Ok())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  eCAL::Finalize();
  return EXIT_SUCCESS;
}
