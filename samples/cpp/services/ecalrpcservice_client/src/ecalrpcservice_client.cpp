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

#include <ecal/ecal.h>
#include <ecal/ecal_rpc.h>

#include <chrono>
#include <thread>

#define SLEEP_TIME 200

// main entry
int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "ecalrpcservice client", eCAL::Init::All);
  eCAL::RPC::Initialize();

  // set global rpc timeout for rpc service response
  eCAL::RPC::SetTimeout(500);

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // StartProcess on all hosts
  /*auto id_vec_start1 = */eCAL::RPC::StartProcess("", "notepad.exe", "", "", false, proc_smode_normal);

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));

  // StopProcess by name on all hosts
  /*auto id_vec_stop1 = */eCAL::RPC::StopProcess("", "notepad.exe");

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));

  // start / stop process sequence
  int cnt = 5;
  while(cnt--)
  {
    // StartProcess on this host
    /*auto id_vec_start2 = */eCAL::RPC::StartProcess(eCAL::Process::GetHostName(), "notepad.exe", "", "", false, proc_smode_normal);

    // sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));

    // StopProcess by name on this host
    /*auto id_vec_stop2 = */eCAL::RPC::StopProcess(eCAL::Process::GetHostName(), "notepad.exe");

    // sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));

    // StartProcess on this host
    auto id_vec_start3 = eCAL::RPC::StartProcess(eCAL::Process::GetHostName(), "notepad.exe", "", "", false, proc_smode_normal);

    // sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));

    // StopProcess by host_name and id
    for(auto iter : id_vec_start3)
    {
      std::string host_name = iter.host_name;
      int         pid       = iter.pid;
      /*auto id_vec_stop3 = */eCAL::RPC::StopProcess(host_name, pid);
    }

    // sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
  }

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Open eCAL Recorder on this host
  eCAL::RPC::RecorderOpen(eCAL::Process::GetHostName(), "");

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Start eCAL Recorder on this host
  eCAL::RPC::RecorderStart(eCAL::Process::GetHostName());

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  // Close eCAL Recorder on this host
  eCAL::RPC::RecorderClose(eCAL::Process::GetHostName());

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Shut down eCAL processes on this host
  eCAL::RPC::ShutdownProcesses(eCAL::Process::GetHostName());

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Shut down eCAL core components on this host
  eCAL::RPC::ShutdownCore(eCAL::Process::GetHostName());

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // Shut down eCAL rpc service on this host
  eCAL::RPC::ShutdownRPCService(eCAL::Process::GetHostName());

  // sleep
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
