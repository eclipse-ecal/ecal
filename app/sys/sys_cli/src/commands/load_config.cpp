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

#include "load_config.h"

#include <ecal_utils/string.h>

namespace eCAL
{
  namespace sys
  {
    namespace command
    {

      std::string LoadConfig::Usage() const
      {
        return "<Path>";
      }

      std::string LoadConfig::Help() const
      {
        return "Loads an eCAL Sys configuration file.";
      }

      std::string LoadConfig::Example() const
      {
        return "\"/path/to/config.ecalsys\"";
      }

      eCAL::sys::Error LoadConfig::Execute(const std::shared_ptr<EcalSys>& ecalsys_instance, const std::vector<std::string>& argv) const
      {
        if (argv.empty())
          return eCAL::sys::Error::ErrorCode::PARAMETER_ERROR;
        else if (argv.size() > 1)
          return Error(Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        try
        {
          bool success = ecalsys_instance->LoadConfig(argv[0], false);
          if (!success)
            return Error(Error::ErrorCode::RESOURCE_UNAVAILABLE, argv[0]);
        }
        catch (const std::exception& e)
        {
          return Error(Error::ErrorCode::RESOURCE_UNAVAILABLE, e.what());
        }

        return Error::ErrorCode::OK;
      }

      eCAL::sys::Error LoadConfig::Execute(const std::string& /*hostname*/, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::sys::Service>>& /*remote_ecalsys_service*/, const std::vector<std::string>& /*argv*/) const
      {
        return eCAL::sys::Error::ErrorCode::COMMAND_NOT_AVAILABLE_IN_REMOTE_MODE;
      }
    }
  }
}