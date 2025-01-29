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

#include <sys_client_core/proto_helpers.h>

namespace eCAL
{
  namespace sys_client
  {
    namespace proto_helpers
    {
      ///////////////////////////////
      // From Protobuf
      ///////////////////////////////

      void FromProtobuf(const eCAL::pb::sys_client::Task&                task_pb,              eCAL::sys_client::Task&            task)
      {
        auto test = task_pb.runner();

        task.runner      = FromProtobuf(task_pb.runner());
        task.path        = task_pb.path();
        task.arguments   = task_pb.arguments();
        task.working_dir = task_pb.working_dir();
      }

      void FromProtobuf(const eCAL::pb::sys_client::Runner&              runner_pb,            eCAL::sys_client::Runner&          runner)
      {
        runner.path             = runner_pb.path();
        runner.arguments        = runner_pb.arguments();
        runner.default_task_dir = runner_pb.default_task_dir();
      }

      void FromProtobuf(const eCAL::pb::sys_client::WindowMode&          window_mode_pb,       eCAL::Process::eStartMode&           window_mode)
      {
        window_mode = FromProtobuf(window_mode_pb);
      }

      void FromProtobuf(const eCAL::pb::sys_client::StartTaskParameters& start_task_param_pb,  StartTaskParameters&               start_task_param)
      {
        start_task_param.task           = FromProtobuf(start_task_param_pb.task());
        start_task_param.window_mode    = FromProtobuf(start_task_param_pb.window_mode());
        start_task_param.create_console = start_task_param_pb.create_console();
      }

      void FromProtobuf(const eCAL::pb::sys_client::StartTaskRequest&    start_task_list_pb,   std::vector<StartTaskParameters>&  start_task_list)
      {
        start_task_list.clear();
        start_task_list.reserve(start_task_list_pb.tasks().size());

        for (const auto& start_task_param_pb : start_task_list_pb.tasks())
        {
          start_task_list.push_back(FromProtobuf(start_task_param_pb));
        }
      }

      void FromProtobuf(const eCAL::pb::sys_client::StopTaskParameters&  stop_task_param_pb,   StopTaskParameters&                stop_task_param)
      {
        stop_task_param.process_id    = stop_task_param_pb.process_id();
        stop_task_param.task          = FromProtobuf(stop_task_param_pb.task());
        stop_task_param.ecal_shutdown = stop_task_param_pb.ecal_shutdown();
      }

      void FromProtobuf(const eCAL::pb::sys_client::StopTaskRequest&     stop_task_list_pb,    std::vector<StopTaskParameters>&   stop_task_list)
      {
        stop_task_list.clear();
        stop_task_list.reserve(stop_task_list_pb.tasks().size());

        for (const auto& stop_task_param_pb : stop_task_list_pb.tasks())
        {
          stop_task_list.push_back(FromProtobuf(stop_task_param_pb));
        }
      }

      void FromProtobuf(const eCAL::pb::sys_client::TaskList&            task_list_pb,         std::vector<Task>&                 task_list)
      {
        task_list.clear();
        task_list.reserve(task_list_pb.tasks_size());
        for (const auto& task_pb : task_list_pb.tasks())
        {
          task_list.push_back(FromProtobuf(task_pb));
        }
      }

      void FromProtobuf(const eCAL::pb::sys_client::ProcessIdList&             pid_list_pb,          std::vector<int32_t>&              pid_list)
      {
        pid_list.clear();
        pid_list.reserve(pid_list_pb.process_ids_size());
        for (const int32_t process_id : pid_list_pb.process_ids())
        {
          pid_list.push_back(process_id);
        }
      }

      void FromProtobuf(const eCAL::pb::sys_client::MatchTaskResponse&   map_task_response_pb, std::vector<std::vector<int32_t>>& map_task_response)
      {
        map_task_response.clear();
        map_task_response.reserve(map_task_response_pb.process_id_lists_size());
        for (const auto& pid_list_pb : map_task_response_pb.process_id_lists())
        {
          map_task_response.push_back(FromProtobuf(pid_list_pb));
        }
      }


      eCAL::sys_client::Task            FromProtobuf(const eCAL::pb::sys_client::Task&                task_pb)
      {
        eCAL::sys_client::Task task;
        FromProtobuf(task_pb, task);
        return task;
      }

      eCAL::sys_client::Runner          FromProtobuf(const eCAL::pb::sys_client::Runner&              runner_pb)
      {
        eCAL::sys_client::Runner runner;
        FromProtobuf(runner_pb, runner);
        return runner;
      }

