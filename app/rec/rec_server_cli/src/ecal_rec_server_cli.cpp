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
 * eCAL Rec Console Application
**/

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <condition_variable>

#include <ecal/ecal.h>
#include <ecal/service/client.h>
#include <ecal_utils/string.h>
#include <rec_server_core/rec_server.h>
#include <rec_client_core/ecal_rec_defs.h>

#include <ecal/msg/protobuf/client.h>
#include <ecal/msg/protobuf/server.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings 
#endif
#include <ecal/app/pb/rec/server_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <rec_server_service.h>

#include <ecal_utils/command_line.h>
#include <ecal_utils/str_convert.h>
#include <ecal_utils/win_cp_changer.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505) // disable tclap warning (unreferenced local function has been removed)
#endif
#include "tclap/CmdLine.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <custom_tclap/advanced_tclap_output.h>
#include <custom_tclap/fuzzy_value_switch_arg_double.h>
#include <custom_tclap/fuzzy_value_switch_arg_unsigned_longlong.h>
#include <custom_tclap/fuzzy_duo_value_arg_unsigned_longlong_string.h>

#include <command_executor.h>

#include <commands/activate.h>
#include <commands/comment.h>
#include <commands/deactivate.h>
#include <commands/delete.h>
#include <commands/get_config.h>
#include <commands/load_config.h>
#include <commands/record.h>
#include <commands/save_pre_buffer.h>
#include <commands/set_config.h>
#include <commands/sleep.h>
#include <commands/status.h>
#include <commands/stop_recording.h>
#include <commands/upload.h>

#include "bytes_to_pretty_string_utils.h"

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <signal.h>

// Win32 Console Handler (CTRL+C, Close Window)
BOOL WINAPI ConsoleHandler(DWORD);

#else // WIN32

#include <signal.h>

/** POSIX signal handler */
void SignalHandler(int s);

#endif // WIN32

/**************************************
* Global Methods
**************************************/

void InterruptStdIn();

bool IsRecorderBusy        (bool print_status = false);
bool IsAnyClientFlushing   (bool print_status = false);
bool IsAnyClientUploading  (bool print_status = false);
bool IsBuiltInFtpServerBusy(bool print_status = false);

/**************************************
 * Global Variables
 **************************************/

// Rec Sever instance and rec_server_service. We will only use one of those, depending on the remote-control setting
std::shared_ptr<eCAL::rec_server::RecServer>                                                rec_server_instance;
std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>> remote_rec_server_service;

// Ptrs for eCAL Rec Server Service (only used in non-remote control mode)
std::shared_ptr<eCAL::rec_cli::RecServerService>                                            rec_server_service;
std::shared_ptr<eCAL::protobuf::CServiceServer<eCAL::pb::rec_server::EcalRecServerService>> rec_server_service_server;

// Ptr for Command Executor
std::unique_ptr<eCAL::rec_cli::CommandExecutor> command_executor;

// Variables for safe shutdown
std::mutex                            ecal_rec_exit_mutex_;
std::condition_variable               ecal_rec_exit_cv_;
std::chrono::steady_clock::time_point ctrl_exit_until(std::chrono::steady_clock::duration::max());
bool                                  ctrl_exit_event(false);

// Ptr for one-shot record command. This is the only command that can be interrupted and thus needs an instance accessible by the signal handler
std::unique_ptr<eCAL::rec_cli::command::Record> record_command;

/**************************************
* Main Function
**************************************/

