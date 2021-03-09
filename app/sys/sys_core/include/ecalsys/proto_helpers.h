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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/pb/sys/state.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "task/task_group.h"
#include "task/ecal_sys_task.h"
#include <ecalsys/ecal_sys.h>
#include <ecalsys/task/task_state.h>

#include <ecal/ecal_process_mode.h>

#include <list>
#include <memory>

namespace eCAL
{
  namespace sys
  {
    namespace proto_helpers
    {
      ///////////////////////////////
      // From Protobuf
      ///////////////////////////////

      void FromProtobuf(const eCAL::pb::ProcessState&         task_state_pb,    TaskState&                    task_state);
      void FromProtobuf(const eCAL::pb::sys::State::Task&     task_pb,          std::shared_ptr<EcalSysTask>& task);
      void FromProtobuf(const eCAL::pb::sys::State::Group&    task_group_pb,    std::shared_ptr<TaskGroup>&   task_group);

      TaskState                    FromProtobuf(const eCAL::pb::ProcessState&         task_state_pb);
      std::shared_ptr<EcalSysTask> FromProtobuf(const eCAL::pb::sys::State::Task&     task_pb);
      std::shared_ptr<TaskGroup>   FromProtobuf(const eCAL::pb::sys::State::Group&    task_group_pb);

      ///////////////////////////////
      // To Protobuf
      ///////////////////////////////

      void ToProtobuf(eCAL::pb::ProcessState&         task_state_pb,    const TaskState&                    task_state);
      void ToProtobuf(eCAL::pb::sys::State::Task&     task_pb,          const std::shared_ptr<EcalSysTask>& task);
      void ToProtobuf(eCAL::pb::sys::State::Group&    task_group_pb,    const std::shared_ptr<TaskGroup>&   task_group);
      void ToProtobuf(eCAL::pb::sys::State&           state_pb,         const EcalSys&                      ecalsys);

      eCAL::pb::ProcessState         ToProtobuf(const TaskState&                    task_state);
      eCAL::pb::sys::State::Task     ToProtobuf(const std::shared_ptr<EcalSysTask>& task);
      eCAL::pb::sys::State::Group    ToProtobuf(const std::shared_ptr<TaskGroup>&   task_group);
      eCAL::pb::sys::State           ToProtobuf(const EcalSys&                      ecalsys);
    }
  }
}
