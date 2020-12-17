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
 * @brief  eCAL rpc interface
**/

#include <ecal/ecal.h>
//#include <ecal/ecal_apps.h>
#include <rpc/ecal_rpc.h>

#include <atomic>
#include <string>
#include <thread>
#include <mutex>
#include <iostream>

#include "ecal_rpc_globals.h"
#include "ecal_rpc_def.h"
#include "ecal_rpc_global_accessors.h"
#include "ecal/pb/rpcservice/service.pb.h"

////////////////////////////////////////////////////////
// local events
////////////////////////////////////////////////////////
namespace eCAL
{
  // const EventHandleT& RecorderStartEvent()
  //{
  //  static EventHandleT evt;
  //  if (!gEventIsValid(evt))
  //  {
  //    gOpenEvent(&evt, EVENT_RECORDER_START);
  //  }
  //  return(evt);
  //}

  // const EventHandleT& RecorderCloseEvent()
  //{
  //  static EventHandleT evt;
  //  if (!gEventIsValid(evt))
  //  {
  //    gOpenEvent(&evt, EVENT_RECORDER_CLOSE);
  //  }
  //  return(evt);
  //}

  // const EventHandleT& CanRecorderStartEvent()
  //{
  //  static EventHandleT evt;
  //  if (!gEventIsValid(evt))
  //  {
  //    gOpenEvent(&evt, EVENT_RECORDER_CAN_START);
  //  }
  //  return(evt);
  //}

  // const EventHandleT& CanRecorderCloseEvent()
  //{
  //  static EventHandleT evt;
  //  if (!gEventIsValid(evt))
  //  {
  //    gOpenEvent(&evt, EVENT_RECORDER_CAN_CLOSE);
  //  }
  //  return(evt);
  //}
}

bool StartEcalApp(const char* app_name_, const char* app_args_)
{
  // build application start command
#ifdef ECAL_OS_WINDOWS
  std::string app_start_path = eCAL::Util::GeteCALHomePath();
  app_start_path += ECAL_BIN_PATH_WINDOWS;
  app_start_path += "\\";
  const char* ext = ".exe";
#endif

#ifdef ECAL_OS_LINUX
  // TODO : Should not be hard coded ! Maybe we put it in the config ?
  std::string app_start_path;
  const char* ext = "";
#endif

  std::string app_start_cmd;
  app_start_cmd += app_name_;
  app_start_cmd += ext;

  app_start_cmd = app_start_path + app_start_cmd;
  if (app_start_cmd.empty()) return(false);

  // call the command
#ifdef ECAL_OS_WINDOWS
  app_start_cmd = "\"" + app_start_cmd + "\"";
#endif
  int pid = eCAL::Process::StartProcess(app_start_cmd.c_str(), app_args_, app_start_path.c_str(), true, proc_smode_normal, false);
  return(pid > 0);
}

namespace eCAL
{
  namespace RPC
  {
    /////////////////////////////////////////////////////////////////////////////
    // ecal rpc service response callback
    /////////////////////////////////////////////////////////////////////////////
    void CheckResult(const struct eCAL::SServiceInfo& service_info_, const std::string& response_, std::vector<SRPCServiceResponse>* server_state_vec_)
    {
      switch (service_info_.call_state)
      {
        // service successful executed
      case call_state_executed:
      {
        eCAL::pb::rpcservice::ProcessResponse response;
        response.ParseFromString(response_);
        SRPCServiceResponse host_pid_state;
        host_pid_state.host_name = service_info_.host_name;
        host_pid_state.pid = response.pid();
        host_pid_state.success = response.result() == eCAL::pb::rpcservice::eServiceResult::success;
        server_state_vec_->push_back(host_pid_state);
      }
      break;
      // service execution failed
      case call_state_failed:
      {
        std::cout << "Received error " << service_info_.service_name << " / " << service_info_.method_name << " : " << service_info_.error_msg << " from host " << service_info_.host_name << std::endl;
      }
      break;
      default:
        break;
      }
    }
    void OnServerResponse(const struct eCAL::SServiceInfo& service_info_, const std::string& response_, std::vector<SRPCServiceResponse>* server_state_vec_)
    {
      CheckResult(service_info_, response_, server_state_vec_);
    }