int main(int argc, char** argv)
{
#ifdef WIN32
  (void)argc; // suppress unused warning
  (void)argv; // suppress unused warning

  EcalUtils::WinCpChanger win_cp_changer(CP_UTF8); // The WinCpChanger will set the Codepage back to the original, once destroyed
#endif // WIN32

  // Define the command line object.
  TCLAP::CmdLine cmd(ECAL_REC_NAME, ' ', ECAL_REC_VERSION_STRING);

  /////////////////////////////////////////////////
  /// Rec Server args
  /////////////////////////////////////////////////
  
  TCLAP::UnlabeledMultiArg<std::string> unlabled_config_arg            ("config_", "The Configuration file to load.", false, "Path");

  TCLAP::ValueArg<std::string>          config_arg                     ("c", "config",                "The configuration file to load.", false, "", "Path");
  TCLAP::SwitchArg                      no_default_config_arg          ("",  "no-default",            "Prevent creating a default config, when no config file is loaded. This prevents having a default root-dir, meas-name and localhost recorder. Not applicable in remote-control mode.", false);

  TCLAP::SwitchArg                      remote_control_arg             ("",  "remote-control",        "Remote control another eCAL Rec host application.", false);
  TCLAP::ValueArg<std::string>          remote_control_host_arg        ("",  "remote-host",           "Set the hostname for remote-controlling.", false, "", "Hostname");

  CustomTclap::FuzzyValueSwitchArgDouble start_recording_arg           ("r", "record",                "Start a new recording. If a duration in seconds is provided, the recording will be stopped after that time. Otherwise, it will keep running until stopped by the user or a succeeding stop-recording call.", false, -1.0, "Seconds");
  TCLAP::SwitchArg                      save_pre_buffer_arg            ("",  "savebuffer",            "Save the content of the pre-buffer as its own recording. Only available in remote-control mode.", false);
  TCLAP::SwitchArg                      stop_arg                       ("x", "stop",                  "Stop the currently running recording. Only available in remote-control mode.", false);
  TCLAP::SwitchArg                      activate_arg                   ("",  "activate",              "Activate the recorder and all clients.", false);
  TCLAP::SwitchArg                      deactivate_arg                 ("",  "deactivate",            "Deactivates the recorder and all clients.", false);
  CustomTclap::FuzzyDuoValueArgUnsignedLongLongString add_comment_arg  ("",  "comment",               "Adds a comment to a measurement. If no ID is given, the comment is added to the last measurement. When your intention is to comment a measurement started with " + start_recording_arg.toString() + ", you should leave out the MeasID or set it to 0.", false, 0, "", "MeasID", "Comment");
  TCLAP::ValueArg<uint64_t>             delete_meas_arg                ("",  "delete",                "Delete a measurement from the host and all clients.", false, 0, "MeasID");
  CustomTclap::FuzzyValueSwitchArgUnsignedLongLong upload_meas_arg     ("",  "upload",                "Upload the given measurement. If no measurement ID is provided, all measurements will be uploaded. When " + start_recording_arg.toString() + " is provided with a duration, the mesasurement will be uploaded after the recorder has finished the measurement. As a recorder may still be flushing, a small delay (1s) is introduced before uploading in this case. Use interactive mode, if this is not sufficient." , false, 0, "MeasID");

  TCLAP::SwitchArg                      status_arg                     ("s", "status",                "Print the status of a remote-controlled recorder.", false);
  TCLAP::ValueArg<uint64_t>             meas_status_arg                ("",  "meas-status",           "Print the status of the given measurement. Only available in remote-control mode.", false, 0, "MeasID");
  TCLAP::ValueArg<std::string>          client_status_arg              ("",  "client-status",         "Print the status of the given recorder client. Only available in remote-control mode.", false, "", "Hostname");
  TCLAP::SwitchArg                      get_config_arg                 ("",  "get-config",            "Print the configuration of the recorder to the console.", false);

  /////////////////////////////////////////////////
  /// Rec Client CLI args (-> override config)
  /////////////////////////////////////////////////
  
  // Client management
  TCLAP::MultiArg<std::string>  set_client_arg     ("", "set-client",    "Sets which hosts shall be recorded by the given client. Use the syntax \"Hostname:Host,To,Record\". For instance, to let PC1 record itself and PC2, use \"PC1:PC1,PC2\". If no tailing list is provided, the client will record topcis from all hosts by default. If a configuration file is being loaded, this will override the setting from the config.", false, "Hostname:Hosts,To,Record");
  TCLAP::MultiArg<std::string>  set_addons_arg     ("", "set-addons",    "Sets the addons that the given client shall enable. Use the syntax \"Hostname:List,Of,Addon,IDs\". You can only set enabled addons for clients that have already been added; i.e. the client will not be added automatically. If no tailing list of addon IDs is provided, all addons will be disabled. If a configuration file is being loaded, this will override the setting from the config.",        false, "Hostname:List,Of,Addon,IDs");
  TCLAP::MultiArg<std::string>  remove_client_arg  ("", "remove-client", "Removes the given client. If the client had any addons enabled, those are removed as well. Not available in remote-control mode.",        false, "Hostname");

  // Settings args
  TCLAP::ValueArg<std::string>  pre_buffer_arg     ("b", "pre-buffer",      "Pre-buffer data for some seconds. To turn off pre-buffering, provide a value <= 0.0. If a configuration file is being loaded, this will override the setting from the config.", false, "-1.0", "Seconds");
  TCLAP::ValueArg<std::string>  blacklist_arg      ("",  "blacklist",       "Record all topics except the listed ones (Comma separated list, e.g.: \"Topic1,Topic2\"). If a configuration file is being loaded, this will override the setting from the config.", false, "", "List");
  TCLAP::ValueArg<std::string>  whitelist_arg      ("",  "whitelist",       "Only record these topics (Comma separated list, e.g.: \"Topic1,Topic2\"). If a configuration file is being loaded, this will override the setting from the config.", false, "", "List");

  // Job-config args
  TCLAP::ValueArg<std::string>  meas_root_dir_arg  ("d", "meas-root-dir",   "Root dir used for recording when " + start_recording_arg.toString() + " is set. If a configuration file is being loaded, this will override the setting from the config.", false, "", "Path");
  TCLAP::ValueArg<std::string>  meas_name_arg      ("n", "meas-name",       "Name of the measurement, when " + start_recording_arg.toString() + " is set. This will create a folder in the directory provided by " + meas_root_dir_arg.toString() + ". If a configuration file is being loaded, this will override the setting from the config.",     false, "", "Directory");
  TCLAP::ValueArg<std::string>  max_file_size_arg  ("",  "max-file-size",   "Maximum file size of the recording files, when " + start_recording_arg.toString() + " is set. If a configuration file is being loaded, this will override the setting from the config.", false, "", "megabytes");
  TCLAP::ValueArg<std::string>  description_arg    ("",  "description",     "Description stored in the measurement folder, when " + start_recording_arg.toString() + " is set. If a configuration file is being loaded, this will override the setting from the config.", false, "", "String");
  CustomTclap::FuzzyValueSwitchArgBool enable_one_file_per_topic_arg   ("", "enable-one-file-per-topic", "Whether to separate each topic in HDF5 file. This helps faster file transfer and less network congestion in case of interest of specific topics only.", false, false, "yes/no");

  TCLAP::ValueArg<std::string>         ftp_server_arg            ("", "ftp-server",          "The server where to upload to when uploading a measurement. Use \"internal\" for the integrated FTP Server. When using an external FTP Server, provide it in the following form: ftp://USERNAME:PASSWORD@HOSTNAME:PORT/path/to/root_dir. If a configuration file is being loaded, this will override the setting from the config.", false, "", "FTP_Server");
  CustomTclap::FuzzyValueSwitchArgBool delete_after_upload_arg   ("", "delete-after-upload", "Whether to delete the local measurement files after they have been uploaded to an FTP server. If a configuration file is being loaded, this will override the setting from the config.", false, false, "yes/no");
  CustomTclap::FuzzyValueSwitchArgBool enable_built_in_client_arg("", "enable-built-in-client", "Whether the built-in recorder client of the host-application shall be used for recording. If turned off, the host application will rely on the presence of a separate rec-client for localhost recording. If a configuration file is being loaded, this will override the setting from the config.", false, false, "yes/no");

  /////////////////////////////////////////////////
  /// Interactive mode
  /////////////////////////////////////////////////

  TCLAP::SwitchArg                      interactive_dont_exit_arg      ("",  "interactive-dont-exit", "When in interactive mode, this option prevents eCAL Rec from exiting, when stdin is closed.", false);
  TCLAP::SwitchArg                      interactive_arg                ("i", "interactive",           "Start eCAL Rec and listen for commands on stdin. When not in remote-control mode itself, eCAL Rec will offer the eCAL Rec Service for being remote-controlled. Note that eCAL Rec will exit, when stdin is closed. To prevent that, combine this option with " + interactive_dont_exit_arg.toString()+ ".", false);

  std::vector<TCLAP::Arg*> arg_vector =
  {
    &config_arg,
    &no_default_config_arg,
    &remote_control_arg,
    &remote_control_host_arg,

    &start_recording_arg,
    &save_pre_buffer_arg,
    &stop_arg,
    &activate_arg,
    &deactivate_arg,
    &add_comment_arg,
    &delete_meas_arg,
    &upload_meas_arg,

    &status_arg,
    &meas_status_arg,
    &client_status_arg,
    &get_config_arg,

    &set_client_arg,
    &set_addons_arg,
    &remove_client_arg,
    &pre_buffer_arg,
    &blacklist_arg,
    &whitelist_arg,
    &meas_root_dir_arg,
    &meas_name_arg,
    &max_file_size_arg,
    &description_arg,
    &enable_one_file_per_topic_arg,
    &ftp_server_arg,
    &delete_after_upload_arg,

    &interactive_arg,
    &interactive_dont_exit_arg,

    &unlabled_config_arg,
  };

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

  // Check cfg file name
  std::string config_file_name;
  bool        config_file_set(false);
  if (config_arg.isSet())
  {
    config_file_name = config_arg.getValue();
    config_file_set  = true;
  }
  else if (unlabled_config_arg.isSet())
  {
    config_file_name = unlabled_config_arg.getValue()[0];
    config_file_set  = true;
  }

  // Create record command, so it is interruptible by the signal handler
  record_command = std::make_unique<eCAL::rec_cli::command::Record>();

  /************************************************************************/
  /* Print welcome header                                                 */
  /************************************************************************/

  std::string rec_name     = std::string("eCAL Recorder Host Application");
  std::string rec_version  = std::string("Version: ") + ECAL_REC_VERSION_STRING;
  std::string ecal_version = std::string("eCAL ") + ECAL_VERSION + " (" + ECAL_DATE + ")";

  EcalUtils::String::CenterString(rec_name,     ' ', 73);
  EcalUtils::String::CenterString(rec_version,  ' ', 73);
  EcalUtils::String::CenterString(ecal_version, ' ', 73);

  std::stringstream header_ss;
  header_ss << "===============================================================================" << std::endl;
  header_ss << "===" << rec_name << "==="                                                        << std::endl;
  header_ss << "===" << rec_version << "==="                                                     << std::endl;
  header_ss << "===" << ecal_version << "==="                                                    << std::endl;
  header_ss << "===============================================================================" << std::endl;
  header_ss                                                                                      << std::endl;

  header_ss << std::endl;

  std::cout << header_ss.str();

  /************************************************************************/
  /* Argument validation                                                  */
  /************************************************************************/

  // remote-host without remote-control
  if (remote_control_host_arg.isSet() && !remote_control_arg.isSet())
  {
    std::cerr << "Error: " << remote_control_host_arg.toString() << " can only be used in combination with " << remote_control_arg.toString() << "." << std::endl;
    return EXIT_FAILURE;
  }

  /************************************************************************/
  /*  Remote control mode                                                 */
  /************************************************************************/
  if (remote_control_arg.isSet()) // Remote-control-mode
  {
    eCAL::Initialize("eCALRec-Remote", eCAL::Init::All);
    eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "Running");

    remote_rec_server_service = std::make_shared<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>();
  }
  else                            // Non-remote control mode
  {
    eCAL::Initialize("eCALRec", eCAL::Init::All);

    rec_server_instance = std::make_shared<eCAL::rec_server::RecServer>();

    // Create the eCALRec server service
    rec_server_service        = std::make_shared<eCAL::rec_cli::RecServerService>(rec_server_instance);
    rec_server_service_server = std::make_shared<eCAL::protobuf::CServiceServer<eCAL::pb::rec_server::EcalRecServerService>>(rec_server_service);
  }

  // Give the monitor some time to connect to eCAL and update
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // Create the command executor for interactive mode (we need this pointer to be valid even in non-interactive mode, as the signal handler will try interrupting it)
  command_executor = std::make_unique<eCAL::rec_cli::CommandExecutor>(rec_server_instance, remote_control_host_arg.getValue(), remote_rec_server_service);

  /************************************************************************/
  /* CTRL+C handler / Signal handler                                      */
  /************************************************************************/
  std::cout << std::endl;
