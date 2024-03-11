/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2023 Continental Corporation
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

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <ecal/ecal.h> // IWYU pragma: keep

#include "publisher.h"
#include "subscriber.h"

#include <exception>
#include <iostream>
#include <iterator>
#include <ratio>
#include <string>
#include <thread>
#include <vector>

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif


void printUsage(const std::string& arg0)
{
  std::cout << "Usage:" << std::endl;
  std::cout << "  " << arg0 << " pub <topic_name> <frequency_hz> <payload_size_bytes> [options]" << std::endl;
  std::cout << "or:" << std::endl;
  std::cout << "  " << arg0 << " sub <topic_name> [callback_delay_ms] [options]" << std::endl;
  std::cout << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -q, --quiet:     Do not print any output" << std::endl;
  std::cout << "  -v, --verbose:   Print all measured times for all messages" << std::endl;
  std::cout << "      --busy-wait: Busy wait when receiving messages (i.e. burn CPU). For subscribers only." << std::endl;
  std::cout << "      --hickup <after_ms> <delay_ms>: Further delay a single callback. For subscribers only." << std::endl;

}

int main(int argc, char** argv)
{
#ifdef WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif // WIN32

  bool quiet_arg           = false;
  bool verbose_print_times = false;
  bool busy_wait_arg       = false;

  bool hickup_arg          = false;
  std::chrono::steady_clock::duration hickup_time (0);
  std::chrono::steady_clock::duration hickup_delay(0);

  // convert argc, argv to vector of strings
  std::vector<std::string> args;
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i)
  {
    args.emplace_back(argv[i]);
  }

  // Check for -h / --help
  if (args.size() < 2
      || std::find(args.begin(), args.end(), "-h") != args.end()
      || std::find(args.begin(), args.end(), "--help") != args.end())
  {
    printUsage(args[0]);
    return 0;
  }

  // find "--hickup" argument and remove it from args
  {
    auto hickup_arg_it = std::find(args.begin(), args.end(), "--hickup");
    if (hickup_arg_it != args.end())
    {
      hickup_arg = true;

      // Check if there are enough arguments for the time and delay after the hickup_arg_it and parse those as doubles
      if (args.size() < static_cast<size_t>(std::distance(args.begin(), hickup_arg_it) + 3))
      {
        std::cerr << "Invalid number of parameters after --hickup" << std::endl;
        printUsage(args[0]);
        return 1;
      }
      else
      {
        try
        {
          // Parse the next two arguments as double 
          const double hickup_time_ms  = std::stod(*(std::next(hickup_arg_it, 1)));
          const double hickup_delay_ms = std::stod(*(std::next(hickup_arg_it, 2)));

          hickup_time  = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double, std::milli>(hickup_time_ms));
          hickup_delay = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double, std::milli>(hickup_delay_ms));
        }
        catch (const std::exception& e)
        {
          std::cerr << "Failed parsing parameters after --hickup: " << e.what() << std::endl;
          printUsage(args[0]);
          return 1;
        }

        // Remove all 3 parameters
        args.erase(hickup_arg_it, std::next(hickup_arg_it, 3));
      }
    }
  }

  // find "--quiet" argument and remove it from args
  {
    auto quiet_arg_it = std::find(args.begin(), args.end(), "--quiet");
    if (quiet_arg_it != args.end())
    {
      quiet_arg = true;
      args.erase(quiet_arg_it);
    }
  }

  // find "-q" argument and remove it from args
  {
    auto q_arg_it = std::find(args.begin(), args.end(), "-q");
    if (q_arg_it != args.end())
    {
      quiet_arg = true;
      args.erase(q_arg_it);
    }
  }
  
  // find "--verbose" argument and remove it from args
  {
    auto verbose_arg_it = std::find(args.begin(), args.end(), "--verbose");
    if (verbose_arg_it != args.end())
    {
      verbose_print_times = true;
      args.erase(verbose_arg_it);
    }
  }

  // find "-v" argument and remove it from args
  {
    auto v_arg_it = std::find(args.begin(), args.end(), "-v");
    if (v_arg_it != args.end())
    {
      verbose_print_times = true;
      args.erase(v_arg_it);
    }
  }

  // Validate quite and verbose args
  if (quiet_arg && verbose_print_times)
  {
    std::cerr << "Invalid arguments: Cannot use \"quiet\" and \"verbose\" simultaneously" << std::endl;
    printUsage(argv[0]);
    return 1;
  }

  // find "--busy-wait" argument and remove it from args
  {
    auto busy_wait_arg_it = std::find(args.begin(), args.end(), "--busy-wait");
    if (busy_wait_arg_it != args.end())
    {
      busy_wait_arg = true;
      args.erase(busy_wait_arg_it);
    }
  }

  if (args[1] == "pub")
  {
    if (args.size() != 5)
    {
      std::cerr << "Invalid number of parameters" << std::endl;
      printUsage(args[0]);
      return 1;
    }
    const std::string        topic_name         = args[2];
    const double             frequency_hz       = std::stod(args[3]);
    const unsigned long long payload_size_bytes = std::stoull(args[4]);

    // Initialize eCAL
    eCAL::Initialize(argc, argv, "ecal-perftool");
    eCAL::Util::EnableLoopback(true);
    
    const Publisher publisher(topic_name, frequency_hz, payload_size_bytes, quiet_arg, verbose_print_times);
    
    // Just don't exit
    while (eCAL::Ok())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // finalize eCAL API
    eCAL::Finalize();
  }
  else if (args[1] == "sub")
  {
    if (args.size() < 3 || args.size() > 4)
    {
      std::cerr << "Invalid number of parameters" << std::endl;
      printUsage(args[0]);
      return 1;
    }

    const std::string               topic_name = args[2];
    const std::chrono::milliseconds callback_delay((args.size() >= 4 ? std::stoull(args[3]) : 0));

    
    // Initialize eCAL
    eCAL::Initialize(argc, argv, "ecal-perftool");
    eCAL::Util::EnableLoopback(true);
    
    const Subscriber subscriber(topic_name, callback_delay, busy_wait_arg, hickup_arg, hickup_time, hickup_delay, quiet_arg, verbose_print_times);

    // Just don't exit
    while (eCAL::Ok())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // finalize eCAL API
    eCAL::Finalize();
  }
  else
  {
    std::cerr << "Invalid parameter: " << args[1] << std::endl;
    printUsage(args[0]);
    return 1;
  }

  return 0;
}
