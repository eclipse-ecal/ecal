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

#include <vector>
#include <string>
#include <sys_error.h>

#include <memory>
#include <ecalsys/ecal_sys.h>

#include <ecal/ecal_client.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/pb/sys/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  namespace sys
  {
    namespace command
    {
      Error ToTaskList(const std::shared_ptr<EcalSys> ecalsys_instance, const std::vector<std::string>& argv, std::list<std::shared_ptr<EcalSysTask>>& output_tasklist);

      Error ToRunnerList(const std::shared_ptr<EcalSys> ecalsys_instance, const std::vector<std::string>& argv, std::list<std::shared_ptr<EcalSysRunner>>& output_runnerlist);

      Error ToGroupList(const std::shared_ptr<EcalSys> ecalsys_instance, const std::vector<std::string>& argv, std::list<std::shared_ptr<TaskGroup>>& output_grouplist);

      Error GetCompleteTaskList(const eCAL::pb::sys::State& state_pb, std::list<std::shared_ptr<EcalSysTask>>& output_tasklist);

      Error GetCompleteGroupList(const eCAL::pb::sys::State& state_pb, std::list<std::shared_ptr<TaskGroup>>& output_grouplist);

      Error ToTaskList(const eCAL::pb::sys::State& state_pb, const std::vector<std::string>& argv, std::list<std::shared_ptr<EcalSysTask>>& output_tasklist);

      Error ToGroupList(const eCAL::pb::sys::State& state_pb, const std::vector<std::string>& argv, std::list<std::shared_ptr<TaskGroup>>& output_grouplist);
    }
  }
}