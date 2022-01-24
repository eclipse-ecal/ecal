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

#include "ecalsys_gui.h"
#include "help_window.h"
#include <QtWidgets/QApplication>

#include "globals.h"
#include "ecalsys/ecal_sys_logger.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505)
#endif
#include <tclap/CmdLine.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <custom_tclap/advanced_tclap_output.h>
#include <custom_tclap/fuzzy_value_switch_arg_bool.h>

#include <thread>

#ifdef WIN32
#include <ecal_utils/command_line.h>
#endif // WIN32


int main(int argc, char *argv[])
{
  //////////////////////////////////////////////////////////////////////////////
  // Parse the command line
  //////////////////////////////////////////////////////////////////////////////

  TCLAP::CmdLine cmd(ECAL_SYS_LIB_NAME, ' ', ECAL_SYS_VERSION_STRING, false);

  TCLAP::UnlabeledMultiArg<std::string> unlabled_config_arg  (     "config_",                    "The Configuration file to load", false, "Path");
  
  TCLAP::SwitchArg help_arg                                  ("h", "help",                       "Displays help.",                 false);
  TCLAP::SwitchArg version_arg                               ("v", "version",                    "Displays version information.",  false);
  TCLAP::ValueArg<std::string> config_arg                    ("c", "config",                     "The Configuration file to load", false,  "", "Path");

  TCLAP::SwitchArg start_arg                                 ("s", "start",                      "Start all tasks",                false);
  TCLAP::SwitchArg stop_arg                                  ("x", "stop",                       "Stop all tasks",                 false);
  TCLAP::SwitchArg restart_arg                               ("r", "restart",                    "Restart all tasks",              false);

  CustomTclap::FuzzyValueSwitchArgBool local_tasks_only_arg           ("", "local-tasks-only",            "Only tasks on local host will be considered.",                      false, false, "true|false");
  CustomTclap::FuzzyValueSwitchArgBool use_localhost_for_all_tasks_arg("", "use-localhost-for-all-tasks", "All tasks will be considered as being on local host. ",             false, false, "true|false");

  std::vector<TCLAP::Arg*> arg_vector =
  {
    &help_arg,
    &version_arg,
    &config_arg,
    &start_arg,
    &stop_arg,
    &restart_arg,
    &local_tasks_only_arg,
    &use_localhost_for_all_tasks_arg,
    &unlabled_config_arg,
  };
  
  for (auto arg_iterator = arg_vector.rbegin(); arg_iterator != arg_vector.rend(); arg_iterator++)
  {
    cmd.add(*arg_iterator);
  }

  std::stringstream tclap_output_stream;
  CustomTclap::AdvancedTclapOutput advanced_tclap_output(std::vector<std::ostream*>{ &tclap_output_stream, &std::cout }, 75);
  advanced_tclap_output.setArgumentHidden(&unlabled_config_arg, true);

  cmd.setOutput(&advanced_tclap_output);
 
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

  QApplication a(argc, argv);

  a.setOrganizationName("Continental");
  a.setOrganizationDomain("continental-corporation.com");
  a.setApplicationName("ecalsysgui");
  a.setApplicationDisplayName("eCAL Sys GUI");

  //////////////////////////////////////
  // --help
  // --version
  //////////////////////////////////////
  if (help_arg.isSet() || version_arg.isSet())
  {
    tclap_output_stream.clear();

    if (help_arg.isSet())
    {
      advanced_tclap_output.usage(cmd);
    }
    else if (version_arg.isSet())
    {
      advanced_tclap_output.version(cmd);
    }

#ifdef WIN32
    // On Windows we display the command line help as GUI window. That approach
    // is somehow standard on Windows, as Windows will not print the
    // stdout/stderr streams to the console the app was started from.

    HelpWindow w(tclap_output_stream.str());

    w.show();
    return a.exec();
#else // WIN32
    return 0;
#endif // WIN32
  }

  //////////////////////////////////////////////////////////////////////////////
  // Start eCAL Sys
  //////////////////////////////////////////////////////////////////////////////

  // Just make sure that eCAL is initialized
  eCAL::Initialize(argc, argv, "eCALSysGUI", eCAL::Init::All);

  //////////////////////////////////////
  // --config
  //////////////////////////////////////
  std::string start_config = "";

  if (config_arg.isSet())
  {
    start_config = config_arg.getValue();
  }
  else if (unlabled_config_arg.isSet())
  {
    start_config = unlabled_config_arg.getValue()[0];
  }

  if (!start_config.empty())
  {
    try
    {
      Globals::EcalSysInstance()->LoadConfig(start_config);
    }
    catch (std::runtime_error& e)
    {
      EcalSysLogger::Log("Error loading config from \"" + std::string(start_config) + "\": " + e.what(), spdlog::level::err);
    }
  }

  //////////////////////////////////////
  // Option validation
  //////////////////////////////////////

  // Both local-tasks-only and use-localhost-for-all-tasks is given
  if ((local_tasks_only_arg.isSet() && local_tasks_only_arg.getValue())
    && (use_localhost_for_all_tasks_arg.isSet() && use_localhost_for_all_tasks_arg.getValue()))
  {
    EcalSysLogger::Log("Error: Cannot use \"" + local_tasks_only_arg.getName()      + "\" option when using \"" + use_localhost_for_all_tasks_arg.getName() + "\" option.", spdlog::level::err);
  }

  auto options         = Globals::EcalSysInstance()->GetOptions();
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

  if (options_changed)
    Globals::EcalSysInstance()->SetOptions(options);

  //////////////////////////////////////
  // --start
  // --stop
  // --restart
  //////////////////////////////////////
  if (start_arg.isSet() || stop_arg.isSet() || restart_arg.isSet())
  {
    if (Globals::EcalSysInstance()->IsConfigOpened())
    {
      std::this_thread::sleep_for(std::chrono::seconds(2));

      if (start_arg.isSet())
      {
        Globals::EcalSysInstance()->StartTasks();
      }
      else if (stop_arg.isSet())
      {
        Globals::EcalSysInstance()->UpdateFromCloud();
        Globals::EcalSysInstance()->StopTasks();
      }
      else if (restart_arg.isSet())
      {
        Globals::EcalSysInstance()->UpdateFromCloud();
        Globals::EcalSysInstance()->RestartTasks();
      }
    }
    else
    {
      EcalSysLogger::Log("Starting / Stopping / Restarting tasks requires a config being loaded.");
    }
  }

  EcalsysGui* w = new EcalsysGui();
  w->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
  w->show();

  auto return_code = a.exec();

  delete Globals::EcalSysInstance();

  eCAL::Finalize();

  return return_code;
}
