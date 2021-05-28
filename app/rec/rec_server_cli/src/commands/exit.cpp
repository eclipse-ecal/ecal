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

#include "exit.h"

#include <ecal_utils/string.h>

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {

      std::string Exit::Usage() const
      {
        return "";
      }

      std::string Exit::Help() const
      {
        return "Quit eCAL Rec";
      }

      std::string Exit::Example() const
      {
        return "";
      }

      eCAL::rec::Error Exit::Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& /*rec_server_instance*/, const std::vector<std::string>& /*argv*/) const
      {
        return eCAL::rec::Error::ErrorCode::OK;
      }

      eCAL::rec::Error Exit::Execute(const std::string& /*hostname*/, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& /*remote_rec_server_service*/, const std::vector<std::string>& /*argv*/) const
      {
        return eCAL::rec::Error::ErrorCode::OK;
      }
    }
  }
}
