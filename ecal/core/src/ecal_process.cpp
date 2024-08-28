/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
* @brief  eCAL process interface
**/

#include <cstdint>
#include <ecal/ecal.h>

#include "ecal_def.h"
#include "ecal_globals.h"

#include "ecal_process_stub.h"
#include "ecal_utils/command_line.h"
#include "ecal_utils/str_convert.h"

#include "io/udp/ecal_udp_configurations.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef ECAL_OS_WINDOWS
#include "ecal_win_main.h"
#include <iphlpapi.h>
#include <ecal_utils/str_convert.h>
#endif /* ECAL_OS_WINDOWS */

#ifdef ECAL_OS_LINUX
#include <spawn.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/select.h>
#include <limits.h>
#include <netinet/in.h>
#include <ecal_utils/ecal_utils.h>
#endif /* ECAL_OS_LINUX */

#ifdef ECAL_OS_MACOS
#include <mach-o/dyld.h>
#include <sys/sysctl.h>
#endif // ECAL_OS_MACOS

#ifdef ECAL_OS_FREEBSD
#include <sys/user.h>
#include <sys/sysctl.h>
#include <libutil.h>
#endif

#ifdef ECAL_CORE_NPCAP_SUPPORT
#include <ecaludp/npcap_helpers.h>
#endif // ECAL_CORE_NPCAP_SUPPORT

#ifndef NDEBUG
#define STD_COUT_DEBUG( x ) { std::stringstream ss; ss << x; std::cout << ss.str(); }
#else
#define STD_COUT_DEBUG( x )
#endif

namespace
{
  std::string GetBufferStr(int size)
  {
    std::string unit = "byte";
    if (size > 1024)
    {
      size /= 1024;
      unit = "kByte";
    }
    if (size > 1024)
    {
      size /= 1024;
      unit = "MByte";
    }
    return std::to_string(size) + " " + unit;
  }

  std::string LayerMode(int mode)
  {
    switch (mode)
    {
    case 0:
      return "off";
      break;
    case 1:
      return "on";
      break;
    case 2:
      return "auto";
      break;
    }
    return "???";
  }
}

namespace eCAL
{
  namespace Process
  {
    void DumpConfig()
    {
      std::string cfg;
      DumpConfig(cfg);
      std::cout << cfg;
    }

    void DumpConfig(std::string& cfg_s_)
    {
      std::stringstream sstream;
      sstream << "------------------------- SYSTEM ---------------------------------" << '\n';
      sstream << "Version                  : " << ECAL_VERSION << " (" << ECAL_DATE << ")" << '\n';
#ifdef ECAL_OS_WINDOWS
#ifdef _WIN64
      sstream << "Platform                 : x64" << '\n';
#else
      sstream << "Platform                 : win32" << std::endl;
#endif
#endif
#ifdef ECAL_OS_LINUX
      sstream << "Platform                 : linux" << std::endl;
#endif
      sstream << '\n';

      if (eCAL::IsInitialized() == 0)
      {
        sstream << "Components               : NOT INITIALIZED ( call eCAL::Initialize() )";
        sstream << '\n';
        cfg_s_ = sstream.str();
        return;
      }

      sstream << "------------------------- CONFIGURATION --------------------------" << '\n';
      sstream << "Default INI              : " << Config::GetLoadedEcalIniPath() << '\n';
      sstream << '\n';

      sstream << "------------------------- NETWORK --------------------------------" << '\n';
      sstream << "Host name                : " << Process::GetHostName() << '\n';
      sstream << "Host group name          : " << Process::GetHostGroupName() << '\n';

      if (Config::IsNetworkEnabled())
      {
        sstream << "Network mode             : cloud" << '\n';
      }
      else
      {
        sstream << "Network mode             : local" << '\n';
      }
      sstream << "Network ttl              : " << UDP::GetMulticastTtl() << '\n';
      sstream << "Network sndbuf           : " << GetBufferStr(Config::GetUdpMulticastSndBufSizeBytes()) << '\n';
      sstream << "Network rcvbuf           : " << GetBufferStr(Config::GetUdpMulticastRcvBufSizeBytes()) << '\n';
      sstream << "Multicast cfg version    : v" << static_cast<uint32_t>(Config::GetUdpMulticastConfigVersion()) << '\n';
      sstream << "Multicast group          : " << Config::GetUdpMulticastGroup() << '\n';
      sstream << "Multicast mask           : " << Config::GetUdpMulticastMask() << '\n';
      const int port = Config::GetUdpMulticastPort();
      sstream << "Multicast ports          : " << port << " - " << port + 10 << '\n';
      sstream << "Multicast join all IFs   : " << (Config::IsUdpMulticastJoinAllIfEnabled() ? "on" : "off") << '\n';
      sstream << '\n';

#if ECAL_CORE_TIMEPLUGIN
      sstream << "------------------------- TIME -----------------------------------" << '\n';
      sstream << "Synchronization realtime : " << Config::GetTimesyncModuleName() << '\n';
      sstream << "Synchronization replay   : " << Config::GetTimesyncModuleReplay() << '\n';
      sstream << "State                    : ";
      if (g_timegate()->IsSynchronized()) sstream << " synchronized " << '\n';
      else                                sstream << " not synchronized " << '\n';
      sstream << "Master / Slave           : ";
      if (g_timegate()->IsMaster())       sstream << " Master " << '\n';
      else                                sstream << " Slave " << '\n';
      int         status_state = 0;
      std::string status_msg;
      g_timegate()->GetStatus(status_state, &status_msg);
      sstream << "Status (Code)            : \"" << status_msg << "\" (" << status_state << ")" << '\n';
      sstream << '\n';
#endif
#if ECAL_CORE_SUBSCRIBER
      sstream << "------------------------- SUBSCRIPTION LAYER DEFAULTS ------------" << '\n';
      sstream << "Layer Mode UDP MC        : " << LayerMode(Config::IsUdpMulticastRecEnabled()) << '\n';
      sstream << "Drop out-of-order msgs   : " << (Config::Experimental::GetDropOutOfOrderMessages() ? "on" : "off") << '\n';
#endif
#ifdef ECAL_CORE_NPCAP_SUPPORT
      sstream << "Npcap UDP Reciever       : " << LayerMode(Config::IsNpcapEnabled());
      if(Config::IsNpcapEnabled() && !ecaludp::npcap::is_initialized())
      {
        sstream << " (Init FAILED!)";
      }
#else  // ECAL_CORE_NPCAP_SUPPORT
      if (Config::IsNpcapEnabled())
      {
        sstream << " (Npcap is enabled, but not configured via CMake!)";
      }
#endif // ECAL_CORE_NPCAP_SUPPORT
      sstream << '\n';

      // write it into std:string
      cfg_s_ = sstream.str();
    }

