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

#include "list.h"
#include "helpers.h"

#include <iostream>
#include <iomanip>

// termcolor pollutes everything by including Windows.h in the header file.
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif // _WIN32
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800) // disable termcolor warnings
#endif
#include <termcolor/termcolor.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ecal_utils/string.h>

#include "helpers.h"

namespace eCAL
{
  namespace sys
  {
    namespace command
    {

      std::string List::Usage() const
      {
        return "[--tasks | --groups | --runners ] [ID or name]";
      }

      std::string List::Help() const
      {
        return "Prints a list of all tasks / groups / runners. If an ID or name is given, detailed information about that item are printed. If no argument --groups / --runners / --tasks is given, information about tasks are printed.";
      }

      std::string List::Example() const
      {
        return "--tasks task1";
      }

      eCAL::sys::Error List::Execute(const std::shared_ptr<EcalSys>& ecalsys_instance, const std::vector<std::string>& argv) const
      {
        bool task_mode = false;
        bool group_mode = false;
        bool runner_mode = false;

        std::vector<std::string> id_or_name;

        if ((argv.size() >= 1)
          && ((argv[0] == "--tasks")
              || (argv[0] == "--runners")
              || (argv[0] == "--groups")))
        {
          if (argv[0] == "--groups")
            group_mode = true;
          else if (argv[0] == "--runners")
            runner_mode = true;
          else
            task_mode = true;

          id_or_name = std::vector<std::string>(std::next(argv.begin()), argv.end());
        }
        else
        {
          task_mode = true;
          id_or_name = argv;
        }

        if (id_or_name.size() > 1)
        {
          return Error(Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));
        }
        
        if (task_mode)
        {
          if (id_or_name.empty())
          {
            return ListTasks(ecalsys_instance->GetTaskList());
          }
          else
          {
            std::list<std::shared_ptr<EcalSysTask>> task_list;
            Error error = ToTaskList(ecalsys_instance, id_or_name, task_list);
            if (error)
              return error;

            return ListSingleTask(task_list.front());
          }
        }
        else if (runner_mode)
        {
          if (id_or_name.empty())
          {
            return ListRunners(ecalsys_instance->GetRunnerList());
          }
          else
          {
            std::list<std::shared_ptr<EcalSysRunner>> runner_list;
            Error error = ToRunnerList(ecalsys_instance, id_or_name, runner_list);
            if (error)
              return error;

            return ListSingleRunner(runner_list.front());
          }
        }
        else if (group_mode)
        {
          if (id_or_name.empty())
          {
            return ListGroups(ecalsys_instance->GetGroupList());
          }
          else
          {
            std::list<std::shared_ptr<TaskGroup>> group_list;
            Error error = ToGroupList(ecalsys_instance, id_or_name, group_list);
            if (error)
              return error;

            return ListSingleGroup(group_list.front());
          }
        }

        return Error(Error::ErrorCode::PARAMETER_ERROR);
      }

      eCAL::sys::Error List::Execute(const std::string& hostname, const std::shared_ptr<eCAL::protobuf::CServiceClient<eCAL::pb::sys::Service>>& remote_ecalsys_service, const std::vector<std::string>& argv) const
      {
        bool task_mode = false;
        bool group_mode = false;
        bool runner_mode = false;

        std::vector<std::string> id_or_name;

        if ((argv.size() >= 1)
          && ((argv[0] == "--tasks")
              || (argv[0] == "--runners")
              || (argv[0] == "--groups")))
        {
          if (argv[0] == "--groups")
            group_mode = true;
          else if (argv[0] == "--runners")
            runner_mode = true;
          else
            task_mode = true;

          id_or_name = std::vector<std::string>(std::next(argv.begin()), argv.end());
        }
        else
        {
          task_mode = true;
          id_or_name = argv;
        }

        if (id_or_name.size() > 1)
        {
          return Error(Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));
        }

        eCAL::pb::sys::State sys_state_pb;
        {
          auto error = GetRemoteSysStatus(hostname, remote_ecalsys_service, sys_state_pb);
          if (error)
            return error;
        }
        
