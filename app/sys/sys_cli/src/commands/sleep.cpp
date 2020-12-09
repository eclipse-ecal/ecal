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

#include "sleep.h"
#include "helpers.h"

#include <ecal_utils/string.h>

#include <thread>

#include <clocale>
#include <locale>
#include <iostream>

namespace eCAL
{
  namespace sys
  {
    namespace command
    {

      std::string Sleep::Usage() const
      {
        return "<seconds>";
      }

      std::string Sleep::Help() const
      {
        return "Sleeps for the given amount of time. Usefull when automatically piping input to eCAL Sys via stdin.";
      }

      std::string Sleep::Example() const
      {
        return "3.5";
      }

      eCAL::sys::Error Sleep::Execute(const std::shared_ptr<EcalSys>& /*ecalsys_instance*/, const std::vector<std::string>& argv) const
      {
        return Execute(argv);
      }

      eCAL::sys::Error Sleep::Execute(const std::string& /*hostname*/, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::sys::Service>>& /*remote_ecalsys_service*/, const std::vector<std::string>& argv) const
      {
        return Execute(argv);
      }

      eCAL::sys::Error Sleep::Execute(const std::vector<std::string>& argv) const
      {
        if (argv.size() > 1)
          return Error(Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));
        else if (argv.empty())
          return Error(Error::ErrorCode::PARAMETER_ERROR, "Number of seconds is required");

        std::chrono::nanoseconds time_to_sleep(0);

        char decimal_point = std::localeconv()->decimal_point[0]; // Locale decimal point for making float strings locale independent

        try
        {
          std::string seconds_string = argv[0];
          std::replace(seconds_string.begin(), seconds_string.end(), '.', decimal_point);

          double seconds_to_sleep = std::stod(seconds_string);

          time_to_sleep = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(seconds_to_sleep));
        }
        catch (const std::exception&)
        {
          return Error(Error::ErrorCode::PARAMETER_ERROR, std::string("\"") + argv[0] + "\" is not a number");
        }

        if (time_to_sleep < std::chrono::nanoseconds(0))
        {
          return Error(Error::ErrorCode::PARAMETER_ERROR, std::string("Time to sleep must be greater than 0"));
        }

        std::cout << "Sleeping for " << std::chrono::duration_cast<std::chrono::duration<double>>(time_to_sleep).count() << "s..." << std::endl;

        std::this_thread::sleep_for(time_to_sleep);

        return Error::ErrorCode::OK;
      }

    }
  }
}