    std::string GetHostName()
    {
      if (g_host_name.empty())
      {
        char hname[1024] = { 0 };
        if (gethostname(hname, 1024) == 0)
        {
          g_host_name = hname;
        }
        else
        {
          std::cerr << "Unable to get host name" << '\n';
        }
      }
      return(g_host_name);
    }

    std::string GetHostGroupName()
    {
      return Config::GetHostGroupName().empty() ? GetHostName() : Config::GetHostGroupName();
    }

    std::string GetUnitName()
    {
      return(g_unit_name);
    }

    std::string GetTaskParameter(const char* sep_)
    {
      std::string par_line;
      for (const auto& par : g_task_parameter)
      {
        if (!par_line.empty()) par_line += sep_;
        par_line += par;
      }
      return(par_line);
    }

    void SleepMS(const long time_ms_)
    {
      #ifdef ECAL_OS_WINDOWS
      {
        Sleep(time_ms_);
      }
      #else
        std::this_thread::sleep_for(std::chrono::milliseconds(time_ms_));
      #endif
    }

    void SleepNS(const long long time_ns_)
    {
      #ifdef ECAL_OS_WINDOWS
      {
        auto milliseconds = time_ns_ / 1000000 + ((time_ns_ % 1000000) != 0);
        Sleep(static_cast<DWORD>(milliseconds));
      }
      #else
        std::this_thread::sleep_for(std::chrono::nanoseconds(time_ns_));
      #endif
    }

    void SetState(eCAL_Process_eSeverity severity_, eCAL_Process_eSeverity_Level level_, const char* info_)
    {
      g_process_severity = severity_;
      g_process_severity_level = level_;
      if (info_ != nullptr)
      {
        g_process_info = info_;
      }
    }

    int AddRegistrationCallback(enum eCAL_Registration_Event event_, const RegistrationCallbackT& callback_)
    {
#if ECAL_CORE_REGISTRATION
      if (g_registration_receiver() == nullptr) return -1;
      if (g_registration_receiver()->AddRegistrationCallback(event_, callback_)) return 0;
#endif
      return -1;
    }

    int RemRegistrationCallback(enum eCAL_Registration_Event event_)
    {
#if ECAL_CORE_REGISTRATION
      if (g_registration_receiver() == nullptr) return -1;
      if (g_registration_receiver()->RemRegistrationCallback(event_)) return 0;
#endif
      return -1;
    }
  }
}


#ifdef ECAL_OS_WINDOWS

namespace
{
  void create_proc_id()
  {
    if (eCAL::g_process_id == 0)
    {
      eCAL::g_process_id   = GetCurrentProcessId();
      eCAL::g_process_id_s = std::to_string(eCAL::g_process_id);
    }
  }
}

namespace eCAL
{
  namespace Process
  {
    int GetProcessID()
    {
      create_proc_id();
      return(g_process_id);
    }

    std::string GetProcessIDAsString()
    {
      create_proc_id();
      return(g_process_id_s);
    }

    std::string GetProcessName()
    {
      if (g_process_name.empty())
      {
        WCHAR pname[1024] = { 0 };
        GetModuleFileNameExW(GetCurrentProcess(), nullptr, pname, 1024);
        g_process_name = EcalUtils::StrConvert::WideToUtf8(pname);
      }
      return(g_process_name);
    }

    std::string GetProcessParameter()
    {
      if (g_process_par.empty())
      {
        g_process_par = EcalUtils::CommandLine::GetUtf8CommandLine();
      }
      return(g_process_par);
    }

    int StartProcess(const char* proc_name_, const char* proc_args_, const char* working_dir_, const bool create_console_, const eCAL_Process_eStartMode process_mode_, const bool block_)
    {
      int ret_pid = 0;

      std::string proc_name = proc_name_;
      char exp_name[MAX_PATH] = { 0 };
      if (ExpandEnvironmentStringsA(proc_name.c_str(), exp_name, MAX_PATH) > 0)
      {
        proc_name = exp_name;
      }

      const std::string proc_args = proc_args_;
      if (!proc_args.empty())
      {
        proc_name += " ";
        proc_name += proc_args;
      }

      short creation_flag = 0;
      if (create_console_)
      {
        creation_flag = CREATE_NEW_CONSOLE;
      }

      short win_state = 0;
      switch (process_mode_)
      {
      case 0:
        win_state = SW_SHOW;
        break;
      case 1:
        win_state = SW_HIDE;
        break;
      case 2:
        win_state = SW_MINIMIZE;
        break;
      case 3:
        win_state = SW_MAXIMIZE;
        break;
      default:
        win_state = SW_SHOW;
        break;
      }

      std::string working_dir = working_dir_;
      if (working_dir.empty())
      {
        char* wd(nullptr);
        if ((wd = _getcwd(nullptr, 0)) != nullptr)
        {
          working_dir = wd;
          free(wd);
        }
      }
      if (!working_dir.empty())
      {
        char exp_dir[MAX_PATH] = { 0 };
        if (ExpandEnvironmentStringsA(working_dir.c_str(), exp_dir, MAX_PATH) > 0)
        {
          working_dir = exp_dir;
        }
      }

      PROCESS_INFORMATION pi{};
      STARTUPINFOW si =
      {
        sizeof(STARTUPINFO),          //DWORD   cb;
        NULL,                         //LPTSTR  lpReserved;
        NULL,                         //LPTSTR  lpDesktop;
        NULL,                         //LPTSTR  lpTitle;
        0,                            //DWORD   dwX;
        0,                            //DWORD   dwY;
        0,                            //DWORD   dwXSize;
        0,                            //DWORD   dwYSize;
        0,                            //DWORD   dwXCountChars;
        0,                            //DWORD   dwYCountChars;
        0,                            //DWORD   dwFillAttribute;
        STARTF_USESHOWWINDOW,         //DWORD   dwFlags
        WORD(win_state),              //WORD    wShowWindow;
        0,                            //WORD    cbReserved2;
        NULL,                         //LPBYTE  lpReserved2;
        NULL,                         //HANDLE  hStdInput;
        NULL,                         //HANDLE  hStdOutput;
        NULL                          //HANDLE  hStdError;
      };

      std::wstring w_proc_name   = EcalUtils::StrConvert::Utf8ToWide(proc_name);
      std::wstring w_working_dir = EcalUtils::StrConvert::Utf8ToWide(working_dir);

      w_proc_name.  push_back('\0'); // 0-termiante string
      w_working_dir.push_back('\0'); // 0-termiante string


      if (CreateProcessW(NULL, &w_proc_name[0], NULL, NULL, 0, creation_flag, NULL, w_working_dir.c_str(), (LPSTARTUPINFOW)&si, &pi) != 0)
      {
        ret_pid = pi.dwProcessId;
      }

      if (block_)
      {
        // Wait until child process exits.
        if (pi.hProcess != nullptr) WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles.
        if (pi.hProcess != nullptr) CloseHandle(pi.hProcess);
        if (pi.hThread != nullptr)  CloseHandle(pi.hThread);
      }

      return(ret_pid);
    }