        if (task_mode)
        {
          if (id_or_name.empty())
          {
            std::list<std::shared_ptr<EcalSysTask>> task_list;
            {
              Error error = eCAL::sys::command::GetCompleteTaskList(sys_state_pb, task_list);
              if (error)
                return error;
            }
            return ListTasks(task_list);
          }
          else
          {
            std::list<std::shared_ptr<EcalSysTask>> task_list;
            {
              Error error = ToTaskList(sys_state_pb, id_or_name, task_list);
              if (error)
                return error;
            }
            return ListSingleTask(task_list.front());
          }
        }
        else if (runner_mode)
        {
          return Error(Error::ErrorCode::COMMAND_NOT_AVAILABLE_IN_REMOTE_MODE);
        }
        else if (group_mode)
        {
          if (id_or_name.empty())
          {
            std::list<std::shared_ptr<TaskGroup>> group_list;
            {
              Error error = eCAL::sys::command::GetCompleteGroupList(sys_state_pb, group_list);
              if (error)
                return error;
            }
            return ListGroups(group_list);
          }
          else
          {
            std::list<std::shared_ptr<TaskGroup>> group_list;
            {
              Error error = ToGroupList(sys_state_pb, id_or_name, group_list);
              if (error)
                return error;
            }
            return ListSingleGroup(group_list.front());
          }
        }

        return Error(Error::ErrorCode::PARAMETER_ERROR);
      }

      eCAL::sys::Error List::ListTasks(const std::list<std::shared_ptr<EcalSysTask>>& task_list) const
      {
        std::vector<std::string> header_data = {"#", "ID", "Task", "Target", "PID", "Current", "State", "Info"};

        size_t longest_launch_order = header_data[0].size();
        size_t longest_id           = header_data[1].size();
        size_t longest_task_name    = header_data[2].size();
        size_t longest_target_host  = header_data[3].size();
        size_t longest_pid          = header_data[4].size();
        size_t longest_current_host = header_data[5].size();
        size_t longest_state        = header_data[6].size();

        std::vector<std::vector<std::string>> string_data;
        std::vector<eCAL_Process_eSeverity>   serverity_data;
        string_data   .reserve(task_list.size());
        serverity_data.reserve(task_list.size());

        for (const auto& task : task_list)
        {
          std::vector<std::string> this_task_string_data;
          this_task_string_data.reserve(header_data.size());

          this_task_string_data.push_back(std::to_string(task->GetLaunchOrder()));
          this_task_string_data.push_back(std::to_string(task->GetId()));
          this_task_string_data.push_back(task->GetName());
          this_task_string_data.push_back(task->GetTarget());

          auto pids = task->GetPids();
          this_task_string_data.push_back(pids.empty() ? "" : std::to_string(pids[0]));

          this_task_string_data.push_back(task->GetHostStartedOn());

          std::stringstream state_ss;
          auto monitoring_state = task->GetMonitoringTaskState();
          std::string state, level;
          monitoring_state.ToString(state, level);

          serverity_data.push_back(monitoring_state.severity);

          switch (monitoring_state.severity)
          {
          case eCAL_Process_eSeverity::proc_sev_healthy:
            state_ss << termcolor::on_green << state << termcolor::reset;
            break;
          case eCAL_Process_eSeverity::proc_sev_warning:
            state_ss << termcolor::on_yellow << state << termcolor::reset;
            break;
          case eCAL_Process_eSeverity::proc_sev_critical:
            state_ss << termcolor::on_red << state << termcolor::reset;
            break;
          case eCAL_Process_eSeverity::proc_sev_failed:
            state_ss << termcolor::on_magenta << state << termcolor::reset;
            break;
          default:
            state_ss << state;
            break;
          }

          this_task_string_data.push_back(state_ss.str());

          this_task_string_data.push_back(monitoring_state.info);

          longest_launch_order = std::max(longest_launch_order, this_task_string_data[0].size());
          longest_id           = std::max(longest_id,           this_task_string_data[1].size());
          longest_task_name    = std::max(longest_task_name,    this_task_string_data[2].size());
          longest_target_host  = std::max(longest_target_host,  this_task_string_data[3].size());
          longest_pid          = std::max(longest_pid,          this_task_string_data[4].size());
          longest_current_host = std::max(longest_current_host, this_task_string_data[5].size());
          longest_state        = std::max(longest_state,        this_task_string_data[6].size());

          string_data.push_back(std::move(this_task_string_data));
        }

        std::cout << std::right
                  << std::setw(longest_launch_order) << header_data[0] << " "
                  << std::left
                  << std::setw(longest_id)           << header_data[1] << " "
                  << std::setw(longest_task_name)    << header_data[2] << " "
                  << std::setw(longest_target_host)  << header_data[3] << " "
                  << std::setw(longest_pid)          << header_data[4] << " "
                  << std::setw(longest_current_host) << header_data[5] << " "
                  << std::setw(longest_state)        << header_data[6] << " "
                  << header_data[7]
                  << std::endl;

        std::cout << std::string(longest_launch_order,  '-') << " "
                  << std::string(longest_id,            '-') << " "
                  << std::string(longest_task_name,     '-') << " "
                  << std::string(longest_target_host,   '-') << " "
                  << std::string(longest_pid,           '-') << " "
                  << std::string(longest_current_host,  '-') << " "
                  << std::string(longest_state,         '-') << " "
                  << std::string(header_data[7].size(), '-')
                  << std::endl;

        for (size_t i = 0; i < string_data.size(); i++)
        {
          std::vector<std::string>& line = string_data[i];

          std::cout << std::right
                    << std::setw(longest_launch_order) << line[0] << " "
                    << std::left
                    << std::setw(longest_id)           << line[1] << " "
                    << std::setw(longest_task_name)    << line[2] << " "
                    << std::setw(longest_target_host)  << line[3] << " "
                    << std::setw(longest_pid)          << line[4] << " "
                    << std::setw(longest_current_host) << line[5] << " ";

          switch (serverity_data[i])
          {
          case eCAL_Process_eSeverity::proc_sev_healthy:
            std::cout << termcolor::on_green;
            break;
          case eCAL_Process_eSeverity::proc_sev_warning:
            std::cout << termcolor::on_yellow;
            break;
          case eCAL_Process_eSeverity::proc_sev_critical:
            std::cout << termcolor::on_red;
            break;
          case eCAL_Process_eSeverity::proc_sev_failed:
            std::cout << termcolor::on_magenta;
            break;
          default:
            break;
          }

          std::cout << std::setw(longest_state)        << line[6] << termcolor::reset << " "
                    << line[7]
                    << std::endl;
        }

        return Error::ErrorCode::OK;
      }

