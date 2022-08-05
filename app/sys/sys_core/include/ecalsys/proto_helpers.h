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
#include <app/pb/sys/state.pb.h>
#include <core/pb/process.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "task/task_group.h"
#include "task/ecal_sys_task.h"
#include <ecalsys/ecal_sys.h>
#include <ecalsys/task/task_state.h>

#include <ecal/ecal_process_mode.h>

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
      TaskState                    FromProtobuf(const eCAL::pb::ProcessState&      task_state_pb);
      TaskState                    FromProtobuf(const eCAL::pb::sys::ProcessState& task_state_pb);
      std::shared_ptr<EcalSysTask> FromProtobuf(const eCAL::pb::sys::State::Task&  task_pb);
      std::shared_ptr<TaskGroup>   FromProtobuf(const eCAL::pb::sys::State::Group& task_group_pb);

      ///////////////////////////////
      // To Protobuf
      ///////////////////////////////
      void                 ToProtobuf(eCAL::pb::sys::State& state_pb, const EcalSys& ecalsys);
      eCAL::pb::sys::State ToProtobuf(const EcalSys&        ecalsys);
    }
  }
}