      eCAL::Process::eStartMode           FromProtobuf(const eCAL::pb::sys_client::WindowMode&          window_mode_pb)
      {
        switch (window_mode_pb)
        {
        case eCAL::pb::sys_client::WindowMode::hidden:
          return eCAL::Process::eStartMode::hidden;
        case eCAL::pb::sys_client::WindowMode::minimized:
          return eCAL::Process::eStartMode::minimized;
        case eCAL::pb::sys_client::WindowMode::maximized:
          return eCAL::Process::eStartMode::maximized;
        default:
          return eCAL::Process::eStartMode::normal;
        }
      }

      StartTaskParameters               FromProtobuf(const eCAL::pb::sys_client::StartTaskParameters& start_task_param_pb)
      {
        StartTaskParameters task_params;
        FromProtobuf(start_task_param_pb, task_params);
        return task_params;
      }

      std::vector<StartTaskParameters>  FromProtobuf(const eCAL::pb::sys_client::StartTaskRequest&    start_task_list_pb)
      {
        std::vector<StartTaskParameters> task_list;
        FromProtobuf(start_task_list_pb, task_list);
        return task_list;
      }

      StopTaskParameters                FromProtobuf(const eCAL::pb::sys_client::StopTaskParameters&  stop_task_param_pb)
      {
        StopTaskParameters stop_task_params;
        FromProtobuf(stop_task_param_pb, stop_task_params);
        return stop_task_params;
      }

      std::vector<StopTaskParameters>   FromProtobuf(const eCAL::pb::sys_client::StopTaskRequest&     stop_task_list_pb)
      {
        std::vector<StopTaskParameters> stop_task_list;
        FromProtobuf(stop_task_list_pb, stop_task_list);
        return stop_task_list;
      }

      std::vector<Task>                 FromProtobuf(const eCAL::pb::sys_client::TaskList&            task_list_pb)
      {
        std::vector<Task> output;
        FromProtobuf(task_list_pb, output);
        return output;
      }

      std::vector<int32_t>              FromProtobuf(const eCAL::pb::sys_client::ProcessIdList&             pid_list_pb)
      {
        std::vector<int32_t> output;
        FromProtobuf(pid_list_pb, output);
        return output;
      }

      std::vector<std::vector<int32_t>> FromProtobuf(const eCAL::pb::sys_client::MatchTaskResponse&   map_task_response_pb)
      {
        std::vector<std::vector<int32_t>> output;
        FromProtobuf(map_task_response_pb, output);
        return output;
      }


      ///////////////////////////////
      // To Protobuf
      ///////////////////////////////

      void ToProtobuf(eCAL::pb::sys_client::Task&                task_pb,              const eCAL::sys_client::Task&           task)
      {
        task_pb.set_path       (task.path);
        task_pb.set_arguments  (task.arguments);
        task_pb.set_working_dir(task.working_dir);
        ToProtobuf             (*task_pb.mutable_runner(), task.runner);
      }

      void ToProtobuf(eCAL::pb::sys_client::Runner&              runner_pb,            const eCAL::sys_client::Runner&         runner)
      {
        runner_pb.set_path            (runner.path);
        runner_pb.set_arguments       (runner.arguments);
        runner_pb.set_default_task_dir(runner.default_task_dir);
      }

      void ToProtobuf(eCAL::pb::sys_client::WindowMode&          window_mode_pb,       const eCAL::Process::eStartMode           window_mode)
      {
        window_mode_pb = ToProtobuf(window_mode);
      }

      void ToProtobuf(eCAL::pb::sys_client::StartTaskParameters& start_task_param_pb,  const StartTaskParameters&              start_task_param)
      {
        ToProtobuf                            (*start_task_param_pb.mutable_task(), start_task_param.task);
        start_task_param_pb.set_window_mode   (ToProtobuf(start_task_param.window_mode));
        start_task_param_pb.set_create_console(start_task_param.create_console);
      }

      void ToProtobuf(eCAL::pb::sys_client::StartTaskRequest&    start_task_list_pb,   const std::vector<StartTaskParameters>& start_task_list)
      {
        start_task_list_pb.clear_tasks();
        for (const StartTaskParameters& param : start_task_list)
        {
          ToProtobuf(*start_task_list_pb.add_tasks(), param);
        }
      }

      void ToProtobuf(eCAL::pb::sys_client::StopTaskParameters&  stop_task_param_pb,   const StopTaskParameters&               stop_task_param)
      {
        stop_task_param_pb.set_process_id   (stop_task_param.process_id);
        ToProtobuf                          (*stop_task_param_pb.mutable_task(), stop_task_param.task);
        stop_task_param_pb.set_ecal_shutdown(stop_task_param.ecal_shutdown);
      }

