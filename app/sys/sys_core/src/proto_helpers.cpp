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

#include <ecalsys/proto_helpers.h>

#include <sys_client_core/proto_helpers.h>

namespace eCAL
{
  namespace sys
  {
    namespace proto_helpers
    {
      ///////////////////////////////
      // From Protobuf
      ///////////////////////////////

      void FromProtobuf(const eCAL::pb::sys::ProcessState&    task_state_pb,    TaskState&                    task_state)
      {
        switch (task_state_pb.severity())
        {
        case eCAL::pb::sys::eProcessSeverity::proc_sev_unknown:
          task_state.severity = eCAL::Process::eSeverity::unknown;
          break;
        case eCAL::pb::sys::eProcessSeverity::proc_sev_healthy:
          task_state.severity = eCAL::Process::eSeverity::healthy;
          break;
        case eCAL::pb::sys::eProcessSeverity::proc_sev_warning:
          task_state.severity = eCAL::Process::eSeverity::warning;
          break;
        case eCAL::pb::sys::eProcessSeverity::proc_sev_critical:
          task_state.severity = eCAL::Process::eSeverity::critical;
          break;
        case eCAL::pb::sys::eProcessSeverity::proc_sev_failed:
          task_state.severity = eCAL::Process::eSeverity::failed;
          break;
        default:
          task_state.severity = eCAL::Process::eSeverity::unknown;
          break;
        }

        switch (task_state_pb.severity_level())
        {
        case eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level1:
          task_state.severity_level = eCAL::Process::eSeverityLevel::level1;
          break;
        case eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level2:
          task_state.severity_level = eCAL::Process::eSeverityLevel::level2;
          break;
        case eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level3:
          task_state.severity_level = eCAL::Process::eSeverityLevel::level3;
          break;
        case eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level4:
          task_state.severity_level = eCAL::Process::eSeverityLevel::level4;
          break;
        case eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level5:
          task_state.severity_level = eCAL::Process::eSeverityLevel::level5;
          break;
        default:
          task_state.severity_level = eCAL::Process::eSeverityLevel::level1;
          break;
        }

        task_state.info = task_state_pb.info();
      }

      void FromProtobuf(const eCAL::pb::sys::State::Task&     task_pb,          std::shared_ptr<EcalSysTask>& task)
      {
        task->SetId(task_pb.id());
        task->SetName(task_pb.name());
        task->SetTarget(task_pb.target_host());
        task->SetMonitoringTaskState(FromProtobuf(task_pb.state()));

        std::vector<int32_t> process_ids;
        for(int32_t process_id : task_pb.process_ids())
          process_ids.push_back(process_id);
        task->SetPids(process_ids);

        task->SetLaunchOrder(task_pb.launch_order());
        task->SetAlgoPath(task_pb.path());
        task->SetWorkingDir(task_pb.working_dir());
        task->SetCommandLineArguments(task_pb.command_line());
        task->SetVisibility(eCAL::sys_client::proto_helpers::FromProtobuf(task_pb.window_mode()));
        task->SetTimeoutAfterStart(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(task_pb.waiting_time_secs())));
        task->SetMonitoringEnabled(task_pb.monitoring_enabled());
        task->SetMonitoringTaskState(FromProtobuf(task_pb.state()));

        auto restart_at_severity = FromProtobuf(task_pb.restart_by_severity());
        if (restart_at_severity.severity != eCAL::Process::eSeverity::unknown)
        {
          task->SetRestartBySeverityEnabled(true);
          task->SetRestartAtSeverity(restart_at_severity);
        }
        else
        {
          task->SetRestartBySeverityEnabled(false);
        }
        task->SetHostStartedOn(task_pb.current_host());
      }

