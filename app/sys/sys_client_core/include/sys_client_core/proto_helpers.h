/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
#include <ecal/app/pb/sys/client_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "runner.h"
#include "task.h"

#include <ecal/process_mode.h>

#include <vector>

namespace eCAL
{
  namespace sys_client
  {
    namespace proto_helpers
    {
      ///////////////////////////////
      // From Protobuf
      ///////////////////////////////

      void FromProtobuf(const eCAL::pb::sys_client::Task&                task_pb,              eCAL::sys_client::Task&            task);
      void FromProtobuf(const eCAL::pb::sys_client::Runner&              runner_pb,            eCAL::sys_client::Runner&          runner);
      void FromProtobuf(const eCAL::pb::sys_client::WindowMode&          window_mode_pb,       eCAL::Process::eStartMode&           window_mode);
      void FromProtobuf(const eCAL::pb::sys_client::StartTaskParameters& start_task_param_pb,  StartTaskParameters&               start_task_param);
      void FromProtobuf(const eCAL::pb::sys_client::StartTaskRequest&    start_task_list_pb,   std::vector<StartTaskParameters>&  start_task_list);
      void FromProtobuf(const eCAL::pb::sys_client::StopTaskParameters&  stop_task_param_pb,   StopTaskParameters&                stop_task_param);
      void FromProtobuf(const eCAL::pb::sys_client::StopTaskRequest&     stop_task_list_pb,    std::vector<StopTaskParameters>&   stop_task_list);
      void FromProtobuf(const eCAL::pb::sys_client::TaskList&            task_list_pb,         std::vector<Task>&                 task_list);
      void FromProtobuf(const eCAL::pb::sys_client::ProcessIdList&       pid_list_pb,          std::vector<int32_t>&              pid_list);
      void FromProtobuf(const eCAL::pb::sys_client::MatchTaskResponse&   map_task_response_pb, std::vector<std::vector<int32_t>>& map_task_response);

      eCAL::sys_client::Task            FromProtobuf(const eCAL::pb::sys_client::Task&                task_pb);
      eCAL::sys_client::Runner          FromProtobuf(const eCAL::pb::sys_client::Runner&              runner_pb);
      eCAL::Process::eStartMode           FromProtobuf(const eCAL::pb::sys_client::WindowMode&          window_mode_pb);
      StartTaskParameters               FromProtobuf(const eCAL::pb::sys_client::StartTaskParameters& start_task_param_pb);
      std::vector<StartTaskParameters>  FromProtobuf(const eCAL::pb::sys_client::StartTaskRequest&    start_task_list_pb);
      StopTaskParameters                FromProtobuf(const eCAL::pb::sys_client::StopTaskParameters&  stop_task_param_pb);
      std::vector<StopTaskParameters>   FromProtobuf(const eCAL::pb::sys_client::StopTaskRequest&     stop_task_list_pb);
      std::vector<Task>                 FromProtobuf(const eCAL::pb::sys_client::TaskList&            task_list_pb);
      std::vector<int32_t>              FromProtobuf(const eCAL::pb::sys_client::ProcessIdList&       pid_list_pb);
      std::vector<std::vector<int32_t>> FromProtobuf(const eCAL::pb::sys_client::MatchTaskResponse&   map_task_response_pb);

      ///////////////////////////////
      // To Protobuf
      ///////////////////////////////

      void ToProtobuf(eCAL::pb::sys_client::Task&                task_pb,              const eCAL::sys_client::Task&            task);
      void ToProtobuf(eCAL::pb::sys_client::Runner&              runner_pb,            const eCAL::sys_client::Runner&          runner);
      void ToProtobuf(eCAL::pb::sys_client::WindowMode&          window_mode_pb,       const eCAL::Process::eStartMode            window_mode);
      void ToProtobuf(eCAL::pb::sys_client::StartTaskParameters& start_task_param_pb,  const StartTaskParameters&               start_task_param);
      void ToProtobuf(eCAL::pb::sys_client::StartTaskRequest&    start_task_list_pb,   const std::vector<StartTaskParameters>&  start_task_list);
      void ToProtobuf(eCAL::pb::sys_client::StopTaskParameters&  stop_task_param_pb,   const StopTaskParameters&                stop_task_param);
      void ToProtobuf(eCAL::pb::sys_client::StopTaskRequest&     stop_task_list_pb,    const std::vector<StopTaskParameters>&   stop_task_list);
      void ToProtobuf(eCAL::pb::sys_client::TaskList&            task_list_pb,         const std::vector<Task>&                 task_list);
      void ToProtobuf(eCAL::pb::sys_client::ProcessIdList&       pid_list_pb,          const std::vector<int32_t>&              pid_list);
      void ToProtobuf(eCAL::pb::sys_client::MatchTaskResponse&   map_task_response_pb, const std::vector<std::vector<int32_t>>& map_task_response);

      eCAL::pb::sys_client::Task                ToProtobuf(const eCAL::sys_client::Task&                task);
      eCAL::pb::sys_client::Runner              ToProtobuf(const eCAL::sys_client::Runner&              runner);
      eCAL::pb::sys_client::WindowMode          ToProtobuf(const eCAL::Process::eStartMode                window_mode);
      eCAL::pb::sys_client::StartTaskParameters ToProtobuf(const StartTaskParameters&                   start_task_param);
      eCAL::pb::sys_client::StartTaskRequest    ToProtobuf(const std::vector<StartTaskParameters>&      start_task_list);
      eCAL::pb::sys_client::StopTaskParameters  ToProtobuf(const StopTaskParameters&                    stop_task_param);
      eCAL::pb::sys_client::StopTaskRequest     ToProtobuf(const std::vector<StopTaskParameters>&       stop_task_list);
      eCAL::pb::sys_client::TaskList            ToProtobuf(const std::vector<Task>&                     task_list_pb);
      eCAL::pb::sys_client::ProcessIdList       ToProtobuf(const std::vector<int32_t>&                  pid_list_pb);
      eCAL::pb::sys_client::MatchTaskResponse   ToProtobuf(const std::vector<std::vector<int32_t>>&     map_task_response_pb);
    }
  }
}