      eCAL::sys::Error List::ListSingleTask(const std::shared_ptr<EcalSysTask>& task) const
      {
        std::string visibility_string;
        switch (task->GetVisibility())
        {
        case eCAL_Process_eStartMode::proc_smode_hidden:
          visibility_string = "hidden";
          break;
        case eCAL_Process_eStartMode::proc_smode_minimized:
          visibility_string = "minimized";
          break;
        case eCAL_Process_eStartMode::proc_smode_maximized:
          visibility_string = "maximized";
          break;
        case eCAL_Process_eStartMode::proc_smode_normal:
          visibility_string = "normal";
          break;
        default:
          visibility_string = "unknown";
          break;
        }

        auto pids = task->GetPids();

        std::string state, level;
        auto task_state = task->GetMonitoringTaskState();
        task_state.ToString(state, level);

        auto runner = task->GetRunner();

        std::cout << "ID:                  " << task->GetId() << std::endl;
        std::cout << "Launch#:             " << task->GetLaunchOrder() << std::endl;
        std::cout << "Task Name:           " << task->GetName() << std::endl;
        std::cout << "Target Host:         " << task->GetTarget() << std::endl;
        std::cout << "Runner:              " << (runner ? runner->GetName() +" (" + std::to_string(task->GetRunner()->GetId()) + ")" : "") << std::endl;
        std::cout << "Path:                " << task->GetAlgoPath() << std::endl;
        std::cout << "Working Dir:         " << task->GetWorkingDir() << std::endl;
        std::cout << "Command line:        " << task->GetCommandLineArguments() << std::endl;
        std::cout << "Visibility:          " << visibility_string << std::endl;
        std::cout << "Waiting time:        " << std::chrono::duration_cast<std::chrono::duration<double>>(task->GetTimeoutAfterStart()).count() << " s" << std::endl;
        std::cout << "Monitoring enabled:  " << (task->IsMonitoringEnabled() ? "Yes" : "No") << std::endl;
        std::cout << "Restart by severity: " << (task->IsRestartBySeverityEnabled() ? "Yes: " + task->GetRestartAtSeverity().ToString() : "No") << std::endl;
        std::cout << "PID:                 " << (pids.empty() ? "" : std::to_string(pids[0])) << std::endl;
        std::cout << "Current Host:        " << task->GetHostStartedOn() << std::endl;

        switch (task_state.severity)
        {
        case eCAL_Process_eSeverity::proc_sev_healthy:
          std::cout << termcolor::on_green;
          break;
        case eCAL_Process_eSeverity::proc_sev_warning:
          std::cout << termcolor::on_yellow;
          break;
        case eCAL_Process_eSeverity::proc_sev_critical:
          std::cout << termcolor::on_red;
          break;
        case eCAL_Process_eSeverity::proc_sev_failed:
          std::cout << termcolor::on_magenta;
          break;
        default:
          break;
        }

        std::cout << "State:               " << (task_state.severity == eCAL_Process_eSeverity::proc_sev_unknown ? state : state + " " + level) << std::endl;
        std::cout << termcolor::reset;

        std::cout << "Info:                " << task_state.info << std::endl;

        return eCAL::sys::Error::ErrorCode::OK;
      }

