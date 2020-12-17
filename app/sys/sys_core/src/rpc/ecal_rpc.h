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
 * @file   ecal_rpc.h
 * @brief  eCAL rpc interface
**/

#pragma once

#include <string>
#include <vector>

#include <ecal/ecal_os.h>
#include <ecal/ecal_process_mode.h>

namespace eCAL
{
  namespace RPC
  {
    /**
     * @brief Initialize eCAL RPC API.
     *
     * @return Zero if succeeded, 1 if already initialized, -1 if failed.
    **/
    int Initialize();

    /**
     * @brief Finalize eCAL RPC API.
     *
     * @return Zero if succeeded, 1 if already finalized, -1 if failed.
    **/
    int Finalize();

    /**
     * @brief Set rpc service response time out.
     *
     * @param timeout_  Maximum time to wait for rpc service response (in milliseconds, -1 means infinite).
     *
     * @return Zero if succeeded.
    **/
    int SetTimeout(int timeout_);

#ifndef ECAL_C_DLL
    /**
     * @brief Common rpc service response info struct.
    **/
    struct SRPCServiceResponse
    {
      SRPCServiceResponse() :
        pid(0),
        success(0)
      {
      }
      std::string  host_name;  //!< host name
      int          pid;        //!< pid of started/stopped process
      int          success;    //!< zero if failed
    };

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
     std::vector<SRPCServiceResponse> StartProcess(const std::string& host_name_, const std::string& process_name_, const std::string& process_dir_, const std::string& process_args_, bool create_console_, eCAL_Process_eStartMode process_mode_);

    /**
     * @brief Stop process on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     * @param process_name_ Process name.
     *
     * @return  Vector of process id's if successful, otherwise zero.
     *          (Dll interface returns number of stopped processes)
    **/
     std::vector<SRPCServiceResponse> StopProcess(const std::string& host_name_, const std::string& process_name_);

    /**
     * @brief Stop process on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     * @param process_id_   Process id.
     *
     * @return  Vector of process id's if successful, otherwise zero.
     *          (Dll interface returns number of stopped processes)
    **/
     std::vector<SRPCServiceResponse> StopProcess(const std::string& host_name_, int process_id_);

    /**
     * @brief Shutdown process on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     * @param process_name_ Process name.
     *
     * @return  Vector of process id's if successful, otherwise zero.
     *          (Dll interface returns number of closed processes)
    **/
     std::vector<SRPCServiceResponse> ShutdownProcess(const std::string& host_name_, const std::string& process_name_);

    /**
     * @brief Shutdown process on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     * @param process_id_   Process id.
     *
     * @return  Vector of process id's if successful, otherwise zero.
     *          (Dll interface returns number of closed processes)
    **/
     std::vector<SRPCServiceResponse> ShutdownProcess(const std::string& host_name_, int process_id_);

    /**
     * @brief Shutdown eCAL user processes on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     *
     * @return  Vector of host names on which eCAL user processes were shutdown.
     *          (Dll interface returns number of closed processes)
    **/
     std::vector<SRPCServiceResponse> ShutdownProcesses(const std::string& host_name_);

    /**
     * @brief Shutdown eCAL core on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     *
     * @return  Vector of host names on which eCAL core was shutdown.
     *          (Dll interface returns number of closed processes)
    **/
     std::vector<SRPCServiceResponse> ShutdownCore(const std::string& host_name_);

    /**
     * @brief Shutdown eCAL rpc service on defined host.
     *
     * @param host_name_    Host name ("" == on all hosts).
     *
     * @return  Vector of host names on which eCAL rpc service was shutdown.
     *          (Dll interface returns number of closed processes)
    **/
     std::vector<SRPCServiceResponse> ShutdownRPCService(const std::string& host_name_);
#endif /* ! ECAL_C_DLL */

    ///**
    // * @brief Open eCAL recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // * @param args_         Recorder arguments.
    // *
    // * @return  Zero if succeeded.
    //**/
    // int RecorderOpen(const std::string& host_name_, const std::string& args_);

    ///**
    // * @brief Close eCAL recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // *
    // * @return  Zero if succeeded.
    //**/
    // int RecorderClose(const std::string& host_name_);

    ///**
    // * @brief Start eCAL recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // *
    // * @return  Zero if succeeded.
    //**/
    // int RecorderStart(const std::string& host_name_);

    ///**
    // * @brief Open eCAL CAN recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // * @param args_         Recorder arguments.
    // *
    // * @return  Zero if succeeded.
    //**/
    // int CanRecorderOpen(const std::string& host_name_, const std::string& args_);

    ///**
    // * @brief Close eCAL CAN recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // *
    // * @return  Zero if succeeded.
    //**/
    // int CanRecorderClose(const std::string& host_name_);

    ///**
    // * @brief Start eCAL CAN recorder on defined host.
    // *
    // * @param host_name_    Host name ("" == on all hosts).
    // *
    // * @return  Zero if succeeded.
    //**/
    // int CanRecorderStart(const std::string& host_name_);

    ///**
    //* @brief Open local eCAL Recorder working application.
    //*
    //* @param args_   Command line arguments.
    //*
    //* @return  True if it succeeds, false if it fails.
    //**/
    // bool RecorderOpenL(const char* args_);