    /**
     * @brief Initialize eCAL RPC API.
     *
     * @return Zero if succeeded, 1 if already initialized, -1 if failed.
    **/
    int Initialize()
    {
      std::lock_guard<std::mutex>rpc_lock(g_rpc_mutex);
      // first call ?
      if (g_globals_ctx == nullptr)
      {
        // instanciate globals
        g_globals_ctx = new CGlobals;
      }

      // reference counting
      g_globals_ctx_ref_cnt++;

      // initialize
      return g_globals()->Initialize();
    }

    /**
     * @brief Finalize eCAL RPC API.
     *
     * @return Zero if succeeded, 1 if already finalized, -1 if failed.
    **/
    int Finalize()
    {
      std::lock_guard<std::mutex>rpc_lock(g_rpc_mutex);
      // nothing to finalize ?
      if (g_globals_ctx == nullptr) return 1;

      // check reference counter
      g_globals_ctx_ref_cnt--;
      if (g_globals_ctx_ref_cnt > 0) return 0;

      // finalize
      int ret = g_globals()->Finalize();
      delete g_globals_ctx;
      g_globals_ctx = nullptr;

      return(ret);
    }

    /**
     * @brief Set rpc service response time out.
     *
     * @param timeout_  Maximum time to wait for rpc service response (in milliseconds, -1 means infinite).
     *
     * @return Zero if succeeded.
    **/
    std::atomic<int> g_service_timeout(2000);  // default == 2 sec
    int SetTimeout(const int timeout_)
    {
      g_service_timeout = timeout_;
      return(0);
    }

    /////////////////////////////////////////////////////////////////////////////
    // call ecal service
    /////////////////////////////////////////////////////////////////////////////
    std::vector<SRPCServiceResponse> CallService(const std::string& host_name_, const std::string& service_name_, const std::string& service_request_)
    {
      static std::vector<SRPCServiceResponse> server_state_vec;
      server_state_vec.clear();

      std::lock_guard<std::mutex>rpc_lock(g_rpc_mutex);

      if (!g_rpc()) return(server_state_vec);
      g_rpc()->SetHostName(host_name_);
      bool broadcast = host_name_.empty() || (host_name_ == "*");
      if (broadcast)
      {
        g_rpc()->AddResponseCallback(std::bind(OnServerResponse, std::placeholders::_1, std::placeholders::_2, &server_state_vec));
        g_rpc()->Call(service_name_, service_request_);
        std::this_thread::sleep_for(std::chrono::milliseconds(g_service_timeout));
      }
      else
      {
        std::string response;
        SServiceInfo service_info;
        g_rpc()->RemResponseCallback();
        if (g_rpc()->Call(host_name_, service_name_, service_request_, service_info, response))
        {
          CheckResult(service_info, response, &server_state_vec);
        }
      }
      return(server_state_vec);
    }

    /**
     * @brief Start process on defined host.
     *
     * @param host_name_      Host name ("" == on all hosts).
     * @param process_name_   Process name.
     * @param process_dir_    Process working directory.
     * @param process_args_   Process arguments.
     * @param create_console_ Create own console window (Windows only).
     * @param process_mode_   Start the process normal, hidden, minimized, maximized (Windows only).
     *
     * @return  Vector of process id's if successful, otherwise zero.
     *          (Dll interface returns number of started processes)
    **/
    std::vector<SRPCServiceResponse> StartProcess(const std::string& host_name_, const std::string& process_name_, const std::string& process_dir_, const std::string& process_args_, bool create_console_, eCAL_Process_eStartMode process_mode_)
    {
      eCAL::pb::rpcservice::StartProcessRequest request;
      request.set_process_name(process_name_);
      request.set_process_dir(process_dir_);
      request.set_process_args(process_args_);
      request.set_create_console(create_console_);
      request.set_start_mode(eCAL::pb::rpcservice::eStartProcessMode(process_mode_));
      return(CallService(host_name_, "StartProcess", request.SerializeAsString()));
    }