      void FromProtobuf(const eCAL::pb::sys::State::Group&    task_group_pb,    std::shared_ptr<TaskGroup>&   task_group)
      {
        // TODO: This conversion faked, it does not represent the group is was created from.

        std::shared_ptr<TaskGroup::GroupState> dummy_state1 = std::make_shared<TaskGroup::GroupState>();
        std::shared_ptr<TaskGroup::GroupState> dummy_state2 = std::make_shared<TaskGroup::GroupState>();

        // We create 2 faked states that appear the same, so always one of them will be available.
        dummy_state1->SetName(task_group_pb.state());
        dummy_state2->SetName(task_group_pb.state());
        dummy_state1->SetColor(TaskGroup::GroupState::Color(
                static_cast<uint8_t>(task_group_pb.colour().r())
                , static_cast<uint8_t>(task_group_pb.colour().g())
                , static_cast<uint8_t>(task_group_pb.colour().b())));
        dummy_state2->SetColor(TaskGroup::GroupState::Color(
                static_cast<uint8_t>(task_group_pb.colour().r())
                , static_cast<uint8_t>(task_group_pb.colour().g())
                , static_cast<uint8_t>(task_group_pb.colour().b())));

        std::list<std::pair<std::shared_ptr<EcalSysTask>, TaskState>> minimal_states_list;
        for (const auto& task_pb : task_group_pb.tasks())
        {
          auto task = FromProtobuf(task_pb);
          minimal_states_list.push_back({task, task->GetMonitoringTaskState()});
        }
        dummy_state1->SetMinimalStatesList(minimal_states_list);

        std::list<std::shared_ptr<TaskGroup::GroupState>> group_state_list { dummy_state1, dummy_state2 }; 

        task_group->SetGroupStateList(group_state_list);
        task_group->SetName(task_group_pb.name());
      }

      TaskState                    FromProtobuf(const eCAL::pb::ProcessState& task_state_pb)
      {
        eCAL::pb::sys::ProcessState sys_task_state_pb;
        sys_task_state_pb.set_severity((eCAL::pb::sys::eProcessSeverity)task_state_pb.severity());
        sys_task_state_pb.set_severity_level((eCAL::pb::sys::eProcessSeverityLevel)task_state_pb.severity());
        sys_task_state_pb.set_info(task_state_pb.info());
        return FromProtobuf(sys_task_state_pb);
      }

      TaskState                    FromProtobuf(const eCAL::pb::sys::ProcessState&         task_state_pb)
      {
        TaskState output;
        FromProtobuf(task_state_pb, output);
        return output;
      }

      std::shared_ptr<EcalSysTask> FromProtobuf(const eCAL::pb::sys::State::Task&     task_pb)
      {
        std::shared_ptr<EcalSysTask> output = std::make_shared<EcalSysTask>();
        FromProtobuf(task_pb, output);
        return output;
      }

      std::shared_ptr<TaskGroup>   FromProtobuf(const eCAL::pb::sys::State::Group&    task_group_pb)
      {
        std::shared_ptr<TaskGroup> output = std::make_shared<TaskGroup>();
        FromProtobuf(task_group_pb, output);
        return output;
      }


      ///////////////////////////////
      // To Protobuf
      ///////////////////////////////

      void ToProtobuf(eCAL::pb::sys::ProcessState&         task_state_pb,    const TaskState&                    task_state)
      {
        switch (task_state.severity)
        {
        case eCAL::Process::eSeverity::unknown:
          task_state_pb.set_severity(eCAL::pb::sys::eProcessSeverity::proc_sev_unknown);
          break;
        case eCAL::Process::eSeverity::healthy:
          task_state_pb.set_severity(eCAL::pb::sys::eProcessSeverity::proc_sev_healthy);
          break;
        case eCAL::Process::eSeverity::warning:
          task_state_pb.set_severity(eCAL::pb::sys::eProcessSeverity::proc_sev_warning);
          break;
        case eCAL::Process::eSeverity::critical:
          task_state_pb.set_severity(eCAL::pb::sys::eProcessSeverity::proc_sev_critical);
          break;
        case eCAL::Process::eSeverity::failed:
          task_state_pb.set_severity(eCAL::pb::sys::eProcessSeverity::proc_sev_failed);
          break;
        default:
          task_state_pb.set_severity(eCAL::pb::sys::eProcessSeverity::proc_sev_unknown);
          break;
        }

        switch (task_state.severity_level)
        {
        case eCAL::Process::eSeverityLevel::level1:
          task_state_pb.set_severity_level(eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level1);
          break;
        case eCAL::Process::eSeverityLevel::level2:
          task_state_pb.set_severity_level(eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level2);
          break;
        case eCAL::Process::eSeverityLevel::level3:
          task_state_pb.set_severity_level(eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level3);
          break;
        case eCAL::Process::eSeverityLevel::level4:
          task_state_pb.set_severity_level(eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level4);
          break;
        case eCAL::Process::eSeverityLevel::level5:
          task_state_pb.set_severity_level(eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level5);
          break;
        default:
          task_state_pb.set_severity_level(eCAL::pb::sys::eProcessSeverityLevel::proc_sev_level_unknown);
          break;
        }

        task_state_pb.set_info(task_state.info);
      }