    bool StopProcess(const char* proc_name_)
    {
      std::string full_proc_name = proc_name_;
      // check extension ..
      // to stop something we need the process name including
      // file extension
      std::string ext;
      const int ext_size(4); //-V112
      if (full_proc_name.size() > ext_size)
      {
        ext = full_proc_name.substr(full_proc_name.length() - ext_size, full_proc_name.length());
      }
      if (ext != ".exe")
      {
        full_proc_name += ".exe";
      }

      std::string commandline;
      commandline += "taskkill /f /im ";
      commandline += full_proc_name;
      commandline += " /t";

      PROCESS_INFORMATION pi{};
      STARTUPINFOW si =
      {
        sizeof(STARTUPINFO),          //DWORD   cb;
        NULL,                         //LPTSTR  lpReserved;
        NULL,                         //LPTSTR  lpDesktop;
        NULL,                         //LPTSTR  lpTitle;
        0,                            //DWORD   dwX;
        0,                            //DWORD   dwY;
        0,                            //DWORD   dwXSize;
        0,                            //DWORD   dwYSize;
        0,                            //DWORD   dwXCountChars;
        0,                            //DWORD   dwYCountChars;
        0,                            //DWORD   dwFillAttribute;
        STARTF_USESHOWWINDOW,         //DWORD   dwFlags
        SW_HIDE,                      //WORD    wShowWindow;
        0,                            //WORD    cbReserved2;
        NULL,                         //LPBYTE  lpReserved2;
        NULL,                         //HANDLE  hStdInput;
        NULL,                         //HANDLE  hStdOutput;
        NULL                          //HANDLE  hStdError;
      };

      std::wstring w_commandline = EcalUtils::StrConvert::Utf8ToWide(commandline);
      w_commandline.push_back('\0'); // 0-termiante string

      if (CreateProcessW(NULL, &w_commandline[0], NULL, NULL, 0, 0, NULL, NULL, (LPSTARTUPINFOW)&si, &pi) != 0) {
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD taskkill_error = 1;
        GetExitCodeProcess(pi.hProcess, &taskkill_error);

        // Close process and thread handles.
        if (pi.hProcess != nullptr) CloseHandle(pi.hProcess);
        if (pi.hThread != nullptr)  CloseHandle(pi.hThread);

        return (taskkill_error == 0);
      }
      else {
        return false;
      }
    }

    bool StopProcess(const int proc_id_)
    {
      if (proc_id_ <= 0) return false;

      std::string commandline;
      commandline += "taskkill /f /pid ";
      commandline += std::to_string(proc_id_);
      commandline += " /t";

      PROCESS_INFORMATION pi{};
      STARTUPINFOW si =
      {
        sizeof(STARTUPINFO),          //DWORD   cb;
        NULL,                         //LPTSTR  lpReserved;
        NULL,                         //LPTSTR  lpDesktop;
        NULL,                         //LPTSTR  lpTitle;
        0,                            //DWORD   dwX;
        0,                            //DWORD   dwY;
        0,                            //DWORD   dwXSize;
        0,                            //DWORD   dwYSize;
        0,                            //DWORD   dwXCountChars;
        0,                            //DWORD   dwYCountChars;
        0,                            //DWORD   dwFillAttribute;
        STARTF_USESHOWWINDOW,         //DWORD   dwFlags
        SW_HIDE,                      //WORD    wShowWindow;
        0,                            //WORD    cbReserved2;
        NULL,                         //LPBYTE  lpReserved2;
        NULL,                         //HANDLE  hStdInput;
        NULL,                         //HANDLE  hStdOutput;
        NULL                          //HANDLE  hStdError;
      };

      std::wstring w_commandline = EcalUtils::StrConvert::Utf8ToWide(commandline);
      w_commandline.push_back('\0'); // 0-termiante string

      if (CreateProcessW(NULL, &w_commandline[0], NULL, NULL, 0, 0, NULL, NULL, (LPSTARTUPINFOW)&si, &pi) != 0) {
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD taskkill_error = 1;
        GetExitCodeProcess(pi.hProcess, &taskkill_error);

        // Close process and thread handles.
        if (pi.hProcess != nullptr) CloseHandle(pi.hProcess);
        if (pi.hThread != nullptr)  CloseHandle(pi.hThread);

        return (taskkill_error == 0);
      }
      else {
        return false;
      }
    }
  }
}

#endif /* ECAL_OS_WINDOWS */


#ifdef ECAL_OS_LINUX

extern char **environ;