    /**
     * @brief Stop process on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     * @param process_name_ Process name.
     *
     * @return  Vector of process id's if successful, otherwise zero.
     *          (Dll interface returns number of stopped processes)
    **/
    std::vector<SRPCServiceResponse> StopProcess(const std::string& host_name_, const std::string& process_name_)
    {
      eCAL::pb::rpcservice::StopProcessNameRequest request;
      request.set_process_name(process_name_);
      return(CallService(host_name_, "StopProcessName", request.SerializeAsString()));
    }

    /**
     * @brief Stop process on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     * @param process_id_   Process id.
     *
     * @return  Vector of process id's if successful, otherwise zero.
     *          (Dll interface returns number of stopped processes)
    **/
    std::vector<SRPCServiceResponse> StopProcess(const std::string& host_name_, const int process_id_)
    {
      eCAL::pb::rpcservice::StopProcessIDRequest request;
      request.set_process_id(process_id_);
      return(CallService(host_name_, "StopProcessID", request.SerializeAsString()));
    }

    /**
     * @brief Shutdown process on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     * @param process_name_ Process name.
     *
     * @return  Vector of process id's if successful, otherwise zero.
     *          (Dll interface returns number of closed processes)
    **/
    std::vector<SRPCServiceResponse> ShutdownProcess(const std::string& host_name_, const std::string& process_name_)
    {
      eCAL::pb::rpcservice::ShutdownProcessNameRequest request;
      request.set_process_name(process_name_);
      return(CallService(host_name_, "ShutdownProcessName", request.SerializeAsString()));
    }

    /**
     * @brief Shutdown process on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     * @param process_id_   Process id.
     *
     * @return  Vector of process id's if successful, otherwise zero.
     *          (Dll interface returns number of closed processes)
    **/
    std::vector<SRPCServiceResponse> ShutdownProcess(const std::string& host_name_, const int process_id_)
    {
      eCAL::pb::rpcservice::ShutdownProcessIDRequest request;
      request.set_process_id(process_id_);
      return(CallService(host_name_, "ShutdownProcessID", request.SerializeAsString()));
    }

    /**
     * @brief Shutdown eCAL user processes on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     *
     * @return  Vector of host names on which eCAL user processes were shutdown.
     *          (Dll interface returns number of closed processes)
    **/
    std::vector<SRPCServiceResponse> ShutdownProcesses(const std::string& host_name_)
    {
      eCAL::pb::rpcservice::ShutdownProcessesRequest request;
      return(CallService(host_name_, "ShutdownProcesses", request.SerializeAsString()));
    }

    /**
     * @brief Shutdown eCAL core on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     *
     * @return  Vector of host names on which eCAL core was shutdown.
     *          (Dll interface returns number of closed processes)
    **/
    std::vector<SRPCServiceResponse> ShutdownCore(const std::string& host_name_)
    {
      eCAL::pb::rpcservice::ShutdownCoreRequest request;
      return(CallService(host_name_, "ShutdownCore", request.SerializeAsString()));
    }

    /**
     * @brief Shutdown eCAL rpc service on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     *
     * @return  Vector of host names on which eCAL rpc service was shutdown.
     *          (Dll interface returns number of closed processes)
    **/
    std::vector<SRPCServiceResponse> ShutdownRPCService(const std::string& host_name_)
    {
      eCAL::pb::rpcservice::ShutdownRPCServiceRequest request;
      return(CallService(host_name_, "ShutdownRPCService", request.SerializeAsString()));
    }

    ///**
    // * @brief Open eCAL recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // * @param args_         Recorder arguments.
    // *
    // * @return  Zero if succeeded.
    //**/
    //int RecorderOpen(const std::string& host_name_, const std::string& args_)
    //{
    //  eCAL::pb::rpcservice::RecorderOpenRequest request;
    //  request.set_parameters(args_);
    //  std::vector<SRPCServiceResponse> host_state_vec = CallService(host_name_, "RecorderOpen", request.SerializeAsString());
    //  if (host_state_vec.empty()) return(-1);
    //  return(0);
    //}

