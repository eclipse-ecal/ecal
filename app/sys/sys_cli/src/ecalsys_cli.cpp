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

/**
 * eCALSys Console Application
**/

#include <iostream>
#include <memory>
#include <string>

#include <ecal/ecal.h>
#include <ecalsys/ecal_sys.h>
#include <ecalsys/esys_defs.h>


#include "tclap/CmdLine.h"
#include <custom_tclap/advanced_tclap_output.h>

#include <command_executor.h>

bool exit_command_received;

int main(int argc, char** argv)
{
  // Define the command line object.
  TCLAP::CmdLine cmd(ECAL_SYS_LIB_NAME, ' ', ECAL_SYS_VERSION_STRING);
  
  TCLAP::UnlabeledMultiArg<std::string> unlabled_config_arg            ("config_", "The Configuration file to load", false, "Path");

  TCLAP::ValueArg<std::string>          config_arg                     ("c", "config", "The Configuration file to load", false, "", "Path");
  
  TCLAP::SwitchArg                      remote_control_arg             ("",  "remote-control",   "Remote control another eCAL Sys instance",                false);
  TCLAP::ValueArg<std::string>          remote_control_host_arg        ("",  "remote-host",      "Set the hostname for remote-controlling", false, "", "Hostname");

  TCLAP::SwitchArg                      start_arg                      ("s", "start",   "Start all tasks",                false);
  TCLAP::SwitchArg                      stop_arg                       ("x", "stop",    "Stop all tasks",                 false);
  TCLAP::SwitchArg                      restart_arg                    ("r", "restart", "Restart all tasks",              false);
  
  TCLAP::SwitchArg                      local_tasks_only_arg           ("", "local-tasks-only",            "Only tasks on local host will be considered",                                   false);
  TCLAP::SwitchArg                      use_localhost_for_all_tasks_arg("", "use-localhost-for-all-tasks", "All tasks will be considered as being on local host",                           false);
  TCLAP::SwitchArg                      disable_target_checks_arg      ("", "disable-target-checks",       "Disable checking for reachable targets before starting tasks",                  false);
  TCLAP::SwitchArg                      disable_update_from_cloud_arg  ("", "disable-update-from-cloud",   "Do not use the monitor to update the tasks for restarting/stopping/monitoring", false);

  TCLAP::SwitchArg                      interactive_arg                ("i", "interactive",            "Start eCAL Sys and dont exit. The user can interactively use Sys or control it with the eCAL Service API.", false);






  // LEGACY ARGS TODO: Add to the arg vector or remove
  //TCLAP::SwitchArg                      monitor_arg                    ("m", "monitor", "Monitor started tasks while publishing the __ecalsys_state__ topic and executing service requests.", false); // TODO: Hide this, as it is basically the same as interactive arg
  //TCLAP::SwitchArg                      local_tasks_only_arg           ("", "local_tasks_only",            "Only tasks on local host will be considered",                                   false);
  //TCLAP::SwitchArg                      use_localhost_for_all_tasks_arg("", "use_localhost_for_all_tasks", "All tasks will be considered as being on local host",                           false);
  //TCLAP::SwitchArg                      disable_target_checks_arg      ("", "disable_target_checks",       "Disable checking for reachable targets before starting tasks",                  false);
  //TCLAP::SwitchArg                      disable_update_from_cloud_arg  ("", "disable_update_from_cloud",   "Do not use the monitor to update the tasks for restarting/stopping/monitoring", false);




  std::vector<TCLAP::Arg*> arg_vector =
  {
    &config_arg,
    &start_arg,
    &stop_arg,
    &restart_arg,
    &local_tasks_only_arg,
    &use_localhost_for_all_tasks_arg,
    &disable_target_checks_arg,
    &disable_update_from_cloud_arg,
    &interactive_arg,
    &unlabled_config_arg,
  };

  for (auto arg_iterator = arg_vector.rbegin(); arg_iterator != arg_vector.rend(); ++arg_iterator)
  {
    cmd.add(*arg_iterator);
  }

  try
  {
    cmd.parse(argc, argv);
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

  /*****************************************************************************************************/
  /*  check for the 2 options "local-tasks-only" and "use-localhost-for-all-tasks" used simultaneously */
  /*****************************************************************************************************/
  if (local_tasks_only_arg.isSet() && use_localhost_for_all_tasks_arg.isSet())
  {
    std::cout << "eCALSys cannot be used with both options \"" << local_tasks_only_arg.getName() << "\" and \"" << use_localhost_for_all_tasks_arg.getName() << "\"" << std::endl;
    std::cout << "Please use maximum one option." << std::endl;
    return EXIT_FAILURE;
  }
  
  /************************************************************************/
  /*  initialize eCAL API                                                 */
  /************************************************************************/
  eCAL::Initialize(0, nullptr, "eCALSys", eCAL::Init::All);
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "Running");
  
  /************************************************************************/
  /*  load the configuration                                              */
  /************************************************************************/
  std::shared_ptr<EcalSys> ecalsys_inst;
  try
  {
    ecalsys_inst = std::make_shared<EcalSys>(cfg_file_name);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  
  if (!ecalsys_inst->IsConfigOpened())
  {
    std::cout << "Configuration file could not be opened => application will close" << std::endl;
    return EXIT_FAILURE;
  }
  
  // Create the eCALSys service
  //std::shared_ptr<eCALSysServiceImpl> ecalsys_service_impl = std::make_shared<eCALSysServiceImpl>(ecalsys_inst);
  //eCAL::protobuf::CServiceServer<eCAL::pb::sys::Service> ecalsys_service(ecalsys_service_impl);
  
  // Give the monitor some time to connect to eCAL and update
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // TODO:do i need this?
  //// Add the verbose function, if required
  //if (verbose_flag)
  //{
  //  ecalsys_inst->SetMonitorUpdateCallback([&ecalsys_inst]() {showMonitoringSummary(ecalsys_inst); });
  //}

  if(interactive_arg.isSet())
  {
    std::cout << "Using interactive mode. Type \"help\" to view a list of all commands." << std::endl;
    eCAL::sys::CommandExecutor command_executor(ecalsys_inst);
    for(;;)
    {
      std::cout << ">> ";
      std::string line;
      std::getline(std::cin, line);

      eCAL::sys::Error error = command_executor.ExecuteCommand(line);
      if (error)
      {
        std::cerr << "Error: " << error.ToString() << std::endl;
      }
    }
  }
}















//#include <iostream>
//#include <memory>
//#include <string>
//#include <thread>
//
//#include "tclap/CmdLine.h"
//#include <custom_tclap/advanced_tclap_output.h>
//
//#include "ecal/ecal.h"
//#include "ecal/msg/protobuf/server.h"
//#include "ecalsys/ecal_sys.h"
//#include "ecalsys/esys_defs.h"
//#include "ecalsys/ecal_sys_logger.h"
//#include "ecalsys_service.h"
//#include "ecalsys_util.h"
//
//#ifdef ECAL_OS_WINDOWS
//#include <conio.h>
//#else
//int _kbhit();
//int _getch(void);
//#endif  // ECAL_OS_WINDOWS
//
//bool exit_command_received = false;
//
//int main(int argc, char** argv)
//{
//  bool start_flag,
//    stop_flag,
//    restart_flag,
//    monitor_flag,
//    verbose_flag,
//    local_tasks_only,
//    use_localhost_for_all_tasks,
//    disable_target_checks,
//    disable_update_from_cloud
//    = false;
//
//  std::string cfg_file_name;
//
//
//  /************************************************************************/
//  /* Create & Parse the command line                                      */
//  /************************************************************************/
//
//  // Define the command line object.
//  TCLAP::CmdLine cmd(ECAL_SYS_LIB_NAME, ' ', ECAL_SYS_VERSION_STRING);
//
//  TCLAP::UnlabeledMultiArg<std::string> unlabled_config_arg ("config_", "The Configuration file to load", false, "Path");
//
//  TCLAP::ValueArg<std::string> config_arg("c", "config", "The Configuration file to load", false, "", "string");
//
//  TCLAP::SwitchArg start_arg  ("s", "start",   "Start all tasks",                false);
//  TCLAP::SwitchArg stop_arg   ("x", "stop",    "Stop all tasks",                 false);
//  TCLAP::SwitchArg restart_arg("r", "restart", "Restart all tasks",              false);
//  TCLAP::SwitchArg monitor_arg("m", "monitor", "Monitor started tasks while publishing the __ecalsys_state__ topic and executing service requests.", false);
//  TCLAP::SwitchArg verbose_arg("v", "verbose", "Verbose monitoring information", false);
//
//  TCLAP::SwitchArg local_tasks_only_arg           ("", "local_tasks_only",            "Only tasks on local host will be considered",                                   false);
//  TCLAP::SwitchArg use_localhost_for_all_tasks_arg("", "use_localhost_for_all_tasks", "All tasks will be considered as being on local host",                           false);
//  TCLAP::SwitchArg disable_target_checks_arg      ("", "disable_target_checks",       "Disable checking for reachable targets before starting tasks",                  false);
//  TCLAP::SwitchArg disable_update_from_cloud_arg  ("", "disable_update_from_cloud",   "Do not use the monitor to update the tasks for restarting/stopping/monitoring", false);
//
//  cmd.add(disable_target_checks_arg);
//  cmd.add(use_localhost_for_all_tasks_arg);
//  cmd.add(local_tasks_only_arg);
//  cmd.add(disable_update_from_cloud_arg);
//  cmd.add(verbose_arg);
//  cmd.add(monitor_arg);
//  cmd.add(restart_arg);
//  cmd.add(stop_arg);
//  cmd.add(start_arg);
//  cmd.add(config_arg);
//
//  std::stringstream tclap_output_stream;
//  CustomTclap::AdvancedTclapOutput advanced_tclap_output(std::vector<std::ostream*>{ &tclap_output_stream, &std::cout }, 75);
//  advanced_tclap_output.setArgumentHidden(&unlabled_config_arg, true);
//
//  cmd.setOutput(&advanced_tclap_output);
//
//  // Parse the argv array.
//  try
//  {
//    cmd.parse(argc, argv);
//  }
//  catch (TCLAP::ArgException &e)  // catch any exceptions
//  {
//    std::cerr << "Error parsing command line: " << e.what() << std::endl;
//  }
//
//  if (config_arg.isSet())
//  {
//    cfg_file_name             = config_arg                     .getValue();
//  }
//  else if (unlabled_config_arg.isSet())
//  {
//    cfg_file_name             = unlabled_config_arg            .getValue()[0];
//  }
//
//  start_flag                  = start_arg                      .getValue();
//  stop_flag                   = stop_arg                       .getValue();
//  restart_flag                = restart_arg                    .getValue();
//  monitor_flag                = monitor_arg                    .getValue();
//  verbose_flag                = verbose_arg                    .getValue();
//
//  local_tasks_only            = local_tasks_only_arg           .getValue();
//  use_localhost_for_all_tasks = use_localhost_for_all_tasks_arg.getValue();
//  disable_target_checks       = disable_target_checks_arg      .getValue();
//  disable_update_from_cloud   = disable_update_from_cloud_arg  .getValue();
//
//  if (cfg_file_name.empty())
//  {
//    std::cout << "Config file is required." << std::endl;
//    return EXIT_FAILURE;
//  }
//
//  if ((start_flag   && (stop_flag  || restart_flag)) ||
//      (stop_flag    && (start_flag || restart_flag)) ||
//      (restart_flag && (start_flag || stop_flag)))
//  {
//    std::cout << "eCALSys must be called with only one of options start / stop / restart" << std::endl;
//    return EXIT_FAILURE;
//  }
//
//  /*****************************************************************************************************/
//  /*  check for the 2 options "local_tasks_only" and "use_localhost_for_all_tasks" used simultaneously */
//  /*****************************************************************************************************/
//  if (local_tasks_only && use_localhost_for_all_tasks)
//  {
//    std::cout << "eCALSys cannot be used with both options \"local_tasks_only\" and \"use_localhost_for_all_tasks\n Please use maximum one option\nApplication will close" << std::endl;
//    return EXIT_FAILURE;
//  }
//
//  /************************************************************************/
//  /*  initialize eCAL API                                                 */
//  /************************************************************************/
//  eCAL::Initialize(0, nullptr, "eCALSys", eCAL::Init::All);
//  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "Running");
//
//  /************************************************************************/
//  /*  load the configuration                                              */
//  /************************************************************************/
//  std::shared_ptr<EcalSys> ecalsys_inst;
//  try
//  {
//    ecalsys_inst = std::make_shared<EcalSys>(cfg_file_name);
//  }
//  catch (const std::exception& e)
//  {
//    std::cerr << e.what() << std::endl;
//    return EXIT_FAILURE;
//  }
//
//  if (!ecalsys_inst->IsConfigOpened())
//  {
//    std::cout << "Configuration file could not be opened => application will close" << std::endl;
//    return EXIT_FAILURE;
//  }
//
//  // Create the eCALSys service
//  std::shared_ptr<eCALSysServiceImpl> ecalsys_service_impl = std::make_shared<eCALSysServiceImpl>(ecalsys_inst);
//  eCAL::protobuf::CServiceServer<eCAL::pb::sys::Service> ecalsys_service(ecalsys_service_impl);
//
//  // Give the monitor some time to connect to eCAL and update
//  std::this_thread::sleep_for(std::chrono::seconds(2));
//
//  // Add the verbose function, if required
//  if (verbose_flag)
//  {
//    ecalsys_inst->SetMonitorUpdateCallback([&ecalsys_inst]() {showMonitoringSummary(ecalsys_inst); });
//  }
//
//  /************************************************************************/
//  /*  check for other eCALSys instances                                   */
//  /************************************************************************/
//  auto sys_instances = ecalsys_inst->GetHostsRunningEcalSys();
//
//  std::string msg;
//  auto my_host_and_pid = std::make_pair(eCAL::Process::GetHostName(), eCAL::Process::GetProcessID());
//  for (const auto& instance : sys_instances)
//  {
//    if (instance != my_host_and_pid)
//    {
//      msg += "\t\t" + instance.first + " (pid: " + std::to_string(instance.second) + ")\n";
//    }
//  }
//
//  if (msg.empty() == false)
//  {
//    std::cout << "\nWARNING: Multiple eCALSys instances detected, eCALSys already running on:\n" << msg
//      << "Please be aware of unintended system behavior!\nOnly ONE eCALSys instance should be active!" << std::endl;
//  }
//
//  /************************************************************************/
//  /*  Check and set "use_only_local" and "use_all_on_local" options       */
//  /************************************************************************/
//  if (local_tasks_only || use_localhost_for_all_tasks || disable_target_checks)
//  {
//    EcalSys::Options options;
//    if (local_tasks_only)
//    {
//      options.local_tasks_only = true;
//    }
//    if (use_localhost_for_all_tasks)
//    {
//      options.use_localhost_for_all_tasks = true;
//    }
//    if (disable_target_checks)
//    {
//      options.check_target_reachability = false;
//    }
//
//    ecalsys_inst->SetOptions(options);
//  }
//
//  /************************************************************************/
//  /*  Check if all targets are reachable before start / restart tasks     */
//  /************************************************************************/
//  bool all_targets_reachable = true;
//  auto options = ecalsys_inst->GetOptions();
//  if (options.check_target_reachability == true &&
//      (options.use_localhost_for_all_tasks == false && options.local_tasks_only == false) &&
//      (start_flag || restart_flag || monitor_flag))
//  {
//    all_targets_reachable = WaitForAllTargetsReachable(ecalsys_inst);
//  }
//
//  if (all_targets_reachable)
//  {
//    /************************************************************************/
//    /*  Update Tasks from cloud                                             */
//    /************************************************************************/
//    if (!disable_update_from_cloud && (stop_flag || restart_flag || monitor_flag))
//    {
//      ecalsys_inst->UpdateFromCloud();
//    }
//
//    /************************************************************************/
//    /*  Stop tasks                                                          */
//    /************************************************************************/
//    if (stop_flag || restart_flag)
//    {
//      std::string monitoring_str;
//      int sleep_time = 0;
//      while (sleep_time <= 2000)
//      {
//        eCAL::Monitoring::GetMonitoring(monitoring_str);
//        eCAL::Process::SleepMS(100);
//        sleep_time += 100;
//      }
//
//      StopTasks(ecalsys_inst);
//      sleep_time = 0;
//      while (sleep_time <= 10000)
//      {
//        eCAL::Process::SleepMS(100);
//        sleep_time += 100;
//      }
//    }
//
//    /************************************************************************/
//    /*  Start tasks                                                         */
//    /************************************************************************/
//    if (start_flag || restart_flag)
//    {
//      StartTasks(ecalsys_inst);
//    }
//
//    /************************************************************************/
//    /*  Monitor                                                             */
//    /************************************************************************/
//    if (monitor_flag)
//    {
//      std::cout << std::endl << "Monitoring... [Press ESC to exit]" << std::endl << std::endl;
//
//      int char_in = -1;
//      while (!exit_command_received && (char_in != 0x1B /*ESC*/))
//      {
//        if (_kbhit())
//        {
//          char_in = _getch();
//        }
//        eCAL::Process::SleepMS(1);
//      }
//
//      std::cout << std::endl << "Exiting... " << std::endl;
//    }
//  }
//
//  ecalsys_inst->WaitForTaskActions();
//
//  /************************************************************************/
//  /*  finalize eCAL API                                                   */
//  /************************************************************************/
//  eCAL::Finalize();
//
//  return EXIT_SUCCESS;
//}