namespace
{
  void create_proc_id()
  {
    if (eCAL::g_process_id == 0)
    {
      eCAL::g_process_id   = getpid();
      eCAL::g_process_id_s = std::to_string(eCAL::g_process_id);
    }
  }

  /**
   * @brief Checks whether all requirements for using a terminal emulator are fulfilled and then returns the according command
   * @return The terminal emulator command or an empty string, if the requirements are not fulfilled
   */
  std::string getTerminalEmulatorCommand()
  {
    // Check whether we are able to use a terminal emulator. The requirements
    // are:
    //   - the DISPLAY variable must be set
    //   - the terminal_emulator must be set in the ecal.yaml
    //   - ecal_process_stub bust be available AND print the correct version

    // ------------------------ DISPLAY variable check -------------------------

    char* display = getenv("DISPLAY");
    if (display && (strlen(display)))
    {
      STD_COUT_DEBUG("[PID " << getpid() << "]: " << "DISPLAY is: " << display << std::endl);
    }
    else
    {
      STD_COUT_DEBUG("[PID " << getpid() << "]: " << "DISPLAY not set. Not using terminal emulator." << std::endl);
      return "";
    }

    // -------------------- terminal_emulator command check --------------------

    const std::string terminal_emulator_command = eCAL::Config::GetTerminalEmulatorCommand();
    if (!terminal_emulator_command.empty())
    {
      STD_COUT_DEBUG("[PID " << getpid() << "]: " << "ecal.yaml terminal emulator command is: " << terminal_emulator_command << std::endl);
    }
    else
    {
      STD_COUT_DEBUG("[PID " << getpid() << "]: " << "ecal.yaml terminal emulator command is not set. Not using terminal emulator." << std::endl);
      return "";
    }

    // ------------------------ ecal_process_stub check ------------------------
    STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Testing ecal_process_stub..." << std::endl);

    std::array<char, 128> buffer;
    std::string process_stub_output;

    FILE* pipe = popen("ecal_process_stub --version", "r");

    if (pipe)
    {
      while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
      {
          process_stub_output += buffer.data();
      }
      process_stub_output = EcalUtils::String::Trim(process_stub_output);

      STD_COUT_DEBUG("[PID " << getpid() << "]: " << process_stub_output << std::endl);

      if (process_stub_output != std::string(ECAL_PROCESS_STUB_VERSION_STRING))
      {
        std::cerr << "[PID " << getpid() << "]: " << "Error testing for ecal_process_stub: Got faulty version string: \"" << process_stub_output << "\". "
                  << "Maybe eCAL is not installed correctly. Switching to fallback mode." << std::endl;
        return "";
      }

      int exit_status = pclose(pipe);
      if (exit_status == -1)
      {
        std::cerr << "[PID " << getpid() << "]: " << "Unable to close pipe to ecal_process_stub instance: " << strerror(errno) << std::endl;
      }
      else
      {
        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "ecal_process_stub terminated with code " << exit_status << std::endl);
      }
    }
    else
    {
      std::cerr << "[PID " << getpid() << "]: " << "Error testing for ecal_process_stub: " << strerror(errno) << ". "
                << "Maybe eCAL is not installed correctly. Switching to fallback mode." << std::endl;
      return "";
    }

    return terminal_emulator_command;
  }
}

namespace eCAL
{
  namespace Process
  {
    int GetProcessID()
    {
      create_proc_id();
      return(g_process_id);
    }

    std::string GetProcessIDAsString()
    {
      create_proc_id();
      return(g_process_id_s);
    }

