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

#pragma once

#include "command.h"

namespace eCAL
{
  namespace rec_cli
  {
    namespace command
    {
      class TEMPLATE : public Command
      {
      public:
        std::string Usage()    const override;
        std::string Help()     const override;
        std::string Example()  const override;

        eCAL::rec::Error Execute(const std::shared_ptr<eCAL::rec_server::RecServer>& rec_server_instance, const std::vector<std::string>& argv) const override;
        eCAL::rec::Error Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService>>& remote_rec_server_service, const std::vector<std::string>& argv) const override;
      };
    }
  }
}