      void ToProtobuf(eCAL::pb::sys::State::Task&     task_pb,          const std::shared_ptr<EcalSysTask>& task)
      {
        task_pb.set_id                 (task->GetId());
        task_pb.set_name               (task->GetName());
        task_pb.set_target_host        (task->GetTarget());
        ToProtobuf                     (*(task_pb.mutable_state()), task->GetMonitoringTaskState());
        for (int process_id : task->GetPids())
        {
          task_pb.add_process_ids(process_id);
        }
        task_pb.set_launch_order       (task->GetLaunchOrder());
        task_pb.set_path               (task->GetAlgoPath());
        task_pb.set_working_dir        (task->GetWorkingDir());
        task_pb.set_command_line       (task->GetCommandLineArguments());
        task_pb.set_window_mode        (eCAL::sys_client::proto_helpers::ToProtobuf(task->GetVisibility()));
        task_pb.set_waiting_time_secs  (std::chrono::duration_cast<std::chrono::duration<double>>(task->GetTimeoutAfterStart()).count());
        task_pb.set_monitoring_enabled (task->IsMonitoringEnabled());
        if (task->IsRestartBySeverityEnabled())
          ToProtobuf (*task_pb.mutable_restart_by_severity(), task->GetRestartAtSeverity());
        task_pb.set_current_host       (task->GetHostStartedOn());
      }

      void ToProtobuf(eCAL::pb::sys::State::Group&    task_group_pb,    const std::shared_ptr<TaskGroup>&   task_group)
      {
        auto state = task_group->Evaluate();

        task_group_pb.set_name(task_group->GetName());

        if (state)
        {
          task_group_pb.set_state(state->GetName());
          task_group_pb.mutable_colour()->set_r(state->GetColor().red);
          task_group_pb.mutable_colour()->set_g(state->GetColor().green);
          task_group_pb.mutable_colour()->set_b(state->GetColor().blue);
        }

        for (const auto& task : task_group->GetAllTasks())
        {
          ToProtobuf(*task_group_pb.add_tasks(), task);
        }
      }

      void ToProtobuf(eCAL::pb::sys::State&           state_pb,         const EcalSys&                      ecalsys)
      {
        for (const auto& task : ecalsys.GetTaskList())
        {
          ToProtobuf(*(state_pb.add_tasks()), task);
        }

        for (const auto& group : ecalsys.GetGroupList())
        {
          ToProtobuf(*(state_pb.add_groups()), group);
        }

        state_pb.set_host(eCAL::Process::GetHostName());
      }


      eCAL::pb::sys::ProcessState         ToProtobuf(const TaskState&                    task_state)
      {
        eCAL::pb::sys::ProcessState output_pb;
        ToProtobuf(output_pb, task_state);
        return output_pb;
      }

      eCAL::pb::sys::State::Task     ToProtobuf(const std::shared_ptr<EcalSysTask>& task)
      {
        eCAL::pb::sys::State::Task output_pb;
        ToProtobuf(output_pb, task);
        return output_pb;
      }

      eCAL::pb::sys::State::Group    ToProtobuf(const std::shared_ptr<TaskGroup>&   task_group)
      {
        eCAL::pb::sys::State::Group output_pb;
        ToProtobuf(output_pb, task_group);
        return output_pb;
      }

      eCAL::pb::sys::State           ToProtobuf(const EcalSys&                      ecalsys)
      {
        eCAL::pb::sys::State output_pb;
        ToProtobuf(output_pb, ecalsys);
        return output_pb;
      }

    }
  }
}