    /**
    * @brief Returns the fully qualified path for the current process's binary
    *
    * @return the process path
    */
    std::string GetProcessName()
    {
      if (g_process_name.empty()) {
        // Read the link to our own executable
        char buf[PATH_MAX] = { 0 };
#if defined(ECAL_OS_MACOS)
        uint32_t length = PATH_MAX;
        if (_NSGetExecutablePath(buf, &length) != 0)
        {
          // Buffer size is too small.
          return "";
        }
        length = strlen(buf);
#elif defined(ECAL_OS_QNX)
        size_t length {0};
        // TODO: Find a suitable method on QNX to retrieve current process name
#elif defined(ECAL_OS_FREEBSD)
        size_t length {0};
        struct kinfo_proc *proc = kinfo_getproc(getpid());
        if (proc)
        {
          strncpy(buf, proc->ki_comm, sizeof(buf));
          length = strlen(buf);
          free(proc);
        }
#else
        ssize_t length = readlink("/proc/self/exe", buf, PATH_MAX);

        if (length < 0)
        {
          std::cerr << "Unable to get process name: " << strerror(errno) << std::endl;
          return "";
        }
#endif
        // Copy the binary name to a std::string
        g_process_name = std::string(buf, length);

      }
      return g_process_name;
    }
    std::string GetProcessParameter()
    {
      if (g_process_par.empty())
      {
#if defined(ECAL_OS_MACOS)
        int pid = getpid();

        int    mib[3], argmax, argc;
        size_t    size;

        mib[0] = CTL_KERN;
        mib[1] = KERN_ARGMAX;

        size = sizeof(argmax);
        if (sysctl(mib, 2, &argmax, &size, NULL, 0) == -1)
        {
          return "";
        }

        /* Allocate space for the arguments. */
        std::vector<char> procargs(argmax);

        /*
         * Make a sysctl() call to get the raw argument space of the process.
         * The layout is documented in start.s, which is part of the Csu
         * project.  In summary, it looks like:
         *
         * /---------------\ 0x00000000
         * :               :
         * :               :
         * |---------------|
         * | argc          |
         * |---------------|
         * | arg[0]        |
         * |---------------|
         * :               :
         * :               :
         * |---------------|
         * | arg[argc - 1] |
         * |---------------|
         * | 0             |
         * |---------------|
         * | env[0]        |
         * |---------------|
         * :               :
         * :               :
         * |---------------|
         * | env[n]        |
         * |---------------|
         * | 0             |
         * |---------------| <-- Beginning of data returned by sysctl() is here.
         * | argc          |
         * |---------------|
         * | exec_path     |
         * |:::::::::::::::|
         * |               |
         * | String area.  |
         * |               |
         * |---------------| <-- Top of stack.
         * :               :
         * :               :
         * \---------------/ 0xffffffff
         */
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROCARGS2;
        mib[2] = pid;

        size = (size_t)argmax;
        if (sysctl(mib, 3, procargs.data(), &size, NULL, 0) == -1)
        {
          return "";
        }

        // First few bytes are the argc
        memcpy(&argc, procargs.data(), sizeof(argc));
        size_t pos = sizeof(argc);

        if (argc == 0)
          return "";

        // Skip the saved exec_path
        for (; pos <= procargs.size(); pos++)
        {
          if (procargs[pos] == '\0')
          {
            // End of exec_path reached
            break;
          }
        }
        if (pos >= procargs.size())
          return "";

        // Skip trailing '\0' characters
        for (; pos <= procargs.size(); pos++)
        {
          if (procargs[pos] != '\0') {
            // Beginning of first argument reached
            break;
          }
        }
        if (pos >= procargs.size())
          return "";

        // Iterate through the '\0' terminated strings and copy them to a C++ vector
        std::vector<std::string> argument_vector;
        argument_vector.reserve(argc);

        size_t current_arg_start = pos;
        int current_arg_number = 0;
        for (size_t i = pos; i < procargs.size(); ++i)
        {
          if (procargs[i] == '\0')
          {
            std::string arg = std::string(&procargs[current_arg_start], i - current_arg_start);
            argument_vector.push_back(arg);
            current_arg_start = i + 1;
            current_arg_number++;
          }

          if (current_arg_number == argc)
            break;
        }

#elif defined(ECAL_OS_QNX)
        // TODO: Find a suitable method on QNX to get process arguments of the current executable
        std::vector<std::string> argument_vector;
        g_process_par = "";
#elif defined(ECAL_OS_FREEBSD)
        // create mib structure for sysctl call
        int mib[4];
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_ARGS;
        mib[3] = getpid();

        // get the length of the return value buffer
        size_t len {0};
        sysctl(mib, sizeof mib , NULL, &len, NULL, 0);

        // retrieve process arguments as null-character seperated c-string
        auto buffer = std::make_unique<char[]>(len);
        sysctl(mib, sizeof mib, buffer.get(), &len, NULL, 0);
        size_t position = 0;
        
        // convert null-character seperated c-string to std::vector<std::string>
        std::vector<std::string> argument_vector;
        while(position < len)
        {
          const char* arg = &(buffer.get()[position]);
          argument_vector.push_back(arg);
          position += strlen(arg) + 1;
        }
#else

        const std::string filename = "/proc/self/cmdline";
        std::vector<std::string> argument_vector;

        std::ifstream cmdline_file(filename, std::ios::binary);
        if (!cmdline_file.is_open())
        {
          std::cerr << "Failed to open " << filename << '\n';
          return "";
        }
        else
        {
          std::string arg;
          while (std::getline(cmdline_file, arg, '\0')) // the cmdline contains arguments separated by \0
          {
            argument_vector.emplace_back(arg);
          }
        }

#endif // ECAL_OS_MACOS

        size_t complete_char_num(0);
        for (std::string& argument : argument_vector)
        {
          std::string escaped_arg;
          escaped_arg.reserve(argument.size() + 2);

          bool constains_space = (argument.find(' ') != std::string::npos);

          // Escape special characters
          if (constains_space) escaped_arg += '\"';
          for (char c : argument)
          {
            if (c == '\\')                              // Escape [\]
              escaped_arg += "\\\\";
            else if (c == '\"')                         // Escape ["]
              escaped_arg += "\\\"";
            else if (c == '\'')                         // Escape [']
              escaped_arg += "\\\'";
            else
              escaped_arg += c;
          }
          if (constains_space) escaped_arg += '\"';

          if(escaped_arg.empty())
            escaped_arg = "\"\"";

          complete_char_num += escaped_arg.size();
          argument = escaped_arg;
        }

        std::string process_par;
        process_par.reserve(complete_char_num + argument_vector.size());

        for (auto arg_it = argument_vector.begin(); arg_it != argument_vector.end(); arg_it++)
        {
          if (arg_it != argument_vector.begin())
            process_par += ' ';
          process_par += *arg_it;
        }

        g_process_par = process_par;
      }
      return(g_process_par);
    }

    int StartProcess(const char* proc_name_,
      const char* proc_args_,
      const char* working_dir_,
      const bool /* create_console_ */,
      const eCAL_Process_eStartMode  process_mode_,
      const bool  block_)
    {
      // Evaluate whether we want to use a terminal emulator. We only use a
      // terminal emulator, when:
      //   - The process_mode_ is not set to hidden
      //   - the DISPLAY variable indicates that we have a display attached
      //   - the terminal_emulator is set in the ecal.yaml
      //   - ecal_process_stub is available AND prints the correct version

      std::string terminal_emulator_command;
      if (process_mode_ != eCAL_Process_eStartMode::proc_smode_hidden)
      {
        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Checking requirements for using a terminal emulator... " << std::endl);
        terminal_emulator_command = getTerminalEmulatorCommand();
      }
      const bool use_terminal_emulator = (!terminal_emulator_command.empty());
      if (use_terminal_emulator)
      {
        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Using terminal emulator command: " << terminal_emulator_command << std::endl);
      }
      else
      {
        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Not using terminal emulator." << std::endl);
      }

      // Create a unique name for a FIFO and a lockfile. The FIFO is used to
      // block the main process and send the PID of the started process to it.
      // The lockfile is used to tell the main process whether loading the
      // target executable has succeeded:
      //
      //   [main]: creates FIFO
      //   [main]: fork() x 2 => creates child process
      //   [main]: reads FIFO => this will block until someone writes something
      //       [child]: creates lockfile and locks it
      //       [child]: sends current PID through the FIFO (weaks up main
      //                thread). Just having a PID here does not mean that
      //                executing the target process will succeed!
      //   [main]: received child-PID
      //   [main]: try to gain lock on lockfile => this will block until child
      //           releases the lock
      //       [child]: Execute target process
      //
      // =============
      // SUCCESS-case:
      // =============
      //       [child]: Linux kernel closes the lockfile for us (we cannot do
      //                that on our own any more, as the original child process
      //                has been replaced)
      //   [main]: Gaining the lock on the lockfile succeeds
      //   [main]: Read lockfile => empty => we know that the child
      //           successefully executed the target process and the PID we
      //           received earlier remains valid
      //
      //   -- Finished --
      //
      // =============
      // FAILURE-case:
      // =============
      //       [child]: Writes errno of the failure to the lockfile
      //       [child]: Close lockfile, release the lock and exit
      //   [main]: Gaining the lock on the lockfile succeeds
      //   [main]: Read lockfile => contains errno => we know that the child
      //           failed to execute the target process and the PID we received
      //           earlier is not valid
      //
      //   -- Finished --
      //
      // It should be noted, that in the terminal-emulator case, everything will
      // be more complex. In that case, the child will not write the FIFO and
      // lockfile. It will however start the terminal emulator which startes
      // ecal_process_stub that will do that.

      static int unique_counter = 0; // static counter to make sure we always get a unique fifo name
      std::string unique_name = "ecal_process_"
                              + std::to_string(getpid())
                              + "_"
                              + std::to_string(unique_counter++)
                              + "_"
                              + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());

