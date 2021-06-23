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

#include "save_config.h"

#include <rec_server_core/proto_helpers.h>

#include <ecal_utils/string.h>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {

      std::string SaveConfig::Usage() const
      {
        return "<Path>";
      }

      std::string SaveConfig::Help() const
      {
        return "Saves the current configuration to a file. Not available in remote-control mode.";
      }

      std::string SaveConfig::Example() const
      {
        return "\"/path/to/config.ecalrec\"";
      }

      eCAL::rec::Error SaveConfig::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const
      {
        // Arg check
        if (argv.empty())
          return eCAL::rec::Error::ErrorCode::PARAMETER_ERROR;
        else if (argv.size() > 1)
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));

        bool success = rec_server_instance->SaveConfigToFile(argv[0]);

        if (!success)
        {
          return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::RESOURCE_UNAVAILABLE, "Failed to save config to \"" + argv[0] + "\"");
        }
        else
        {
          return eCAL::rec::Error::ErrorCode::OK;
        }
      }

      eCAL::rec::Error SaveConfig::Execute(const std::string& /*hostname*/, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& /*remote_rec_server_service*/, const std::vector<std::string>& /*argv*/) const
      {
        return eCAL::rec::Error(eCAL::rec::Error::ErrorCode::COMMAND_NOT_AVAILABLE_IN_REMOTE_MODE);
      }
    }
  }
}