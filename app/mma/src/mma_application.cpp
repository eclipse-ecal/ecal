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

#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "ecal/ecal.h"
#include "ecal/process.h"
#include "ecal/msg/protobuf/publisher.h"

#include "../include/logger.h"
#include "../include/mma.h"
#include "../include/mma_defs.h"
#include "../include/zombie_instance_killer.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/app/pb/mma/mma.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef ECAL_OS_WINDOWS
#include <windows.h>
#include <conio.h>
#include <direct.h>
#include <TlHelp32.h>
#else
#include <cstdlib>
#include <iostream>
#include <list>
#include <stdio.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int _kbhit()
{
  static const int STDIN = 0;
  static bool initialized = false;

  if (!initialized)
  {
    // Use termios to turn off line buffering
    termios term;
    tcgetattr(STDIN, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN, TCSANOW, &term);
    setbuf(stdin, NULL);
    initialized = true;
  }

  int bytesWaiting;
  ioctl(STDIN, FIONREAD, &bytesWaiting);
  return bytesWaiting;
}

int _getch(void)
{
  struct termios oldattr, newattr;
  int ch;
  tcgetattr(STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  return ch;
}
#endif  // ECAL_OS_WINDOWS

bool PadString(std::string& str, char padding_char, size_t max_size)
{
  bool ret_val = false;

  if (str.length() < max_size)
  {
    size_t empty_space = max_size - str.length();
    size_t left_padding = empty_space / 2;

    str.insert(0, left_padding, padding_char);
    str.insert(str.length(), empty_space - left_padding, padding_char);
    ret_val = true;
  }

  return ret_val;
}

int main(int argc, char** argv)
{
  std::list<std::string> processes_names;

#ifdef ECAL_OS_WINDOWS
  processes_names.push_back("ecal_mma.exe");
#endif // ECAL_OS_WINDOWS
#ifdef ECAL_OS_LINUX
  processes_names.push_back("ecal_mma");
#endif // ECAL_OS_LINUX

  if (ZombieInstanceKiller::KillZombieInstance(processes_names))
  {
    // enable closing of log files
    eCAL::Process::SleepMS(1000);
  }

  std::string app_version_header  = " " + std::string(MMA_APPLICATION_NAME) + " " + std::string(MMA_VERSION) + " ";
  std::string ecal_version_header = " (eCAL Lib " + std::string(ECAL_VERSION) + ") ";

  PadString(app_version_header,  '-', 79);
  PadString(ecal_version_header, ' ', 79);

  std::cout << app_version_header << std::endl << ecal_version_header << std::endl << std::endl;

  // initialize eCAL API
  if (!eCAL::Initialize(MMA_APPLICATION_NAME, eCAL::Init::Publisher))
  {
    std::cout << "eCAL initialization failed !";
    return 1;
  }

  if (argc == 2 && (strcmp(argv[1], "/v") == 0 || strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--v") == 0))
  {
    std::cout << "Logging enabled" << std::endl;
    Logger::getLogger()->ResumeLogging();
  }

  eCAL::Process::SetState(eCAL::Process::eSeverity::healthy, eCAL::Process::eSeverityLevel::level1, "Running");

  // create mma agent
  std::cout << std::endl << "Initializing machine monitoring agent ..." << std::endl;
  MMA machine_monitoring_agent;
  std::cout << "Machine monitoring agent initialized" << std::endl << std::endl;

  // start agent thread
  std::atomic<bool> agent_stop(false);
  std::thread agent_t([&]() {

    eCAL::pb::mma::State machine_state;
    eCAL::protobuf::CPublisher<eCAL::pb::mma::State> state_publisher("machine_state_" + eCAL::Process::GetHostName());

    std::cout << "Sending state info ..." << std::endl << std::endl;
    std::cout << "Press ESC to exit"      << std::endl;

    // send state every second
    while (!agent_stop)
    {
      machine_monitoring_agent.Get(machine_state);
      state_publisher.Send(machine_state);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  });

  // enter main loop
  while (eCAL::Ok() == true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (_kbhit() > 0 && _getch() == 0x1B /*ESC*/)
      break;
  }

  // stop agent thread
  std::cout << std::endl << "Terminating machine monitoring agent ..." << std::endl;
  agent_stop = true; 
  agent_t.join();
  // finalize eCAL API
  eCAL::Finalize();

  return 0;
}