    ///**
    // * @brief Close eCAL recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // *
    // * @return  Zero if succeeded.
    //**/
    //int RecorderClose(const std::string& host_name_)
    //{
    //  eCAL::pb::rpcservice::RecorderCloseRequest request;
    //  std::vector<SRPCServiceResponse> host_state_vec = CallService(host_name_, "RecorderClose", request.SerializeAsString());
    //  if (host_state_vec.empty()) return(-1);
    //  return(0);
    //}

    ///**
    // * @brief Start eCAL recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // *
    // * @return  Zero if succeeded.
    //**/
    //int RecorderStart(const std::string& host_name_)
    //{
    //  eCAL::pb::rpcservice::RecorderCloseRequest request;
    //  std::vector<SRPCServiceResponse> host_state_vec = CallService(host_name_, "RecorderStart", request.SerializeAsString());
    //  if (host_state_vec.empty()) return(-1);
    //  return(0);
    //}

    ///**
    // * @brief Open eCAL CAN recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // * @param args_         Recorder arguments.
    // *
    // * @return  Zero if succeeded.
    //**/
    //int CanRecorderOpen(const std::string& host_name_, const std::string& args_)
    //{
    //  eCAL::pb::rpcservice::CanRecorderOpenRequest request;
    //  request.set_parameters(args_);
    //  std::vector<SRPCServiceResponse> host_state_vec = CallService(host_name_, "CanRecorderOpen", request.SerializeAsString());
    //  if (host_state_vec.empty()) return(-1);
    //  return(0);
    //}

    ///**
    // * @brief Close eCAL CAN recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // *
    // * @return  Zero if succeeded.
    //**/
    //int CanRecorderClose(const std::string& host_name_)
    //{
    //  eCAL::pb::rpcservice::CanRecorderCloseRequest request;
    //  std::vector<SRPCServiceResponse> host_state_vec = CallService(host_name_, "CanRecorderClose", request.SerializeAsString());
    //  if (host_state_vec.empty()) return(-1);
    //  return(0);
    //}

    ///**
    // * @brief Start eCAL CAN recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // *
    // * @return  Zero if succeeded.
    //**/
    //int CanRecorderStart(const std::string& host_name_)
    //{
    //  eCAL::pb::rpcservice::CanRecorderCloseRequest request;
    //  std::vector<SRPCServiceResponse> host_state_vec = CallService(host_name_, "CanRecorderStart", request.SerializeAsString());
    //  if (host_state_vec.empty()) return(-1);
    //  return(0);
    //}

    ///**
    // * @brief Open eCAL recorder application.
    // *
    // * @param args_  Command line arguments.
    //**/
    //bool RecorderOpenL(const char* args_)
    //{
    //  // open ecal recorder
    //  return (StartEcalApp(eCAL::Apps::REC, args_));
    //}

    ///**
    // * @brief Close eCAL recorder application.
    //**/
    //bool RecorderCloseL()
    //{
    //  // send close event
    //  gSetEvent(RecorderCloseEvent());

    //  // return true
    //  return(true);
    //}

    ///**
    // * @brief Start eCALrec recording.
    //**/
    //bool RecorderStartL()
    //{
    //  // start recording
    //  gSetEvent(RecorderStartEvent());

    //  // return true
    //  return(true);
    //}

    ///**
    // * @brief Open eCAL CAN recorder application.
    // *
    // * @param args_  Command line arguments.
    //**/
    //bool CanRecorderOpenL(const char* args_)
    //{
    //  // open ecal recorder
    //  return (StartEcalApp(eCAL::Apps::CAN_REC, args_));
    //}

    ///**
    // * @brief Close eCAL CAN recorder application.
    //**/
    //bool CanRecorderCloseL()
    //{
    //  // send close event
    //  gSetEvent(CanRecorderCloseEvent());

    //  // return true
    //  return(true);
    //}

    ///**
    // * @brief Start eCALCanRec recording.
    //**/
    //bool CanRecorderStartL()
    //{
    //  // start recording
    //  gSetEvent(CanRecorderStartEvent());

    //  // return true
    //  return(true);
    //}

    ///**
    //* @brief Open eCALRPCService application.
    //**/
    //bool RPCServiceOpenL()
    //{
    //  // open eCALRPCService
    //  return(StartEcalApp(eCAL::Apps::RPCSERVICE, ""));
    //}
  }
}