      std::string fifo_name     = "/tmp/" + unique_name;
      std::string lockfile_name = "/var/lock/" + unique_name + ".lock";

      STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Creating FIFO \"" << fifo_name << "\"" << std::endl);

      if(mkfifo(fifo_name.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1)
      {
        std::cerr << "[PID " << getpid() << "]: " << "Error creating FIFO \"" << fifo_name << ": " << strerror(errno) << std::endl;
      }
      else
      {
        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Successfully created FIFO" << std::endl);
      }

      // Duplicate the current process. The child process (child_pid == 0) will
      // fork again and then call the desired executable. The main process
      // (child_pid != 0) will continue normally.
      STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Forking!" << std::endl);
      pid_t child_pid = fork();

      if (child_pid < 0)
      {
        std::cerr << "[PID " << getpid() << "]: " << "Error forking: " << strerror(errno) << std::endl;
        return 0;
      }
      else if (child_pid == 0)
      {
        // --------------------------- Child process ---------------------------
        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "First message from child process" << std::endl);

        // Create a new Session, if we start processes non-blocking. If we start
        // blocking, we don't create a new session, as otherwise the init
        // process (PID 1) would take the responsibility of waiting for the
        // process and retrieving its return value.
        if (!block_)
        {
          STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Creating new session" << std::endl);
          pid_t session_id = setsid();
          if (session_id == -1)
          {
            std::cerr << "[PID " << getpid() << "]: " << "Failed creating new session: " << strerror(errno) << std::endl;
          }
          else
          {
            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Successfully created new session with ID " << session_id << std::endl);
          }
        }

        // Fork the process again, into the child and the grandchild (The child
        // receives grandchild_pid != 0, the grandchild receives
        // grandchild_pid == 0).
        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Forking!" << std::endl);
        pid_t grandchild_pid = fork();
        if (grandchild_pid < 0)
        {
          std::cerr << "[PID " << getpid() << "]: " << "Error forking: " << strerror(errno) << std::endl;
          _exit(EXIT_FAILURE);
        }
        else if (grandchild_pid != 0)
        {
          _exit(EXIT_SUCCESS);
        }
        else
        {
          // ----------------------- Grandchild process ------------------------
          STD_COUT_DEBUG("[PID " << getpid() << "]: " << "First message from grandchild process" << std::endl);

          // Change the working directory
          if (working_dir_ && strcmp(working_dir_, ""))
          {
            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Chdir to working directory \"" << working_dir_ << "\"" << std::endl);
            if (chdir(working_dir_))
            {
              std::cerr << "[PID " << getpid() << "]: " << "Error changing working directory to " << working_dir_ << ": " << strerror(errno) << std::endl;
              _exit(EXIT_FAILURE);
            }
          }

          // Create the actual argument vector. We have to respect whether we
          // want to use a terminal emulator here, as using a terminal emulator
          // will result in additional arguments. Thus, the argument vector will
          // look like this:
          //
          //    Without terminal emulator:
          //        [0]: Process name
          //        [1]: Process arg 0
          //        [2]: Process arg 1
          //             ...
          //
          //    With terminal emulator:
          //        [0]: terminal emulator command
          //        [1]: terminal emulator arg 0
          //             ...
          //      [n+1]: terminal emulator arg n
          //      [n+2]: ecal_process_stub
          //      [n+3]: fifo name
          //      [n+4]: lockfile name
          //      [n+5]: Process name
          //      [n+6]: Process arg 0
          //      [n+7]: Process arg 1
          //             ...
          //
          // In order to make our life easier and prevent strings to go out of
          // scope, we store everything in an std::vector and create the C-style
          // argv (nullptr-terminated!) from that.

          std::vector<std::string> argv;

          std::vector<std::string> proc_argv = EcalUtils::CommandLine::ToArgv(std::string(proc_args_));

          if (!use_terminal_emulator)
          {
            argv.reserve(proc_argv.size() + 1);
          }
          else
          {
            std::vector<std::string> terminal_emulator_argv = EcalUtils::CommandLine::ToArgv(std::string(terminal_emulator_command));
            argv.reserve(terminal_emulator_argv.size() + 4 + proc_argv.size());
            for (const std::string& arg : terminal_emulator_argv)
            {
              argv.push_back(arg);
            }
            argv.push_back("ecal_process_stub");
            argv.push_back(fifo_name);
            argv.push_back(lockfile_name);
          }

          argv.push_back(proc_name_);
          for (const std::string& arg : proc_argv)
          {
            argv.push_back(arg);
          }

          // create C-style argv
          const char** c_argv = new const char*[argv.size() + 1];
          for (size_t i = 0; i < argv.size(); i++)
          {
            c_argv[i] = argv[i].c_str();
          }
          c_argv[argv.size()] = nullptr;

          // Create and lock the lockfile. The lockfile will automaticall be
          // closed (and unlocked) when the execvp was successfull or the
          // process exits.
          // We have to do this BEFORE writing the FIFO, because after we have
          // written the PID to the FIFO, the main process will also attempt
          // to lock the lockfile.
          // This only applies when we are NOT using a terminal emulator. If we
          // are using a terminal emulator, the ecal_process_stub will do that
          // job for us and communicate with the main process.
          int lockfile_fd = 0;
          if (!use_terminal_emulator)
          {
            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Opening lockfile \"" << lockfile_name << "\"" << std::endl);
            lockfile_fd = open(lockfile_name.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, 0666);
            if (lockfile_fd)
            {
              STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Locking lockfile..." << std::endl);
              if (flock(lockfile_fd, LOCK_EX) == -1)
              {
                std::cerr << "[PID " << getpid() << "]: " << "Error locking lockfile \"" << lockfile_name << "\": " << strerror(errno) << std::endl;
              }
              else
              {
                STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Successfully gained lock!" << std::endl);
              }
            }
            else
            {
              std::cerr << "[PID " << getpid() << "]: " << "Error creating lockfile \"" << lockfile_name << "\": " << strerror(errno) << std::endl;
            }
          }