    ///**
    //* @brief Close local eCAL Recorder working application.
    //*
    //* @return  True if it succeeds, false if it fails.
    //**/
    // bool RecorderCloseL();

    ///**
    //* @brief Start local eCAL Recording.
    //*
    //* @return  True if it succeeds, false if it fails.
    //**/
    // bool RecorderStartL();

    ///**
    //* @brief Open local eCAL CAN Recorder working application.
    //*
    //* @param args_   Command line arguments.
    //*
    //* @return  True if it succeeds, false if it fails.
    //**/
    // bool CanRecorderOpenL(const char* args_);

    ///**
    //* @brief Close local eCAL CAN Recorder working application.
    //*
    //* @return  True if it succeeds, false if it fails.
    //**/
    // bool CanRecorderCloseL();

    ///**
    //* @brief Start local eCAL CAN Recording.
    //*
    //* @return  True if it succeeds, false if it fails.
    //**/
    // bool CanRecorderStartL();

    ///**
    //* @brief Open eCALRPCService application.
    //*
    //* @return  True if it succeeds, false if it fails.
    //**/
    // bool RPCServiceOpenL();
  }
}

#ifdef ECAL_C_DLL

#include <ecal/ecalc.h>

namespace eCAL
{
  namespace RPC
  {
    inline int SetTimeout(int timeout_)
    {
      return(eCAL_RPC_SetTimeout(timeout_));
    }

    inline int StartProcess(const std::string& host_name_, const std::string& process_name_, const std::string& process_dir_, const std::string& process_args_, bool create_console_, eCAL_Process_eStartMode process_mode_)
    {
      return(eCAL_RPC_StartProcess(host_name_.c_str(), process_name_.c_str(), process_dir_.c_str(), process_args_.c_str(), create_console_, process_mode_));
    }

    inline int StopProcess(const std::string& host_name_, const std::string& process_name_)
    {
      return(eCAL_RPC_StopProcessName(host_name_.c_str(), process_name_.c_str()));
    }

    inline int StopProcess(const std::string& host_name_, int process_id_)
    {
      return(eCAL_RPC_StopProcessID(host_name_.c_str(), process_id_));
    }

    inline int ShutdownProcess(const std::string& host_name_, const std::string& process_name_)
    {
      return(eCAL_RPC_ShutdownProcessName(host_name_.c_str(), process_name_.c_str()));
    }

    inline int ShutdownProcess(const std::string& host_name_, int process_id_)
    {
      return(eCAL_RPC_ShutdownProcessID(host_name_.c_str(), process_id_));
    }

    inline int ShutdownProcesses(const std::string& host_name_)
    {
      return(eCAL_RPC_ShutdownProcesses(host_name_.c_str()));
    }

    inline int ShutdownCore(const std::string& host_name_)
    {
     return(eCAL_RPC_ShutdownCore(host_name_.c_str()));
    }

    inline int ShutdownRPCService(const std::string& host_name_)
    {
      return(eCAL_RPC_ShutdownRPCService(host_name_.c_str()));
    }

    //inline int RecorderOpen(const std::string& host_name_, const std::string& args_)
    //{
    //  return(eCAL_RPC_RecorderOpen(host_name_.c_str(), args_.c_str()));
    //}

    //inline int RecorderClose(const std::string& host_name_)
    //{
    //  return(eCAL_RPC_RecorderClose(host_name_.c_str()));
    //}

    //inline int RecorderStart(const std::string& host_name_)
    //{
    //  return(eCAL_RPC_RecorderStart(host_name_.c_str()));
    //}

    //inline int CanRecorderOpen(const std::string& host_name_, const std::string& args_)
    //{
    //  return(eCAL_RPC_CanRecorderOpen(host_name_.c_str(), args_.c_str()));
    //}

    //inline int CanRecorderClose(const std::string& host_name_)
    //{
    //  return(eCAL_RPC_CanRecorderClose(host_name_.c_str()));
    //}

    //inline int CanRecorderStart(const std::string& host_name_)
    //{
    //  return(eCAL_RPC_CanRecorderStart(host_name_.c_str()));
    //}

    //inline bool RecorderOpenL(const char* args_)
    //{
    //  return(eCAL_RecorderOpen(args_) != 0);
    //}

    //inline bool RecorderCloseL()
    //{
    //  return(eCAL_RecorderClose() != 0);
    //}

    //inline bool RecorderStartL()
    //{
    //  return(eCAL_RecorderStart() != 0);
    //}

    //inline bool CanRecorderOpenL(const char* args_)
    //{
    //  return(eCAL_CanRecorderOpen(args_) != 0);
    //}

    //inline bool CanRecorderCloseL()
    //{
    //  return(eCAL_CanRecorderClose() != 0);
    //}

    //inline bool CanRecorderStartL()
    //{
    //  return(eCAL_CanRecorderStart() != 0);
    //}

    //inline bool RPCServiceOpenL()
    //{
    //  return(eCAL_RPCServiceOpen() != 0);
    //}
  }
}

#endif /* ECAL_C_DLL */
