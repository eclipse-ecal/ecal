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

#include <ecal_utils/string.h>

#include <thread>

#include <clocale>
#include <locale>
#include <iostream>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {

      std::string Sleep::Usage() const
      {
        return "<Seconds>";
      }

      std::string Sleep::Help() const
      {
        return "Sleeps for the given amount of time. Usefull when automatically piping input to eCAL Rec via stdin.";
      }

      std::string Sleep::Example() const
      {
        return "3.5";
      }

      eCAL::rec::Error Sleep::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& /*rec_server_instance*/, const std::vector<std::string>& argv) const
      {
        return Execute(argv);
      }

      eCAL::rec::Error Sleep::Execute(const std::string& /*hostname*/, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& /*remote_rec_server_service*/, const std::vector<std::string>& argv) const
      {
        return Execute(argv);
      }

      eCAL::rec::Error Sleep::Execute(const std::vector<std::string>& argv) const
      {
        if (argv.size() > 1)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));
        else if (argv.empty())
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, "Number of seconds is required");

        std::chrono::duration<double> time_to_sleep(0);

        {
          eCAL::rec::Error error = parseTime(argv[0], time_to_sleep);
          if (error)
            return error;
        }

        return Execute(time_to_sleep);
      }

      eCAL::rec::Error Sleep::Execute(const std::chrono::duration<double> time_to_sleep) const
      {
        if (time_to_sleep < std::chrono::nanoseconds(0))
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, std::string("Time to sleep must be greater than 0"));
        }

        std::cout << "Sleeping for " << std::chrono::duration_cast<std::chrono::duration<double>>(time_to_sleep).count() << "s..." << std::endl;

        sleeper.SleepFor(std::chrono::duration_cast<std::chrono::nanoseconds>(time_to_sleep));

        return eCAL::rec::Error::ErrorCode::OK;
      }

      void Sleep::Interrupt() const
      {
        sleeper.InterruptSleeping();
      }

      eCAL::rec::Error Sleep::parseTime(const std::string& time_string, std::chrono::duration<double>& time_out)
      {
        char decimal_point = std::localeconv()->decimal_point[0]; // Locale decimal point for making float strings locale independent

        try
        {
          std::string seconds_string = time_string;
          std::replace(seconds_string.begin(), seconds_string.end(), '.', decimal_point);

          double seconds_to_sleep = std::stod(seconds_string);

          time_out = std::chrono::duration<double>(seconds_to_sleep);
        }
        catch (const std::exception&)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::PARAMETER_ERROR, std::string("\"") + time_string + "\" is not a number");
        }

        return eCAL::rec::Error::ErrorCode::OK;
      }
    }
  }
}
