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

#include <rec_client_core/ecal_rec.h>
#include <rec_client_core/ecal_rec_defs.h>

#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/server.h>

#include <ecal_utils/string.h>

#include <tclap/CmdLine.h>

#include "ecal_rec_service.h"



#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <signal.h>

/** Win32 Console Handler (CTRL+C, Close Window) */
BOOL WINAPI ConsoleHandler(DWORD);

#else // WIN32

#include <signal.h>

/** POSIX signal handler */
void SignalHandler(int s);

#endif // WIN32

/** Global eCAL Rec instance */
std::shared_ptr<eCAL::rec::EcalRec> ecal_rec;

bool IsRecorderBusy();
void UpdateEcalState();

std::mutex                            ecal_rec_exit_mutex_;
std::condition_variable               ecal_rec_exit_cv_;
std::chrono::steady_clock::time_point ctrl_exit_until(std::chrono::steady_clock::duration::max());
bool                                  ctrl_exit_event(false);



int main(int argc, char** argv)
{
  TCLAP::CmdLine cmd("eCAL Recorder", ' ', ECAL_REC_VERSION_STRING);
  
  assert(cmd.getArgList().size() == 3);
  auto arg_it = cmd.getArgList().begin();

  //TCLAP::Arg& ignore_rest_arg = **arg_it;
  ++arg_it;
  //TCLAP::Arg& version_arg     = **arg_it;
  ++arg_it;
  //TCLAP::Arg& help_arg        = **arg_it;

  // Settings args
  TCLAP::ValueArg<double>       pre_buffer_arg     ("b", "pre-buffer",      "Pre-buffer data for some seconds",                                                                                                                                       false, -1.0, "seconds");
  TCLAP::ValueArg<std::string>  blacklist_arg      ("",  "blacklist",       "Record all topics except the listed ones (Comma separated list, e.g.: \"Topic1,Topic2\")",                                                                               false, "", "list");
  TCLAP::ValueArg<std::string>  whitelist_arg      ("",  "whitelist",       "Only record these topics (Comma separated list, e.g.: \"Topic1,Topic2\")",                                                                                               false, "", "list");
  TCLAP::ValueArg<std::string>  host_filter_arg    ("f", "hosts",           "Only record a topic when it is published by any of these hosts (Comma-separated list, e.g.: \"Computer1,Computer2\")",                                                   false, "", "list");
  TCLAP::ValueArg<std::string>  addons_arg         ("",  "addons",          "Enables the given recorder addons (Comma-separated list, e.g.: \"de.conti.addon1,de.conti.addon2\"",                                                                      false, "", "list");

  // Command args
  TCLAP::SwitchArg              record_arg         ("r", "record",          "Directly start a recording. Make sure to set all necessary parameters.",                                                                                                 false);
  TCLAP::SwitchArg              connect_to_ecal_arg("",  "connect-to-ecal", "Directly connect to eCAL.",                                                                                                                                              false);

  // Job-config args
  TCLAP::ValueArg<std::string>  meas_root_dir_arg  ("d", "meas-root-dir",   "Root dir used for recording when --" + record_arg.getName() + " is set.",                                                                                                false, "", "path");
  TCLAP::ValueArg<std::string>  meas_name_arg      ("n", "meas-name",       "Name of the measurement, when --" + record_arg.getName() + " is set. This will create a folder in the directory provided by --" + meas_root_dir_arg.getName() + ".",     false, "", "directory");
  TCLAP::ValueArg<unsigned int> max_file_size_arg  ("",  "max-file-size",   "Maximum file size of the recording files, when --" + record_arg.getName() + " is set.",                                                                                  false, 100, "megabytes");
  TCLAP::ValueArg<std::string>  description_arg    ("",  "description",     "Description stored in the measurement folder, when --" + record_arg.getName() + " is set.",                                                                              false, "", "string");

  // Various args
  TCLAP::SwitchArg              list_addons_arg    ("",  "list-addons",     "Lists addons and exit.",                                                                                                                                                  false);

  std::vector<TCLAP::Arg*> arg_vector =
  {
    &pre_buffer_arg,
    &blacklist_arg,
    &whitelist_arg,
    &host_filter_arg,
    &addons_arg,
    &record_arg,
    &connect_to_ecal_arg,
    &meas_root_dir_arg,
    &meas_name_arg,
    &max_file_size_arg,
    &description_arg,
    &list_addons_arg,
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

  // TODO: Check the validity of all arguments
  ecal_rec = std::make_shared<eCAL::rec::EcalRec>();

  //////////////////////////////////
  // List-addons
  //////////////////////////////////
  if (list_addons_arg.isSet())
  {
    auto status = ecal_rec->GetRecorderStatus();

    std::cout << "Found " + std::to_string(status.addon_statuses_.size()) + " addon" + (status.addon_statuses_.size() == 1 ? "" : "s") << std::endl;
    for (const auto& addon_status : status.addon_statuses_)
    {
      std::cout << addon_status.addon_id_ << " (" << addon_status.name_ << ") " << addon_status.addon_executable_path_ << std::endl;
    }
    ecal_rec = nullptr;
    return 0;
  }

  //////////////////////////////////
  // Pre-buffer
  //////////////////////////////////
  if (pre_buffer_arg.isSet())
  {
    ecal_rec->SetPreBufferingEnabled(true);
    auto buffer_length = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(pre_buffer_arg.getValue()));
    ecal_rec->SetMaxPreBufferLength(buffer_length);
  }

  //////////////////////////////////
  // Blacklist / whitelist
  //////////////////////////////////
  if (blacklist_arg.isSet())
  {
    // Split given list
    std::list<std::string> blacklist_list;
    EcalUtils::String::Split(blacklist_arg.getValue(), ",", blacklist_list);

    // Trim whitespaces and copy into set
    std::set<std::string> blacklist_set;
    for (const auto& topic : blacklist_list)
    {
      blacklist_set.emplace(EcalUtils::String::Trim(topic));
    }

    ecal_rec->SetRecordMode(eCAL::rec::RecordMode::Blacklist, blacklist_set);
  }
  else if (whitelist_arg.isSet())
  {
    // Split given list
    std::list<std::string> whitelist_list;
    EcalUtils::String::Split(whitelist_arg.getValue(), ",", whitelist_list);

    // Trim whitespaces and copy into set
    std::set<std::string> whitelist_set;
    for (const auto& topic : whitelist_list)
    {
      whitelist_set.emplace(EcalUtils::String::Trim(topic));
    }

    ecal_rec->SetRecordMode(eCAL::rec::RecordMode::Whitelist, whitelist_set);
  }

  //////////////////////////////////
  // Host filter
  //////////////////////////////////
  if (host_filter_arg.isSet())
  {
    // Split given list
    std::list<std::string> host_filter_list;
    EcalUtils::String::Split(host_filter_arg.getValue(), ",", host_filter_list);

    // Trim whitespaces and copy into set
    std::set<std::string> host_filter_set;
    for (const auto& host : host_filter_list)
    {
      host_filter_set.emplace(EcalUtils::String::Trim(host));
    }

    ecal_rec->SetHostFilter(host_filter_set);
  }

  //////////////////////////////////
  // Addons
  //////////////////////////////////
  if (addons_arg.isSet())
  {
    // Split given list
    std::list<std::string> addons_list;
    EcalUtils::String::Split(addons_arg.getValue(), ",", addons_list);

    // Trim whitespaces and copy into set
    std::set<std::string> addons_set;
    for (const auto& addon : addons_list)
    {
      addons_set.emplace(EcalUtils::String::Trim(addon));
    }

    ecal_rec->SetEnabledAddons(addons_set);
  }

  //////////////////////////////////
  // Connect to eCAL Command
  //////////////////////////////////
  if (connect_to_ecal_arg.isSet() && connect_to_ecal_arg.getValue())
  {
    ecal_rec->ConnectToEcal();
  }

  //////////////////////////////////
  // Record command
  //////////////////////////////////
  if (record_arg.isSet() && record_arg.getValue())
  {
    eCAL::rec::JobConfig job_config;

    //////////////////////////////////
    // meas_root_dir
    //////////////////////////////////
    if (meas_root_dir_arg.isSet())
    {
      job_config.SetMeasRootDir(meas_root_dir_arg.getValue());
    }
    //////////////////////////////////
    // meas_name
    //////////////////////////////////
    if (meas_name_arg.isSet())
    {
      job_config.SetMeasName(meas_name_arg.getValue());
    }
    //////////////////////////////////
    // max_file_size
    //////////////////////////////////
    if (max_file_size_arg.isSet())
    {
      job_config.SetMaxFileSize(max_file_size_arg.getValue());
    }
    //////////////////////////////////
    // description
    //////////////////////////////////
    if (description_arg.isSet())
    {
      job_config.SetDescription(description_arg.getValue());
    }

    ecal_rec->ConnectToEcal();
    ecal_rec->StartRecording(job_config);
  }

  std::string rec_name = std::string("eCAL Recorder Client");
  std::string rec_version = std::string("Version: ") + ECAL_REC_VERSION_STRING;
  std::string ecal_version = std::string("eCAL ") + ECAL_VERSION + " (" + ECAL_DATE + ")";
  EcalUtils::String::CenterString(rec_name, ' ', 73);
  EcalUtils::String::CenterString(rec_version, ' ', 73);
  EcalUtils::String::CenterString(ecal_version, ' ', 73);

  std::stringstream header_ss;
  header_ss << "===============================================================================" << std::endl;
  header_ss << "===" << rec_name << "==="                                                        << std::endl;
  header_ss << "===" << rec_version << "==="                                                     << std::endl;
  header_ss << "===" << ecal_version << "==="                                                    << std::endl;
  header_ss << "===============================================================================" << std::endl;
  header_ss                                                                                      << std::endl;

  // Signal handling for Ctrl+C
#ifdef WIN32

  std::string attention_string1 = "!! Attention !!";
  std::string attention_string2 = "Closing the console with the [X] button may lead to incomplete measurements";
  EcalUtils::String::CenterString(attention_string1, ' ', 73);
  EcalUtils::String::CenterString(attention_string2, ' ', 73);

  header_ss << std::endl;
  header_ss << attention_string1 << std::endl;
  header_ss << attention_string2 << std::endl;

  header_ss << std::endl;

  if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE))
  {
    header_ss << "Press Ctrl+C to exit" << std::endl;
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
    header_ss << "Press Ctrl+C to exit" << std::endl;
  }
  else
  {
    std::cerr << "Unable to set signal handler: " << strerror(errno) << std::endl;    
  }
     