      eCAL::sys::Error List::ListRunners(const std::list<std::shared_ptr<EcalSysRunner>>& runner_list) const
      {
        std::vector<std::string> header_data = {"ID", "Name"};

        size_t longest_id = header_data[0].size();

        std::vector<std::vector<std::string>> string_data;
        string_data   .reserve(runner_list.size());

        for (const auto& runner : runner_list)
        {
          std::vector<std::string> this_runner_string_data;
          this_runner_string_data.reserve(header_data.size());

          this_runner_string_data.push_back(std::to_string(runner->GetId()));
          this_runner_string_data.push_back(runner->GetName());

          longest_id = std::max(longest_id, this_runner_string_data[0].size());

          string_data.push_back(std::move(this_runner_string_data));
        }

        std::cout << std::right
                  << std::setw(longest_id) << header_data[0] << " "
                  << std::left
                  << header_data[1]
                  << std::endl;

        std::cout << std::string(longest_id,            '-') << " "
                  << std::string(header_data[1].size(), '-')
                  << std::endl;

        for (size_t i = 0; i < string_data.size(); i++)
        {
          std::vector<std::string>& line = string_data[i];

          std::cout << std::right
                    << std::setw(longest_id) << line[0] << " "
                    << std::left
                    << line[1]
                    << std::endl;
        }

        return Error::ErrorCode::OK;
      }

      eCAL::sys::Error List::ListSingleRunner(const std::shared_ptr<EcalSysRunner>& runner) const
      {
        std::cout << "ID:                " << runner->GetId()              << std::endl;
        std::cout << "Name:              " << runner->GetName()            << std::endl;
        std::cout << "Path:              " << runner->GetPath()            << std::endl;
        std::cout << "Default algo dir:  " << runner->GetDefaultAlgoDir()  << std::endl;
        std::cout << "Command line args: " << runner->GetLoadCmdArgument() << std::endl;

        return eCAL::sys::Error::ErrorCode::OK;
      }

      eCAL::sys::Error List::ListGroups(const std::list<std::shared_ptr<TaskGroup>>& group_list) const
      {
        std::vector<std::string> header_data = {"ID", "Name", "State"};

        size_t longest_id   = header_data[0].size();
        size_t longest_name = header_data[1].size();

        std::vector<std::vector<std::string>> string_data;
        string_data   .reserve(group_list.size());

        for (const auto& group : group_list)
        {
          std::vector<std::string> this_group_string_data;
          this_group_string_data.reserve(header_data.size());

          auto group_state = group->Evaluate();

          this_group_string_data.push_back(std::to_string(group->GetId()));
          this_group_string_data.push_back(group->GetName());
          this_group_string_data.push_back(group_state ? group_state->GetName() : "");

          longest_id   = std::max(longest_id,   this_group_string_data[0].size());
          longest_name = std::max(longest_name, this_group_string_data[1].size());

          string_data.push_back(std::move(this_group_string_data));
        }

        std::cout << std::right
                  << std::setw(longest_id) << header_data[0] << " "
                  << std::left
                  << std::setw(longest_name) << header_data[1] << " "
                  << header_data[2]
                  << std::endl;

        std::cout << std::string(longest_id,            '-') << " "
                  << std::string(longest_name,          '-') << " "
                  << std::string(header_data[2].size(), '-')
                  << std::endl;

        for (size_t i = 0; i < string_data.size(); i++)
        {
          std::vector<std::string>& line = string_data[i];

          std::cout << std::right
            << std::setw(longest_id)   << line[0] << " "
            << std::left
            << std::setw(longest_name) << line[1] << " "
            << line[2]
            << std::endl;
        }

        return Error::ErrorCode::OK;
      }

      eCAL::sys::Error List::ListSingleGroup(const std::shared_ptr<TaskGroup>& group) const
      {
        auto task_set = group->GetAllTasks();
        std::list<std::shared_ptr<EcalSysTask>> task_list(task_set.begin(), task_set.end());

        auto group_state = group->Evaluate();
        
        std::cout << "ID:    " << group->GetId() << std::endl;
        std::cout << "Name:  " << group->GetName() << std::endl;
        std::cout << "State: " << (group_state ? group_state->GetName() : "") << termcolor::reset << std::endl;
        std::cout << std::endl;
        
        ListTasks(task_list);

        return Error::ErrorCode::OK;
      }




















    }
  }
}
