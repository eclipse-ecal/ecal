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

#include "update_from_cloud.h"
#include <ecal_utils/ecal_utils.h>

namespace eCAL
{
  namespace sys
  {
    namespace command
    {

      std::string UpdateFromCloud::Usage() const
      {
        return "";
      }

      std::string UpdateFromCloud::Help() const
      {
        return "Updates the state of all (eCAL-) tasks that are visible, even if they have not been started by this eCAL Sys instance.";
      }

      std::string UpdateFromCloud::Example() const
      {
        return "";
      }

      eCAL::sys::Error UpdateFromCloud::Execute(const std::shared_ptr<EcalSys>& ecalsys_instance, const std::vector<std::string>& argv) const
      {
        if (!argv.empty())
        {
          return Error(Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));
        }

        ecalsys_instance->UpdateFromCloud();

        return Error::ErrorCode::OK;
      }

      eCAL::sys::Error UpdateFromCloud::Execute(const std::string& /*hostname*/, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::sys::Service>>& /*remote_ecalsys_service*/, const std::vector<std::string>& /*argv*/) const
      {
        return eCAL::sys::Error::ErrorCode::COMMAND_NOT_AVAILABLE_IN_REMOTE_MODE;
      }

    }
  }
}