#endif
  
  header_ss << std::endl;
  header_ss << "-------------------------------------------------------------------------------" << std::endl;
  header_ss << std::endl;

  std::cout << header_ss.str();

  std::shared_ptr<eCAL::pb::rec_client::EcalRecClientService> rec_service_(new EcalRecService(ecal_rec));
  eCAL::protobuf::CServiceServer<eCAL::pb::rec_client::EcalRecClientService> rec_service_server_(rec_service_);

  while (eCAL::Ok())
  {
    {
      std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
      if (ctrl_exit_event || static_cast<EcalRecService*>(rec_service_.get())->IsExitCommandSet())
        break;
    }

    UpdateEcalState();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Shutdown service
  rec_service_server_.Destroy();
  rec_service_ = nullptr;

  // Stop recording
  ecal_rec->StopRecording();

  // Wait for all writers
  for (;;)
  {
    if (!IsRecorderBusy())
    {
      break; // Exit because the recorder is finished
    }
    else
    {
      {
        std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
        if (std::chrono::steady_clock::now() > ctrl_exit_until)
          break; // Exit anyways
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }

  // Manually destroy ecal_rec, as the logging sinks might otherwise be closed before ecal_rec is destroyed, which will cause the logger to crash
  ecal_rec = nullptr;

  ecal_rec_exit_cv_.notify_all();

  return 0;
}

bool IsRecorderBusy()
{
  bool any_job_running = false;
  auto recorder_status = ecal_rec->GetRecorderStatus();

  for (const auto& job_status : recorder_status.job_statuses_)
  {
    if ((job_status.state_ == eCAL::rec::JobState::Recording)
      || (job_status.state_ == eCAL::rec::JobState::Flushing)
      || (job_status.state_ == eCAL::rec::JobState::Uploading))
    {
      any_job_running = true;
      break;
    }
  }

  return any_job_running;
}

void UpdateEcalState()
{
  auto rec_status = ecal_rec->GetRecorderStatus();

  if (!rec_status.initialized_)
  {
    eCAL::Process::SetState(eCAL_Process_eSeverity::proc_sev_warning, eCAL_Process_eSeverity_Level::proc_sev_level1
      , "Not initialized");
  }
  else
  {
    const eCAL::rec::JobStatus*            recording_job_status_(nullptr);
    std::list<const eCAL::rec::JobStatus*> flushing_jobs_;
    std::list<const eCAL::rec::JobStatus*> uploading_jobs_;

    // Collect The currently recording job, the flushing jobs and the jobs that are being uploaded
    for (const auto& job_status : rec_status.job_statuses_)
    {
      if (job_status.state_ == eCAL::rec::JobState::Recording)
      {
        recording_job_status_ = &job_status;
      }
      else if (job_status.state_ == eCAL::rec::JobState::Uploading)
      {
        uploading_jobs_.push_back(&job_status);
      }
      else
      {
        if (job_status.state_ == eCAL::rec::JobState::Flushing)
        {
          flushing_jobs_.push_back(&job_status);
        }
        else
        {
          for (const auto& addon_job_status : job_status.rec_addon_statuses_)
          {
            if (addon_job_status.second.state_ == eCAL::rec::RecAddonJobStatus::State::Flushing)
            {
              flushing_jobs_.push_back(&job_status);
              break;
            }
          }
        }
      }
    }

    eCAL_Process_eSeverity severity(eCAL_Process_eSeverity::proc_sev_healthy);
    std::string state_string;

    // General error
    if (!rec_status.info_.first)
    {
      severity = eCAL_Process_eSeverity::proc_sev_critical;
      state_string = "[" + rec_status.info_.second + "] ";
    }

    // Recording / Initialized
    if (recording_job_status_ == nullptr)
    {
      state_string += "Intialized ";
    }
    else
    {
      state_string += "Recording ";
    }

    if (rec_status.subscribed_topics_.empty())
      severity = std::max(severity, eCAL_Process_eSeverity::proc_sev_warning);

    state_string += "(" + std::to_string(rec_status.subscribed_topics_.size()) + " Topics) ";

    // Number of flushing jobs
    if (!flushing_jobs_.empty())
    {
      state_string += ", " + std::to_string(flushing_jobs_.size()) + " job" + (flushing_jobs_.size() > 1 ? "s" : "") + " flushing";
    }

    // Number of jobs being uploaded
    if (!uploading_jobs_.empty())
    {
      state_string += ", " + std::to_string(flushing_jobs_.size()) + " job" + (flushing_jobs_.size() > 1 ? "s" : "") + " uploading";
    }

    eCAL::Process::SetState(severity, eCAL_Process_eSeverity_Level::proc_sev_level1, state_string.c_str());
  }
}

#ifdef WIN32
BOOL WINAPI ConsoleHandler(DWORD dwType)
{
  if (dwType == CTRL_C_EVENT)
  {
    std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
    ctrl_exit_event = true;
    return TRUE;
  }
  else if (dwType == CTRL_BREAK_EVENT)
  {
    std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);

    ctrl_exit_until = std::chrono::steady_clock::now(); // Exit immediatelly
    ctrl_exit_event = true;

    return TRUE;
  }
  else if (dwType == CTRL_CLOSE_EVENT)
  {
    std::unique_lock<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);

    ctrl_exit_event = true;
    ctrl_exit_until = std::chrono::steady_clock::now() + std::chrono::seconds(3); // Give it 3 seconds to finish, kill it otherwise

    ecal_rec_exit_cv_.wait(ecal_rec_exit_lock);

    return TRUE;
  }

  return FALSE;
}
#else // WIN32

void SignalHandler(int s)
{
  std::lock_guard<decltype(ecal_rec_exit_mutex_)> ecal_rec_exit_lock(ecal_rec_exit_mutex_);
  
  if (s == SIGINT)
  {
    ctrl_exit_event = true;
  }
  else if (s == SIGTERM)
  {
    ctrl_exit_event = true;
    ctrl_exit_until = std::chrono::steady_clock::now() + std::chrono::seconds(3); // Give it 3 seconds to finish, kill it otherwise    
  }
}

#endif //WIN32
