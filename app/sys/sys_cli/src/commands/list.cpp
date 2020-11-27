/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include <ecal_utils/string.h>

namespace eCAL
{
  namespace sys
  {
    namespace command
    {

      std::string List::Usage() const
      {
        return "[ID or name]";
      }

      std::string List::Help() const
      {
        return "Prints a list of all tasks. If an ID or task name is given, detailed information about that task are printed.";
      }

      std::string List::Example() const
      {
        return "task1";
      }

      eCAL::sys::Error List::Execute(const std::shared_ptr<EcalSys>& ecalsys_instance, const std::vector<std::string>& argv)
      {
        if (argv.empty())
        {
          // Print a list of all tasks

          std::vector<std::string> header_data = {"#", "ID", "Task", "Target", "PID", "Current", "State", "Info"};
          std::vector<std::vector<std::string>> string_data;

          size_t longest_launch_order = header_data[0].size();
          size_t longest_id           = header_data[1].size();
          size_t longest_task_name    = header_data[2].size();
          size_t longest_target_host  = header_data[3].size();
          size_t longest_pid          = header_data[4].size();
          size_t longest_current_host = header_data[5].size();
          size_t longest_state        = header_data[6].size();

          auto task_list = ecalsys_instance->GetTaskList();
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

            auto monitoring_state = task->GetMonitoringTaskState();
            std::string state, level;
            monitoring_state.ToString(state, level);
            this_task_string_data.push_back(state);

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

          std::stringstream list_ss;

          list_ss << std::right
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

          list_ss << std::string(longest_launch_order,  '-') << " "
                  << std::string(longest_id,            '-') << " "
                  << std::string(longest_task_name,     '-') << " "
                  << std::string(longest_target_host,   '-') << " "
                  << std::string(longest_pid,           '-') << " "
                  << std::string(longest_current_host,  '-') << " "
                  << std::string(longest_state,         '-') << " "
                  << std::string(header_data[7].size(), '-')
                  << std::endl;

          for (const auto& line : string_data)
          {
            list_ss << std::right
                    << std::setw(longest_launch_order) << line[0] << " "
                    << std::left
                    << std::setw(longest_id)           << line[1] << " "
                    << std::setw(longest_task_name)    << line[2] << " "
                    << std::setw(longest_target_host)  << line[3] << " "
                    << std::setw(longest_pid)          << line[4] << " "
                    << std::setw(longest_current_host) << line[5] << " "
                    << std::setw(longest_state)        << line[6] << " "
                    << line[7]
                    << std::endl;
          }

          std::cout << list_ss.str();

          return Error::ErrorCode::OK;
        }
        else if (argv.size() == 1)
        {
          std::list<std::shared_ptr<EcalSysTask>> task_list;
          auto error = ToTaskList(ecalsys_instance, argv, task_list);

          if (error)
          {
            return error;
          }
          else
          {
            auto task = task_list.front();

            std::stringstream info_ss;

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

            info_ss << "ID:                  " << task->GetId() << std::endl;
            info_ss << "Launch#:             " << task->GetLaunchOrder() << std::endl;
            info_ss << "Task Name:           " << task->GetName() << std::endl;
            info_ss << "Target Host:         " << task->GetTarget() << std::endl;
            info_ss << "Runner:              " << task->GetRunner()->GetName() << " (" << task->GetRunner()->GetId() << ")" << std::endl;
            info_ss << "Path:                " << task->GetAlgoPath() << std::endl;
            info_ss << "Working Dir:         " << task->GetWorkingDir() << std::endl;
            info_ss << "Command line:        " << task->GetCommandLineArguments() << std::endl;
            info_ss << "Visibility:          " << visibility_string << std::endl;
            info_ss << "Waiting time:        " << std::chrono::duration_cast<std::chrono::duration<double>>(task->GetTimeoutAfterStart()).count() << " s" << std::endl;
            info_ss << "Monitoring enabled:  " << (task->IsMonitoringEnabled() ? "Yes" : "No") << std::endl;
            info_ss << "Restart by severity: " << (task->IsRestartBySeverityEnabled() ? "Yes: " + task->GetRestartAtSeverity().ToString() : "No") << std::endl;
            info_ss << "PID:                 " << (pids.empty() ? "" : std::to_string(pids[0])) << std::endl;
            info_ss << "Current Host:        " << task->GetHostStartedOn() << std::endl;
            info_ss << "State:               " << (task_state.severity == eCAL_Process_eSeverity::proc_sev_unknown ? state : state + " " + level) << std::endl;
            info_ss << "Info:                " << task_state.info << std::endl;


            std::cout << info_ss.str();
            
            return eCAL::sys::Error::ErrorCode::OK;
          }
          return Error::ErrorCode::OK;

        }
        else
        {
          return Error(Error::ErrorCode::TOO_MANY_PARAMETERS, EcalUtils::String::Join(" ", std::vector<std::string>(std::next(argv.begin()), argv.end())));
        }
      }
    }
  }
}