#ifdef WIN32

  if (!remote_control_arg.isSet())
  {
    std::string attention_string1 = "!! Attention !!";
    std::string attention_string2 = "Closing the console with the [X] button may lead to incomplete measurements";
    EcalUtils::String::CenterString(attention_string1, ' ', 73);
    EcalUtils::String::CenterString(attention_string2, ' ', 73);

    std::cout << std::endl;
    std::cout << attention_string1 << std::endl;
    std::cout << attention_string2 << std::endl;
    std::cout << std::endl;
  }


  if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE))
  {
    std::cout << "Press Ctrl+C to exit" << std::endl;
  }
  else
  {
    std::cerr << "Unable to set Ctrl+C handler" << std::endl;
  }

#else // WIN32
  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = SignalHandler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGTERM, &sigIntHandler, NULL);
  if (sigaction(SIGINT, &sigIntHandler, NULL) == 0)
  {
    std::cout << "Press Ctrl+C to exit" << std::endl;
  }
  else
  {
    std::cerr << "Unable to set signal handler: " << strerror(errno) << std::endl;    
  }

#endif
  std::cout << std::endl;

  // ==========================================================================
  // ======================== Configuration parameters ========================
  // ==========================================================================

  // If an error occured and we don't use interactive mode, we want to return EXIT_FAILED at the end
  bool set_config_error_occured(false);

  /************************************************************************/
  /* Load the configuration                                               */
  /************************************************************************/
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      if (config_file_set)
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
        if (rec_server_instance)
        {
          error = eCAL::rec_cli::command::LoadConfig().Execute(rec_server_instance, { config_file_name });
        }
        else
        {
          error = eCAL::rec_cli::command::LoadConfig().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, { config_file_name });
        }

        if (error)
        {
          std::cerr << "ERROR loading config: " << error.ToString() << std::endl;
          set_config_error_occured = true;
        }
      }
      else if (rec_server_instance && !no_default_config_arg.isSet())
      {
        // This host records everything
        rec_server_instance->SetEnabledRecClients({ { eCAL::Process::GetHostName(), eCAL::rec_server::ClientConfig()} });

        rec_server_instance->SetMeasRootDir("$TARGET{OSSELECT WIN \"C:\" LINUX \"$TARGET{ENV HOME}\"}/ecal_meas");
        rec_server_instance->SetMeasName("$HOST{TIME}_measurement");
        rec_server_instance->SetDescription(
          R"(Measurement started by: $HOST{USERNAME}@$HOST{HOSTNAME}
    Date: $HOST{TIME %F %R}
    )"
    );
        rec_server_instance->SetMaxFileSizeMib(1000);

      }
    }
  }


  /************************************************************************/
  /* Set Client                                                           */
  /************************************************************************/
  if (set_client_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetClient(rec_server_instance, set_client_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetClient(remote_control_host_arg.getValue(), remote_rec_server_service, set_client_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << set_client_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Set Addons                                                           */
  /************************************************************************/
  if (set_addons_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetAddons(rec_server_instance, set_addons_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetAddons(remote_control_host_arg.getValue(), remote_rec_server_service, set_addons_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << set_addons_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Remove Clients                                                       */
  /************************************************************************/
  if (remove_client_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::RemoveClient(rec_server_instance, remove_client_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::RemoveClient(remote_control_host_arg.getValue(), remote_rec_server_service, remove_client_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << remove_client_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Pre buffer                                                           */
  /************************************************************************/
  if (pre_buffer_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetPreBuffer(rec_server_instance, pre_buffer_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetPreBuffer(remote_control_host_arg.getValue(), remote_rec_server_service, pre_buffer_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << pre_buffer_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Blacklist                                                            */
  /************************************************************************/
  if (blacklist_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetBlacklist(rec_server_instance, blacklist_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetBlacklist(remote_control_host_arg.getValue(), remote_rec_server_service, blacklist_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << blacklist_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Whitelist                                                            */
  /************************************************************************/
  if (whitelist_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetWhitelist(rec_server_instance, whitelist_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetWhitelist(remote_control_host_arg.getValue(), remote_rec_server_service, whitelist_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << whitelist_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Set Meas root dir                                                    */
  /************************************************************************/
  if (meas_root_dir_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetMeasRootDir(rec_server_instance, meas_root_dir_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetMeasRootDir(remote_control_host_arg.getValue(), remote_rec_server_service, meas_root_dir_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << meas_root_dir_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Set Meas Name                                                        */
  /************************************************************************/
  if (meas_name_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetMeasName(rec_server_instance, meas_name_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetMeasName(remote_control_host_arg.getValue(), remote_rec_server_service, meas_name_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << meas_name_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Max file size                                                        */
  /************************************************************************/
  if (max_file_size_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetMaxFileSize(rec_server_instance, max_file_size_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetMaxFileSize(remote_control_host_arg.getValue(), remote_rec_server_service, max_file_size_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << max_file_size_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Description                                                          */
  /************************************************************************/
  if (description_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetDescription(rec_server_instance, description_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetDescription(remote_control_host_arg.getValue(), remote_rec_server_service, description_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << description_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Enable one file per topic                                            */
  /************************************************************************/
  if (enable_one_file_per_topic_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetOneFilePerTopicEnabled(rec_server_instance, enable_one_file_per_topic_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetOneFilePerTopicEnabled(remote_control_host_arg.getValue(), remote_rec_server_service, enable_one_file_per_topic_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << enable_one_file_per_topic_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }

  }


  /************************************************************************/
  /* Ftp Server                                                           */
  /************************************************************************/
  if (ftp_server_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetFtpServer(rec_server_instance, ftp_server_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetFtpServer(remote_control_host_arg.getValue(), remote_rec_server_service, ftp_server_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << ftp_server_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Delete after upload                                                  */
  /************************************************************************/
  if (delete_after_upload_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::SetConfig::SetDeleteAfterUpload(rec_server_instance, delete_after_upload_arg.getValue());
      }
      else
      {
        error = eCAL::rec_cli::command::SetConfig::SetDeleteAfterUpload(remote_control_host_arg.getValue(), remote_rec_server_service, delete_after_upload_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << delete_after_upload_arg.toString() << ": " << error.ToString() << std::endl;
        set_config_error_occured = true;
      }
    }
  }

  // ==========================================================================
  // ======================== Single shot commands     ========================
  // ==========================================================================

  bool command_error_occured(false);

  /************************************************************************/
  /* Activate                                                             */
  /************************************************************************/
  if (activate_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      if (set_config_error_occured)
      {
        std::cerr << "WARNING: Previous set-config error detected. Not executing --" + activate_arg.toString() + " command." << std::endl;
      }
      else
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
        if (rec_server_instance)
        {
          error = eCAL::rec_cli::command::Activate().Execute(rec_server_instance, {});
        }
        else
        {
          error = eCAL::rec_cli::command::Activate().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, {});
        }

        if (error)
        {
          std::cerr << "ERROR executing " << activate_arg.toString() << ": " << error.ToString() << std::endl;
          command_error_occured = true;
        }
      }
    }
  }

  /************************************************************************/
  /* Save pre buffer                                                      */
  /************************************************************************/
  if (save_pre_buffer_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      if (set_config_error_occured)
      {
        std::cerr << "WARNING: Previous set-config error detected. Not executing --" + save_pre_buffer_arg.toString() + " command." << std::endl;
      }
      else
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
        if (rec_server_instance)
        {
          // In direct mode there cannot be any buffered data, so we cannot save a pre-buffer
          error = eCAL::rec::Error(eCAL::rec::Error::ErrorCode::COMMAND_ONLY_AVAILABLE_IN_REMOTE_MODE, save_pre_buffer_arg.toString());
        }
        else
        {
          error = eCAL::rec_cli::command::SavePreBuffer().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, {});
        }

        if (error)
        {
          std::cerr << "ERROR executing " << save_pre_buffer_arg.toString() << ": " << error.ToString() << std::endl;
          command_error_occured = true;
        }
      }
    }
  }

  /************************************************************************/
  /* Start recording                                                      */
  /************************************************************************/
  bool need_to_wait_for_local_recording = false;
  if (start_recording_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      if (set_config_error_occured)
      {
        std::cerr << "WARNING: Previous set-config error detected. Not executing --" + start_recording_arg.toString() + " command." << std::endl;
      }
      else
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
        if (rec_server_instance)
        {
          error = record_command->Execute(rec_server_instance, std::chrono::duration<double>(start_recording_arg.getValue()));

          if (!error && (start_recording_arg.getValue() <= 0.0))
          {
            // If we are recording locally, we need to wait for the local
            // Brecorder. We must not exit immediatelly, even if the user didn't
            // set the interactive arg.
            need_to_wait_for_local_recording = true;
          }
        }
        else
        {
          error = record_command->Execute(remote_control_host_arg.getValue(), remote_rec_server_service, std::chrono::duration<double>(start_recording_arg.getValue()));
        }

        if (error)
        {
          std::cerr << "ERROR executing " << start_recording_arg.toString() << ": " << error.ToString() << std::endl;
          command_error_occured = true;
        }
      }
    }
  }

  /************************************************************************/
  /* Stop recording                                                       */
  /************************************************************************/
  if (stop_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      if (set_config_error_occured)
      {
        std::cerr << "WARNING: Previous set-config error detected. Not executing --" + stop_arg.toString() + " command." << std::endl;
      }
      else
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
        if (rec_server_instance)
        {
          error = eCAL::rec::Error(eCAL::rec::Error::ErrorCode::COMMAND_ONLY_AVAILABLE_IN_REMOTE_MODE, stop_arg.toString());
        }
        else
        {
          error = eCAL::rec_cli::command::StopRecording().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, {});
        }

        if (error)
        {
          std::cerr << "ERROR executing " << stop_arg.toString() << ": " << error.ToString() << std::endl;
          command_error_occured = true;
        }
      }
    }
  }

  /************************************************************************/
  /* Add comment                                                          */
  /************************************************************************/
  if (add_comment_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      if (set_config_error_occured)
      {
        std::cerr << "WARNING: Previous set-config error detected. Not executing --" + add_comment_arg.toString() + " command." << std::endl;
      }
      else
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
        if (rec_server_instance)
        {
          if (add_comment_arg.isUnsignedLongLongValueSet() && (add_comment_arg.getUnsignedLongLongValue() != 0))
            error = eCAL::rec_cli::command::Comment().Execute(rec_server_instance, { std::to_string(add_comment_arg.getUnsignedLongLongValue()), add_comment_arg.getStringValue() });
          else
            error = eCAL::rec_cli::command::Comment().Execute(rec_server_instance, { add_comment_arg.getStringValue() });
        }
        else
        {
          if (add_comment_arg.isUnsignedLongLongValueSet() && (add_comment_arg.getUnsignedLongLongValue() != 0))
            error = eCAL::rec_cli::command::Comment().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, { std::to_string(add_comment_arg.getUnsignedLongLongValue()), add_comment_arg.getStringValue() });
          else
            error = eCAL::rec_cli::command::Comment().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, { add_comment_arg.getStringValue() });
        }

        if (error)
        {
          std::cerr << "ERROR executing " << add_comment_arg.toString() << ": " << error.ToString() << std::endl;
          command_error_occured = true;
        }
      }
    }
  }

  /************************************************************************/
  /* Delete                                                               */
  /************************************************************************/
  if (delete_meas_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      if (set_config_error_occured)
      {
        std::cerr << "WARNING: Previous set-config error detected. Not executing --" + delete_meas_arg.toString() + " command." << std::endl;
      }
      else
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
        if (rec_server_instance)
        {
          error = eCAL::rec::Error(eCAL::rec::Error::ErrorCode::COMMAND_ONLY_AVAILABLE_IN_REMOTE_MODE, delete_meas_arg.toString());
        }
        else
        {
          error = eCAL::rec_cli::command::Delete().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, { std::to_string(delete_meas_arg.getValue()) });
        }

        if (error)
        {
          std::cerr << "ERROR executing " << delete_meas_arg.toString() << ": " << error.ToString() << std::endl;
          command_error_occured = true;
        }
      }
    }
  }

  /************************************************************************/
  /* Upload                                                               */
  /************************************************************************/
  if (upload_meas_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      if (set_config_error_occured)
      {
        std::cerr << "WARNING: Previous set-config error detected. Not executing --" + upload_meas_arg.toString() + " command." << std::endl;
      }
      else
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);

        if ((start_recording_arg.isSet() || save_pre_buffer_arg.isSet() || stop_arg.isSet()) && (upload_meas_arg.getValue() == 0))
        {
          // Primivively wait 1 second and hope everybody has finished flushing until then
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (rec_server_instance)
        {
          if(upload_meas_arg.getValue() != 0)
            // Upload specific measurement
            error = eCAL::rec_cli::command::Upload().Execute(rec_server_instance, { std::to_string(upload_meas_arg.getValue()) });
          else
            // Upload all measurements
            error = eCAL::rec_cli::command::Upload().Execute(rec_server_instance, { });
        }
        else
        {
          if(upload_meas_arg.getValue() != 0)
            // Upload specific measurement
            error = eCAL::rec_cli::command::Upload().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, { std::to_string(upload_meas_arg.getValue()) });
          else
            // Upload all measurements
            error = eCAL::rec_cli::command::Upload().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, { });
        }

        if (error)
        {
          std::cerr << "ERROR executing " << upload_meas_arg.toString() << ": " << error.ToString() << std::endl;
          command_error_occured = true;
        }
        else if (!(interactive_arg.isSet() || interactive_dont_exit_arg.isSet()))
        {
          // Primivively wait another second to make sure all clients have at least started to upload. The safe shutdown routine will then wait for all clients to finish uploading.
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
      }
    }
  }

  /************************************************************************/
  /* Deactivate                                                           */
  /************************************************************************/
  if (deactivate_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      if (set_config_error_occured)
      {
        std::cerr << "WARNING: Previous set-config error detected. Not executing --" + deactivate_arg.toString() + " command." << std::endl;
      }
      else
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
        if (rec_server_instance)
        {
          error = eCAL::rec_cli::command::DeActivate().Execute(rec_server_instance, {});
        }
        else
        {
          error = eCAL::rec_cli::command::DeActivate().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, {});
        }

        if (error)
        {
          std::cerr << "ERROR executing " << deactivate_arg.toString() << ": " << error.ToString() << std::endl;
          command_error_occured = true;
        }
      }
    }
  }


  // ==========================================================================
  // ======================== Status commands          ========================
  // ==========================================================================
  
  bool status_error_occured(false);

  /************************************************************************/
  /* Recorder status                                                      */
  /************************************************************************/
  if (status_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      if (set_config_error_occured)
      {
        std::cerr << "WARNING: Previous set-config error detected. Not executing --" + status_arg.toString() + " command." << std::endl;
      }
      else
      {
        eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
        if (rec_server_instance)
        {
          error = eCAL::rec_cli::command::Status().Execute(rec_server_instance, {});
        }
        else
        {
          error = eCAL::rec_cli::command::Status().printStatus(remote_control_host_arg.getValue(), remote_rec_server_service);
        }

        if (error)
        {
          std::cerr << "ERROR executing " << status_arg.toString() << ": " << error.ToString() << std::endl;
          status_error_occured = true;
        }
      }
    }
  }

  /************************************************************************/
  /* Meas status                                                          */
  /************************************************************************/
  if (meas_status_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        // It doesn't make sense to retrieve the status of a recorder that hasn't been doing anything
        error = eCAL::rec::Error(eCAL::rec::Error::ErrorCode::COMMAND_ONLY_AVAILABLE_IN_REMOTE_MODE, meas_status_arg.toString());
      }
      else
      {
        error = eCAL::rec_cli::command::Status().printJobState(remote_control_host_arg.getValue(), remote_rec_server_service, meas_status_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << meas_status_arg.toString() << ": " << error.ToString() << std::endl;
        status_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Client status                                                        */
  /************************************************************************/
  if (client_status_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        // It doesn't make sense to retrieve the status of a recorder that hasn't been doing anything
        error = eCAL::rec::Error(eCAL::rec::Error::ErrorCode::COMMAND_ONLY_AVAILABLE_IN_REMOTE_MODE, client_status_arg.toString());
      }
      else
      {
        error = eCAL::rec_cli::command::Status().printClientInformation(remote_control_host_arg.getValue(), remote_rec_server_service, client_status_arg.getValue());
      }

      if (error)
      {
        std::cerr << "ERROR executing " << client_status_arg.toString() << ": " << error.ToString() << std::endl;
        status_error_occured = true;
      }
    }
  }

  /************************************************************************/
  /* Get Config                                                           */
  /************************************************************************/
  if (get_config_arg.isSet())
  {
    bool is_exiting;
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      is_exiting = ctrl_exit_event;
    }

    if (!is_exiting)
    {
      eCAL::rec::Error error(eCAL::rec::Error::ErrorCode::GENERIC_ERROR);
      if (rec_server_instance)
      {
        error = eCAL::rec_cli::command::GetConfig().Execute(rec_server_instance, {});
      }
      else
      {
        error = eCAL::rec_cli::command::GetConfig().Execute(remote_control_host_arg.getValue(), remote_rec_server_service, {});
      }

      if (error)
      {
        std::cerr << "ERROR executing " << get_config_arg.toString() << ": " << error.ToString() << std::endl;
        status_error_occured = true;
      }
    }
  }

  // ==========================================================================
  // ======================== Blocking behaviour       ========================
  // ==========================================================================

  /************************************************************************/
  /* Recording in non-interactive-mode                                    */
  /************************************************************************/
  if (!interactive_arg.isSet() && !interactive_dont_exit_arg.isSet() && need_to_wait_for_local_recording)
  {
    std::cout << "A recording is running. eCAL Rec will exit, when the recording is stopped." << std::endl;

    for (;;)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      {
        // Exit if the user pressed CTRL+C / tent SIGINT / etc.
        std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
        if (ctrl_exit_event)
          break;
      }

      // Exit, if the recorder is not recoring any more, e.g. because someone
      // stopped the recording via the eCAL Service
      if (!rec_server_instance->IsRecording())
        break;
    }
  }

  /************************************************************************/
  /* Interactive Mode                                                     */
  /************************************************************************/
  if (interactive_arg.isSet() || interactive_dont_exit_arg.isSet())
  {
#ifdef WIN32
    // Create buffer fo the manual ReadConsoleW call
    std::wstring w_buffer;
    w_buffer.reserve(4096);
#endif // WIN32

    std::cout << "Using interactive mode. Type \"help\" to view a list of all commands." << std::endl;
    for (;;)
    {
      {
        std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
        if (ctrl_exit_event)
          break;
      }

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
      
      if (!success || ctrl_exit_event)
      {
        std::cout << "Stdin closed." << std::endl;
        break;
      }

      {
        std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
        if (ctrl_exit_event)
          break;
      }

      eCAL::rec::Error error = command_executor->ExecuteCommand(line);
      if (error)
      {
        std::cerr << "Error: " << error.ToString() << std::endl;
      }
      if (command_executor->IsExitCommandCalled())
      {
        ctrl_exit_event = true;
        break;
      }
    }
  }

  if (interactive_dont_exit_arg.isSet() && !command_executor->IsExitCommandCalled())
  {
    while (eCAL::Ok())
    {
      {
        std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
        if (ctrl_exit_event)
          break;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  /************************************************************************/
  /* Safe shutdown                                                        */
  /************************************************************************/

  if (rec_server_instance)
  {
    std::cout << "Shutting down eCAL Rec..." << std::endl;

    rec_server_service_server->Destroy(); // Prevent other applications to control this recorder from now on!

    rec_server_instance      ->StopRecording();
    rec_server_instance      ->WaitForPendingRequests();

    // Wait for all writers
    for (int counter = 0;;counter = ((counter + 1) % 10)) // Counter is only used to not print the status every loop
    {
      bool print_status = (counter == 0);
      if (!IsRecorderBusy(print_status))
      {
        break; // Exit because the recorder is finished
      }
      else
      {
        {
          std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
          if (std::chrono::steady_clock::now() > ctrl_exit_until)
          {
            std::cout << "Aborting pending actions" << std::endl;
            break; // Exit anyways
          }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
    }
  }
  else
  {
    std::cout << "Shutting down remote-control for eCAL Rec..." << std::endl;
    remote_rec_server_service->Destroy();
  }

  // TODO: On rare occations, eCAL rec hangs when hitting ctrl+c on linux. Remove the debug output, once that bug is found.
  //       Note, just in case the error vanishes: I moved the mutex lock in the signal handler into the SIGINT/SIGTERM if statements, so the signal handler will only aquire the lock when needed. Previously, the handler aquired the lock before checking if it even needed it.


  {
    std::cerr << "Shutdown: aquiring lock...\n";
    std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_); // Wait for the ConsoleHandler / SignalHandler to exit
    
    std::cerr << "Shutdown: Finalizing eCAL...\n";
    eCAL::Finalize();
  
    std::cerr << "Shutdown: Destroying recorder_command...\n";
    record_command            = nullptr;

    std::cerr << "Shutdown: Destroying command_executor...\n";
    command_executor          = nullptr;

    std::cerr << "Shutdown: Destroying rec_server_service...\n";
    rec_server_service        = nullptr;

    std::cerr << "Shutdown: Destroying rec_server_service_server...\n";
    rec_server_service_server = nullptr;

    std::cerr << "Shutdown: Destroying rec_server_instance...\n";
    rec_server_instance       = nullptr;

    std::cerr << "Shutdown: Destroying remote_rec_server_service...\n";
    remote_rec_server_service = nullptr;
  }

  std::cout << "Exiting" << std::endl;

  // Interactive mode always return EXIT_SUCCESS
  // Non-interactive mode returns EXIT_FAILURE, if any command has returned an error.
  if (interactive_arg.isSet() || interactive_dont_exit_arg.isSet())
    return EXIT_SUCCESS;
  else
    return ((set_config_error_occured || command_error_occured || status_error_occured) ? EXIT_FAILURE : EXIT_SUCCESS);
}

bool IsRecorderBusy(bool print_status)
{
  if (rec_server_instance)
  {
    if(print_status)
      std::cout << "Waiting for the following events:" << std::endl;

    bool still_busy = false;
    still_busy = IsAnyClientFlushing   (print_status) || still_busy;
    still_busy = IsAnyClientUploading  (print_status) || still_busy;
    still_busy = IsBuiltInFtpServerBusy(print_status) || still_busy;

    if (print_status && !still_busy)
    {
      std::cout << "  -- no pending events --" << std::endl;
    }

    return still_busy;
  }
  else
  {
    return false;
  }
}

bool IsAnyClientFlushing   (bool print_status)
{
  int64_t unflushed_frames = 0;
  std::set<uint64_t> job_ids_flushing;

  for (const auto& rec_status : rec_server_instance->GetRecorderStatuses())
  {
    for (const auto& job_status : rec_status.second.first.job_statuses_)
    {
      // Save the currently flushing jobs
      if (job_status.state_ == eCAL::rec::JobState::Flushing)
        job_ids_flushing.emplace(job_status.job_id_);

      // Accumulate HDF5 frames that still need to be flushed
      unflushed_frames += job_status.rec_hdf5_status_.unflushed_frame_count_;

      // Accumulate Addon frames that still need to be flushed
      for (const auto& addon_jobstatus : job_status.rec_addon_statuses_)
      {
        if (addon_jobstatus.second.state_ == eCAL::rec::RecAddonJobStatus::State::Flushing)
          job_ids_flushing.emplace(job_status.job_id_);

        unflushed_frames += addon_jobstatus.second.unflushed_frame_count_;
      }
    }
  }

  if (rec_server_instance->IsUsingBuiltInRecorderEnabled())
  {
    // Even if the built-in recorder is not enabled, we must still wait for it to flush.

    auto built_in_rec_status = rec_server_instance->GetBuiltInRecorderInstanceStatus();

    for (const auto& job_status : built_in_rec_status.job_statuses_)
    {
      // Save the currently flushing jobs
      if (job_status.state_ == eCAL::rec::JobState::Flushing)
        job_ids_flushing.emplace(job_status.job_id_);

      // Accumulate HDF5 frames that still need to be flushed
      unflushed_frames += job_status.rec_hdf5_status_.unflushed_frame_count_;

      // Accumulate Addon frames that still need to be flushed
      for (const auto& job_addon_status : job_status.rec_addon_statuses_)
      {
        if (job_addon_status.second.state_ == eCAL::rec::RecAddonJobStatus::State::Flushing)
          job_ids_flushing.emplace(job_status.job_id_);

        unflushed_frames += job_addon_status.second.unflushed_frame_count_;
      }
    }
  }

  bool busy = (job_ids_flushing.size() != 0);

  // Print status to stdout
  if (print_status && busy)
  {
    std::cout << "  " << job_ids_flushing.size() << (job_ids_flushing.size() == 1 ? (" recording is being flushed") : (" recordings are being flushed")) << " (" << unflushed_frames << " frames left)" << std::endl;
  }

  return busy;
}

bool IsAnyClientUploading  (bool print_status)
{
  // Collect jobs that are uploading
  
  auto job_history = rec_server_instance->GetJobHistory();

  std::map<int64_t, std::map<std::string, std::pair<bool, eCAL::rec::UploadStatus>>> uploading_jobs_;  ///< { MeasId: { Hostname: (Uploading, UploadStatus) }} map that can only grow bigger. It contains all Uploads since the dialog opened (even the finished ones)

  for (const auto& job_history_entry : job_history)
  {
    for (const auto& client_status : job_history_entry.client_statuses_)
    {
      if (client_status.second.job_status_.state_ == eCAL::rec::JobState::Uploading)
      {
        const int64_t                  meas_id       = job_history_entry.local_evaluated_job_config_.GetJobId();
        const std::string&             hostname      = client_status.first;
        const eCAL::rec::UploadStatus& upload_status = client_status.second.job_status_.upload_status_;

        auto known_uploading_meas_it = uploading_jobs_.find(meas_id);
        if (known_uploading_meas_it == uploading_jobs_.end())
        {
          known_uploading_meas_it = uploading_jobs_.emplace(meas_id, std::map<std::string, std::pair<bool, eCAL::rec::UploadStatus>>()).first;
        }

        auto known_uploading_host_it = known_uploading_meas_it->second.find(hostname);
        if (known_uploading_host_it == known_uploading_meas_it->second.end())
        {
          known_uploading_meas_it->second.emplace(hostname, std::pair<bool, eCAL::rec::UploadStatus>(true, upload_status));
        }
      }
    }
  }

  // Lets accumulate all data!
  uint64_t num_total_bytes   (0);
  uint64_t num_uploaded_bytes(0);

  for (const auto& job : uploading_jobs_)
  {
    bool job_uploading(false);

    for (const auto& client_upload_status:  job.second)
    {
      job_uploading      = (job_uploading || client_upload_status.second.first);
      num_total_bytes    += client_upload_status.second.second.bytes_total_size_;
      num_uploaded_bytes += client_upload_status.second.second.bytes_uploaded_;
    }
  }

  uint64_t num_bytes_left = 0;
  if (num_total_bytes > num_uploaded_bytes)
    num_bytes_left = num_total_bytes - num_uploaded_bytes;

  if (print_status && (uploading_jobs_.size() > 0))
  {
    std::cout << "  Uploading " << uploading_jobs_.size() << (uploading_jobs_.size() == 1 ? "recording" : "recordings") << " (" << eCAL::rec_cli::bytesToPrettyString(num_bytes_left) << " left)" << std::endl;
  }

  return (uploading_jobs_.size() > 0);
}

bool IsBuiltInFtpServerBusy(bool print_status)
{
  int open_connections = rec_server_instance->GetInternalFtpServerOpenConnectionCount();
  
  if (print_status && (open_connections > 0))
  {
    std::cout << "  " << open_connections << " open FTP connections" << std::endl;
  }

  return (open_connections > 0);
}

#ifdef WIN32
BOOL WINAPI ConsoleHandler(DWORD dwType)
{
  if (dwType == CTRL_C_EVENT)
  {
    std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
    if (!ctrl_exit_event)
    {
      ctrl_exit_event = true;

      if(command_executor)
        command_executor->Interrupt();

      if (record_command)
        record_command->Interrupt();

      InterruptStdIn();
    }
    return TRUE;
  }
  else if (dwType == CTRL_BREAK_EVENT)
  {
    std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
    if (!ctrl_exit_event)
    {
      ctrl_exit_until = std::chrono::steady_clock::now(); // Exit immediatelly
      ctrl_exit_event = true;

      if(command_executor)
        command_executor->Interrupt();

      if (record_command)
        record_command->Interrupt();

      InterruptStdIn();
    }
    return TRUE;
  }
  else if (dwType == CTRL_CLOSE_EVENT)
  {
    std::unique_lock<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
    if (!ctrl_exit_event)
    {
      ctrl_exit_event = true;
      ctrl_exit_until = std::chrono::steady_clock::now() + std::chrono::seconds(3); // Give it 3 seconds to finish, kill it otherwise

      if(command_executor)
        command_executor->Interrupt();

      if (record_command)
        record_command->Interrupt();

      InterruptStdIn();

      ecal_rec_exit_cv_.wait(ecal_rec_exit_lock);
    }

    return TRUE;
  }

  return FALSE;
}

void InterruptStdIn()
{
  // Virtually press Return, so the cin reading loop will exit

  DWORD dwTmp;
  INPUT_RECORD ir[2];

  ir[0].EventType                        = KEY_EVENT;
  ir[0].Event.KeyEvent.bKeyDown          = TRUE;
  ir[0].Event.KeyEvent.dwControlKeyState = 0;
  ir[0].Event.KeyEvent.uChar.UnicodeChar = VK_RETURN;
  ir[0].Event.KeyEvent.wRepeatCount      = 1;
  ir[0].Event.KeyEvent.wVirtualKeyCode   = VK_RETURN;
  ir[0].Event.KeyEvent.wVirtualScanCode  = static_cast<WORD>(MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC));

  ir[1]                                  = ir[0];
  ir[1].Event.KeyEvent.bKeyDown          = FALSE;

  HANDLE std_in_handle = GetStdHandle(STD_INPUT_HANDLE);

  if (std_in_handle != INVALID_HANDLE_VALUE)
    WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), ir, 2, &dwTmp);
}


#else // WIN32

void SignalHandler(int s)
{
  // TODO: On rare occations, eCAL rec hangs when hitting ctrl+c. Remove the debug output, once that bug is found.
  //       Note, just in case the error vanishes: I moved the mutex lock into the SIGINT/SIGTERM if statements, so the signal handler will only aquire the lock when needed. Previously, the handler aquired the lock before checking if it even needed it.

  std::cerr << "Signal Handler: caught signal " + std::to_string(s) + "\n";

  if (s == SIGINT)
  {
    std::cerr << "Signal Handler: signal is SIGINT\n";

    std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
    std::cerr << "Signal Handler: Aquired exit_mutex\n";

    if (!ctrl_exit_event)
    {
      std::cerr << "Signal Handler: Start interrupting running eCAL Rec\n";

      ctrl_exit_event = true;

      if(command_executor)
      {
        std::cerr << "Signal Handler: Interrupting command_executor...\n";
        command_executor->Interrupt();
      }

      if (record_command)
      {
        std::cerr << "Signal Handler: Interrupting record_command...\n";
        record_command->Interrupt();
      }

      std::cerr << "Signal Handler: Interrupting stdin...\n";
      InterruptStdIn();
    }
    else
    {
      std::cerr << "Signal Handler: Application has already been interrupted. Doing nothing.\n";
    }
  }
  else if (s == SIGTERM)
  {
    std::cerr << "Signal Handler: Signal is SIGTERM\n";

    std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
    std::cerr << "Signal Handler: Aquired exit_mutex\n";

    if (!ctrl_exit_event)
    {
      ctrl_exit_event = true;
      ctrl_exit_until = std::chrono::steady_clock::now() + std::chrono::seconds(3); // Give it 3 seconds to finish, kill it otherwise    

      if(command_executor)
      {
        std::cerr << "Signal Handler: Interrupting command_executor...\n";
        command_executor->Interrupt();
      }

      if (record_command)
      {
        std::cerr << "Signal Handler: Interrupting record_command...\n";
        record_command->Interrupt();
      }

      std::cerr << "Signal Handler: Interrupting stdin...\n";
      InterruptStdIn();
    }
    else
    {
      std::cerr << "Signal Handler: Application has already been interrupted. Doing nothing.\n";
    }
  }
  else
  {
    std::cerr << "Signal Handler: Unknown singal. Doing nothing.\n";
  }

  std::cerr << "Signal Handler: Finished!\n";
}

void InterruptStdIn()
{
  ::fclose(stdin);
}

#endif //WIN32