      void ToProtobuf(eCAL::pb::sys_client::StopTaskRequest&     stop_task_list_pb,    const std::vector<StopTaskParameters>&   stop_task_list)
      {
        stop_task_list_pb.clear_tasks();
        for (const StopTaskParameters& param : stop_task_list)
        {
          ToProtobuf(*stop_task_list_pb.add_tasks(), param);
        }
      }

      void ToProtobuf(eCAL::pb::sys_client::TaskList&            task_list_pb,         const std::vector<Task>&                 task_list)
      {
        task_list_pb.clear_tasks();
        task_list_pb.mutable_tasks()->Reserve(static_cast<int>(task_list.size()));
        for (const Task& task : task_list)
        {
          ToProtobuf(*task_list_pb.add_tasks(), task);
        }
      }

      void ToProtobuf(eCAL::pb::sys_client::ProcessIdList&             pid_list_pb,          const std::vector<int32_t>&              pid_list)
      {
        pid_list_pb.clear_process_ids();
        pid_list_pb.mutable_process_ids()->Reserve(static_cast<int>(pid_list.size()));
        for (const int32_t process_id : pid_list)
        {
          pid_list_pb.add_process_ids(process_id);
        }
      }

      void ToProtobuf(eCAL::pb::sys_client::MatchTaskResponse&   map_task_response_pb, const std::vector<std::vector<int32_t>>& map_task_response)
      {
        map_task_response_pb.clear_process_id_lists();
        map_task_response_pb.mutable_process_id_lists()->Reserve(static_cast<int>(map_task_response.size()));
        for (const std::vector<int32_t>& pid_list : map_task_response)
        {
          ToProtobuf(*map_task_response_pb.add_process_id_lists(), pid_list);
        }
      }


      eCAL::pb::sys_client::Task                ToProtobuf(const eCAL::sys_client::Task&                task)
      {
        eCAL::pb::sys_client::Task output;
        ToProtobuf(output, task);
        return output;
      }

      eCAL::pb::sys_client::Runner              ToProtobuf(const eCAL::sys_client::Runner&              runner)
      {
        eCAL::pb::sys_client::Runner output;
        ToProtobuf(output, runner);
        return output;
      }

      eCAL::pb::sys_client::WindowMode    ToProtobuf(const eCAL::Process::eStartMode                window_mode)
      {
        switch (window_mode)
        {
        case eCAL::Process::eStartMode::hidden:
          return eCAL::pb::sys_client::WindowMode::hidden;
        case eCAL::Process::eStartMode::minimized:
          return eCAL::pb::sys_client::WindowMode::minimized;
        case eCAL::Process::eStartMode::maximized:
          return eCAL::pb::sys_client::WindowMode::maximized;
        default:
          return eCAL::pb::sys_client::WindowMode::normal;
        }
      }

      eCAL::pb::sys_client::StartTaskParameters ToProtobuf(const StartTaskParameters&                   start_task_param)
      {
        eCAL::pb::sys_client::StartTaskParameters output;
        ToProtobuf(output, start_task_param);
        return output;
      }

      eCAL::pb::sys_client::StartTaskRequest    ToProtobuf(const std::vector<StartTaskParameters>&      start_task_list)
      {
        eCAL::pb::sys_client::StartTaskRequest output;
        ToProtobuf(output, start_task_list);
        return output;
      }

      eCAL::pb::sys_client::StopTaskParameters  ToProtobuf(const StopTaskParameters&                    stop_task_param)
      {
        eCAL::pb::sys_client::StopTaskParameters output;
        ToProtobuf(output, stop_task_param);
        return output;
      }

      eCAL::pb::sys_client::StopTaskRequest     ToProtobuf(const std::vector<StopTaskParameters>&       stop_task_list)
      {
        eCAL::pb::sys_client::StopTaskRequest output;
        ToProtobuf(output, stop_task_list);
        return output;
      }

      eCAL::pb::sys_client::TaskList            ToProtobuf(const std::vector<Task>&                     task_list)
      {
        eCAL::pb::sys_client::TaskList output_pb;
        ToProtobuf(output_pb, task_list);
        return output_pb;
      }

      eCAL::pb::sys_client::ProcessIdList       ToProtobuf(const std::vector<int32_t>&                  pid_list)
      {
        eCAL::pb::sys_client::ProcessIdList output_pb;
        ToProtobuf(output_pb, pid_list);
        return output_pb;
      }

      eCAL::pb::sys_client::MatchTaskResponse   ToProtobuf(const std::vector<std::vector<int32_t>>&     map_task_response)
      {
        eCAL::pb::sys_client::MatchTaskResponse output_pb;
        ToProtobuf(output_pb, map_task_response);
        return output_pb;
      }
    }
  }
}