          // Open FIFO and send the current PID to the eCAL. The main process
          // will wait until we send the PID, here. It is important to know that
          // although we have a valid PID, we cannot know whether the execvp
          // will be successfull, yet. This is were the lockfile becomes
          // important, as it will implicitely be unlocked when execvp is
          // successfull.
          // This only applies when we are NOT using a terminal emulator. If we
          // are using a terminal emulator, the ecal_process_stub will do that
          // job for us and communicate with the main process.
          if (!use_terminal_emulator)
          {
            pid_t current_pid = getpid();

            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Opening FIFO \"" << fifo_name << "\"" << std::endl);
            int fifo_fd = open(fifo_name.c_str(), O_WRONLY);

            if(fifo_fd < 0)
            {
              std::cerr << "[PID " << getpid() << "]: " << "Error opening FIFO \"" << fifo_name << ": " << strerror(errno) << std::endl;
            }
            else
            {
              STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Writing PID " << current_pid << " to FIFO" << std::endl);
              if (write(fifo_fd, &current_pid, sizeof(current_pid)) >= 0)
              {
                STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Successfully written PID" << std::endl);
              }
              else
              {
                std::cerr << "[PID " << getpid() << "]: " << "Error writing PID " << current_pid << " to FIFO \"" << fifo_name << "\": " << strerror(errno) << std::endl;
              }
            }

            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Closing FIFO" << std::endl);
            close(fifo_fd);
          }

          // Now call execvp, which will replace this grandchild process by the
          // new one, if successfull. In that case, the lockfill (that we have
          // opened and locked in the no-terminal-emulator-case) will
          // automatically be closed and unlocked.
#ifndef NDEBUG
          STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Executing process \"" << c_argv[0] << "\" with parameters:" << std::endl);
          for (int i = 0; c_argv[i]; i++)
          {
            STD_COUT_DEBUG("  [" << i << "]: " << c_argv[i] << std::endl);
          }
#endif // !NDEBUG

          execvp(c_argv[0], (char**)c_argv);

          // ERROR! >> If we have ever reached this code, execvp has not
          // succeeded. We have to distict between the two paths:
          //
          //   Without terminal emulator:
          //     Loading the process executable has failed. We have already sent
          //     the PID to the main process, which is now trying to gain the
          //     lock to the lockfile. We now have to write an errorcode to the
          //     lockfile and release the lock.
          //
          //   With terminal emulator:
          //     Loading the terminal emulator itself has failed. The main
          //     process is still waiting for a PID written to the FIFO, which
          //     would normally be sent by the ecal_process_stub started by the
          //     terminal emulator. As starting the terminal emulator has
          //     failed, it's now our responsibility to send something through
          //     the pipe and weake up the main process.

          std::cerr << "[PID " << getpid() << "]: " << "Error executing the program " << c_argv[0] << ": " << strerror(errno) << std::endl;

          if (!use_terminal_emulator)
          {
            // When execvp failed, the lockfile remains open and locked. We can now write our error code to it.
            int errno_error = errno;
            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Writing error code " << errno_error << "to lockfile" << std::endl);

            if (write(lockfile_fd, &errno_error, sizeof(errno_error)) < 0)
            {
              std::cerr << "[PID " << getpid() << "]: " << "Error writing error code " << errno_error << " to lockfile \"" << lockfile_name << "\": " << strerror(errno) << std::endl;
            }
            else
            {
              STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Successfully written error code" << std::endl);
            }
          }
          else
          {
            // When starting the terminal emulator failed, we write 0 to the FIFO in order to tell the main process. It would otherwise wait forever.
            pid_t error_pid = 0;

            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Opening FIFO \"" << fifo_name << "\"" << std::endl);
            int fifo_fd = open(fifo_name.c_str(), O_WRONLY);

            if(fifo_fd < 0)
            {
              std::cerr << "[PID " << getpid() << "]: " << "Error opening FIFO \"" << fifo_name << ": " << strerror(errno) << std::endl;
            }
            else
            {
              STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Writing PID " << error_pid << " to FIFO to tell the main process that we failed executing the terminal emulator" << std::endl);
              if (write(fifo_fd, &error_pid, sizeof(error_pid)) <= 0)
              {
                std::cerr << "[PID " << getpid() << "]: " << "Error writing PID " << error_pid << " to FIFO \"" << fifo_name << "\": " << strerror(errno) << std::endl;
              }
              else
              {
                STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Writing PID successfull" << std::endl);
              }
            }

            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Closing FIFO" << std::endl);
            close(fifo_fd);
          }

          delete[] c_argv;

          STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Unlocking lockfile" << std::endl);
          if (flock(lockfile_fd, LOCK_UN) == -1)
          {
            std::cerr << "[PID " << getpid() << "]: " << "Error unlocking lockfile \"" << lockfile_name << "\": " << strerror(errno) << std::endl;
          }
          else
          {
            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Successfully unlocked lockfile!" << std::endl);
          }

          STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Process will now exit" << std::endl);
          _exit(EXIT_FAILURE);
        }
      }
      else
      {
        // --------------------------- Main process ----------------------------

        // This is the main process that has to continue normally.

        pid_t process_pid = 0;

        // Wait for the child process to finish, so it won't get zombified

        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Waiting for child (PID " << child_pid << ") to finish" << std::endl);
        pid_t wait_for_child_result = waitpid(child_pid, nullptr, 0);
        if ((wait_for_child_result < 1) || (wait_for_child_result == 0 /* should never happen*/))
        {
          std::cerr << "[PID " << getpid() << "]: " << "Error waiting for child to exit: " << strerror(errno) << std::endl;
        }
        else
        {
          STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Child process has finished." << std::endl);
        }


        // Open the FIFO and read the PID from it. This will block until we
        // actually receive the data, so we are dependent on some process to
        // actually write data to the FIFO.
        // It should be noted that even if we receive a valid PID (pid != 0), we
        // cannot know whether executing the process has already succeeded. We
        // only know wether the fork() has worked (or the ecal_process_stub has
        // started in case of a terminal emulator). Thus we have to lock the
        // lockfile and check if it contains an error number, once we gained the
        // lock.
        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Opening FIFO \"" << fifo_name << "\"" << std::endl);
        int fifo_fd = open(fifo_name.c_str(), O_RDWR, O_NONBLOCK);  // Open FIFO non-blocking requires to open it Read-Write, although we only want to read it.
        if(fifo_fd < 0)
        {
          std::cerr << "[PID " << getpid() << "]: " << "Error opening FIFO \"" << fifo_name << ": " << strerror(errno) << std::endl;
        }
        else
        {
          STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Successfully opened FIFO" << std::endl);

          struct timeval timeout;
          timeout.tv_sec = 5;
          timeout.tv_usec = 0;

          fd_set fifo_fd_set;
          FD_ZERO(&fifo_fd_set);
          FD_SET(fifo_fd, &fifo_fd_set);

          STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Waiting up to " << timeout.tv_sec << "s for PID in FIFO..." << std::endl);

          int ready = select(fifo_fd + 1, &fifo_fd_set, NULL, NULL, &timeout);

          if(ready == -1)
          {
            std::cerr << "[PID " << getpid() << "]: " << "Error waiting for PID in FIFO \"" << fifo_name << ": " << strerror(errno) << std::endl;
          }
          else if (ready == 0)
          {
            std::cerr << "[PID " << getpid() << "]: " << "Error: timeout occured while waiting for PID in FIFO \"" << fifo_name << "\"" << std::endl;
          }
          else
          {
            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Reading PID of new process..." << std::endl);

            if(read(fifo_fd, &process_pid, sizeof(process_pid)) >= 0)
            {
              STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Received PID " << process_pid << std::endl);
            }
            else
            {
              std::cerr << "[PID " << getpid() << "]: " << "Error reading PID from FIFO \"" << fifo_name << "\": " << strerror(errno) << std::endl;
            }
          }
        }
        close(fifo_fd);

        // Open the lockfile , lock it and read it. At this point, we can be
        // pretty sure that the file already exists, as the grandchild process
        // has created it BEFORE it wrote the PID to the FIFO. As the granchild
        // also locked the file, this will block until:
        //   - The execvp call has succeded and the lockfile is closed
        //     implicitely
        //  or
        //   - The execvp call failed and the lockfile is closed by the
        //     granchild process exiting. In this case, the granchild also
        //     abused the lockfile to write its errno to it, so we can read it.

        if (process_pid > 0)
        {
          STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Opening lockfile \"" << lockfile_name << "\"" << std::endl);
          int lockfile_fd = open(lockfile_name.c_str(), O_RDWR, 0666);
          if (lockfile_fd < 0)
          {
            std::cerr << "[PID " << getpid() << "]: " << "Error opening lockfile \"" << lockfile_name << "\": " << strerror(errno) << std::endl;
          }
          else
          {
            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Locking lockfile..." << std::endl);
            if (flock(lockfile_fd, LOCK_EX) == -1)
            {
              std::cerr << "[PID " << getpid() << "]: " << "Error locking lockfile \"" << lockfile_name << "\": " << strerror(errno) << std::endl;
            }
            else
            {
              STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Successfully gained lock!" << std::endl);
            }
            int errno_error = 0;

            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Reading lockfile" << std::endl);

            int bytes_read = read(lockfile_fd, &errno_error, sizeof(errno_error));
            if (bytes_read < 0)
            {
              std::cerr << "[PID " << getpid() << "]: " << "Error reading lockfile \"" << lockfile_name << "\": " << strerror(errno) << std::endl;
            }
            else if (bytes_read == 0)
            {
              STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Lockfile does not contain data => No error" << std::endl);
            }
            else
            {
              STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Lockfile contains error code " << errno_error << std::endl);
            }

            if (errno_error)
            {
              std::cerr << "Failed to execute process: " << strerror(errno_error) << std::endl;
              process_pid = 0;
            }
            STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Closing lockfile" << std::endl);
            close(lockfile_fd);
          }
        }


        if (block_ && (process_pid > 0))
        {
          waitpid(process_pid, nullptr, 0);
        }

        // Remove file system artefacts
        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Unlinking fifo" << std::endl);
        if (unlink(fifo_name.c_str()))
          std::cerr << "[PID " << getpid() << "]: " << "Error unlinking file \"" << fifo_name << "\": " << strerror(errno) << std::endl;
        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Unlinking lockfile." << std::endl);
        if (unlink(lockfile_name.c_str()))
          std::cerr << "[PID " << getpid() << "]: " << "Error unlinking file \"" << lockfile_name << "\": " << strerror(errno) << std::endl;

        STD_COUT_DEBUG("[PID " << getpid() << "]: " << "Finished" << std::endl);

        return process_pid;
      }
    }

    bool StopProcess(const char* proc_name_)
    {
      std::string command = "killall -e " + std::string(proc_name_);
      return (system(command.c_str()) == 0);
    }

    bool StopProcess(const int proc_id_)
    {
      if (proc_id_ <= 0) return false;

      bool ret_val = (kill(proc_id_, SIGTERM) == 0);

      return ret_val;
    }
  }
}

#endif /* ECAL_OS_LINUX */

#undef STD_COUT_DEBUG
