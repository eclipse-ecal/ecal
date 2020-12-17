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

#include "config_manager.h"

#include <map>
#include <regex>

#include "esys_cfg.h"
#include "esys_cfg_parser.h"
#include "ecal_utils/string.h"

#include "ecalsys/ecal_sys_logger.h"

ConfigManager::~ConfigManager(){}

bool ConfigManager::LoadConfig(EcalSys& ecalsys, const std::string& path, bool append)
{
  // Currently we only have Version 1.0 configs. As soon as we add a newer
  // format, this function has to be refactored.

  eCAL::Sys::Config::CConfiguration config;
  std::string exception_message = "";

  bool success = false;
  try {
    success = eCAL::Sys::Config::Open(path, config, exception_message);
  }
  catch (...) {
    // using the exception message as parameter feels unnecessary. We just add it to the exception.
    success = false;
  }

  if (!success)
  {
    throw std::runtime_error(exception_message.c_str());
  }
  else
  {
    if (!append)
    {
      // Remove everything from the current config
      ecalsys.ClearConfig();
    }

    // In the case of appending configs, there might be ID conflicts. Thus we keep a map of ID in the case we have to assign a new ID.
    std::map<uint32_t, uint32_t> runner_id_map;

    // Add Runners
    for (const auto runner_config : config.runners_) {
      // Silently remove the EXE and BAT runner as those were unnecessary.
      if (((runner_config.name_ == eCAL::Sys::Config::EXE_RUNNER) || (runner_config.name_ == eCAL::Sys::Config::BAT_RUNNER))
        && runner_config.default_algo_dir_  == ""
        && runner_config.load_cmd_argument_ == ""
        && runner_config.path_              == ""
        )
      {
        continue;
      }

      std::shared_ptr<EcalSysRunner> runner(new EcalSysRunner((uint32_t)runner_config.GetId(), runner_config.name_, runner_config.path_, runner_config.default_algo_dir_, runner_config.load_cmd_argument_));

      if (ecalsys.GetRunner((uint32_t)runner_config.GetId()))
      {
        // A Runner with that ID already exists. Thus, we let ecalsys assign a new one
        ecalsys.AddRunner(runner, false);
      }
      else
      {
        ecalsys.AddRunner(runner, true);
      }
      runner_id_map[runner_config.GetId()] = runner->GetId();
    }

    // Add Tasks
    for (const auto& task_config : config.tasks_) {
      std::shared_ptr<EcalSysTask> new_task(new EcalSysTask());

      // Parse the restart-below-severity
      TaskState restart_below_severity;
      restart_below_severity.FromString(task_config.monitoring_.restart_below_severity_, task_config.monitoring_.restart_below_severity_level_);

      // convert restart-below-severity into restart-at-severity by adding one level
      TaskState restart_at_severity;
      restart_at_severity.FromInt(restart_below_severity.ToInt() + 1);

      // parse the start visibility
      eCAL_Process_eStartMode visibility = eCAL_Process_eStartMode::proc_smode_normal;
      if (EcalUtils::String::Icompare(task_config.start_stop_.visibility_, "Normal")    == true)
        visibility = proc_smode_normal;
      if (EcalUtils::String::Icompare(task_config.start_stop_.visibility_, "Hidden")    == true)
        visibility = proc_smode_hidden;
      if (EcalUtils::String::Icompare(task_config.start_stop_.visibility_, "Minimized") == true)
        visibility = proc_smode_minimized;
      if (EcalUtils::String::Icompare(task_config.start_stop_.visibility_, "Maximized") == true)
        visibility = proc_smode_maximized;

      // Find the correct runner (if the runner was the EXE / BAT runner that we removed earlier we will get a nullpointer, which is exactly what we want, here)
      std::shared_ptr<EcalSysRunner> runner(nullptr);
      for (auto id_pair : runner_id_map)
      {
        if (id_pair.first == (uint32_t)task_config.start_stop_.runner_id_)
        {
          runner = ecalsys.GetRunner(id_pair.second);
        }
      }

      // Set all values
      new_task->SetId                         ((uint32_t)task_config.GetId());
      new_task->SetName                       (task_config.name_);
      new_task->SetTarget                     (task_config.start_stop_.target_);
      new_task->SetAlgoPath                   (task_config.start_stop_.algo_);
      new_task->SetWorkingDir                 (task_config.start_stop_.working_dir_);
      new_task->SetRunner                     (runner);
      new_task->SetLaunchOrder                ((unsigned int)task_config.start_stop_.launch_order_);
      new_task->SetTimeoutAfterStart          (task_config.start_stop_.timeout_);
      new_task->SetVisibility                 (visibility);
      new_task->SetCommandLineArguments       (task_config.start_stop_.additional_cmd_line_args_);
      new_task->SetMonitoringEnabled          (task_config.start_stop_.do_monitor_);
      new_task->SetRestartBySeverityEnabled   (task_config.monitoring_.restart_by_severity_);
      new_task->SetRestartAtSeverity          (restart_at_severity);

      if (ecalsys.GetTask((uint32_t)task_config.GetId()))
      {
        //The ID is already in use
        ecalsys.AddTask(new_task, false);
      }
      else
      {
        ecalsys.AddTask(new_task, true);
      }
    }

    // Add Groups (Called functions in eCALsys v1.0 configs)
    for (const auto& function : config.functions_) {
      std::shared_ptr<TaskGroup> task_group(new TaskGroup());
      auto group_state_list = task_group->GetGroupStateList();

      // Sort the list by the legacy prio
      std::list<eCAL::Sys::Config::CConfiguration::Function::State> sorted_function_state_list = function.states_;
      sorted_function_state_list.sort([](const eCAL::Sys::Config::CConfiguration::Function::State& a, const eCAL::Sys::Config::CConfiguration::Function::State& b) {return a.prio_ < b.prio_; });

      // Fill the Task group with group states
      for (auto config_function_state : sorted_function_state_list)
      {
        std::shared_ptr<TaskGroup::GroupState> group_state(new TaskGroup::GroupState);

        // Parse the Color
        static const std::regex regex_color(std::string("^rgb\\((\\d{1,3}), (\\d{1,3}), (\\d{1,3})\\)$"), std::regex::icase);
        std::smatch match;
        if (std::regex_match(config_function_state.colour_, match, regex_color) == true && match.size() == 4)
        {
          int red   = (atoi(match[1].str().c_str()));
          int green = (atoi(match[2].str().c_str()));
          int blue  = (atoi(match[3].str().c_str()));

          red   = (red   <= 255 ? red : 255);
          green = (green <= 255 ? green : 255);
          blue  = (blue  <= 255 ? blue : 255);

          group_state->SetColor(TaskGroup::GroupState::Color(red, green, blue));
        }
        group_state->SetName(config_function_state.name_);

        // Create List of minimal Severitys
        auto state_list = group_state->GetMinimalStatesList();
        for (auto config_function_task : config_function_state.tasks_)
        {
          unsigned int task_id = config_function_task.GetId();
          auto task = ecalsys.GetTask((uint32_t)task_id);
          TaskState minimal_severity;
          minimal_severity.FromString(config_function_task.severity_, config_function_task.severity_level_);

          if (task)
          {
            state_list.push_back(std::pair<std::shared_ptr<EcalSysTask>, TaskState>(task, minimal_severity));
          }
          else
          {
            EcalSysLogger::Log("Error: Task " + std::to_string(config_function_task.GetId()) + " not found");
          }
        }
        group_state->SetMinimalStatesList(state_list);
        group_state_list.push_back(group_state);
      }
      task_group->SetGroupStateList(group_state_list);
      task_group->SetName(function.name_);
      
      ecalsys.AddTaskGroup(task_group, true);
    }

    // Set options
    if (!append)
    {
      auto options = ecalsys.GetOptions();
      options.check_target_reachability   = config.GetOptions().all_targets_reachable_;
      options.kill_all_on_close           = config.GetOptions().stop_all_on_close_;
      options.use_localhost_for_all_tasks = config.GetOptions().use_all_on_this_host_;
      options.local_tasks_only            = config.GetOptions().use_only_local_host_;
      ecalsys.SetOptions(options);
    }

    return true;
  }
}

bool ConfigManager::SaveConfig(EcalSys& ecalsys, const std::string& path, ConfigVersion version)
{
  if (version == ConfigVersion::v1_0)
  {
    eCAL::Sys::Config::CConfiguration config;

    // Get relevant data from ecalsys
    auto runner_list = ecalsys.GetRunnerList();
    auto task_list   = ecalsys.GetTaskList();
    auto group_list  = ecalsys.GetGroupList();
    auto options     = ecalsys.GetOptions();

    // Sort the lists, to get a deterministic xml
    task_list.sort([](const std::shared_ptr<EcalSysTask>& t1, const std::shared_ptr<EcalSysTask>& t2)
                  {
                    if (t1->GetId() != t2->GetId())
                      return t1->GetId() < t2->GetId();
                    else
                      return t1->GetName() < t2->GetName(); // should never happen
                  });

    runner_list.sort([](const std::shared_ptr<EcalSysRunner>& r1, const std::shared_ptr<EcalSysRunner>& r2)
                  {
                    if (r1->GetId() != r2->GetId())
                      return r1->GetId() < r2->GetId();
                    else
                      return r1->GetName() < r2->GetName(); // should never happen
                  });

    group_list.sort([](const std::shared_ptr<TaskGroup>& g1, const std::shared_ptr<TaskGroup>& g2)
                  {
                    if (g1->GetId() != g2->GetId())
                      return g1->GetId() < g2->GetId();
                    else
                      return g1->GetName() < g2->GetName(); // should never happen
                  });

    // save data in the "old" config struct

    // save runners
    for (auto& runner : runner_list)
    {
      eCAL::Sys::Config::CConfiguration::Runner runner_config((unsigned int) runner->GetId());
      runner_config.name_              = runner->GetName();
      runner_config.path_              = runner->GetPath();
      runner_config.default_algo_dir_  = runner->GetDefaultAlgoDir();
      runner_config.load_cmd_argument_ = runner->GetLoadCmdArgument();
      runner_config.imported_ = false;

      config.runners_.push_back(runner_config);
    }

 // Whoever though that it might be a good idea to clutter ALL namespaces with a #define max should be punished.
#ifdef max
#define max_temp max
#undef max
#endif //max

    // Add the legacy exe runner that does nothing but is required for backwards
    // compatibility. We omit the bat runner, as it does excatly the same thing
    // as the exe runner. Yes, it also does nothing :-)
    // Of course we have to look for a free ID for that runner. For this, we 
    // simply scan all IDs from 1 -> infinity until we find an unused ID.
    uint32_t free_id = 1;
    for (free_id = 1; free_id < std::numeric_limits<uint32_t>::max(); free_id++)
    {
      auto it = std::find_if(runner_list.begin(), runner_list.end(),
        [free_id](const std::shared_ptr<EcalSysRunner> r) {return r->GetId() == free_id; });

      if (it == runner_list.end())
      {
        // The ID is not in use
        break;
      }
    }
    eCAL::Sys::Config::CConfiguration::Runner exe_runner(free_id);
    exe_runner.name_              = eCAL::Sys::Config::EXE_RUNNER;
    exe_runner.path_              = "";
    exe_runner.default_algo_dir_  = "";
    exe_runner.load_cmd_argument_ = "";
    exe_runner.imported_          = false;

    config.runners_.push_back(exe_runner);

#ifdef max_temp
#define max max_temp
#undef max_temp
#endif //max_temp

    // save tasks
    for (auto& task : task_list)
    {
      eCAL::Sys::Config::CConfiguration::Task task_config((unsigned int)task->GetId());

      task_config.name_                                 = task->GetName();
      task_config.start_stop_.target_                   = task->GetTarget();

      auto runner = task->GetRunner();
      uint32_t runner_id;
      if (runner)
      {
        // The runner exists => we use it's ID
        runner_id = runner->GetId();
      }
      else
      {
        // No runner exists => we assign that exe runner we created earlier!
        runner_id = exe_runner.GetId();
      }
      task_config.start_stop_.runner_id_                = (unsigned int)runner_id;
      task_config.start_stop_.algo_                     = task->GetAlgoPath();
      task_config.start_stop_.additional_cmd_line_args_ = task->GetCommandLineArguments();
      task_config.start_stop_.working_dir_              = task->GetWorkingDir();
      task_config.start_stop_.launch_order_             = (size_t) task->GetLaunchOrder();
      task_config.start_stop_.timeout_                  = std::chrono::duration_cast<std::chrono::milliseconds>(task->GetTimeoutAfterStart());

      std::string visibility_string;
      switch (task->GetVisibility())
      {
      case proc_smode_normal:
        visibility_string = "normal";
        break;
      case proc_smode_hidden:
        visibility_string = "hidden";
        break;
      case proc_smode_minimized:
        visibility_string = "minimized";
        break;
      case proc_smode_maximized:
        visibility_string = "maximized";
        break;
      default:
        visibility_string = "normal";
      }
      task_config.start_stop_.visibility_                   = visibility_string;
      task_config.start_stop_.do_monitor_                   = task->IsMonitoringEnabled();

      task_config.monitoring_.restart_by_severity_          = task->IsRestartBySeverityEnabled();


      // Convert the restart *at* severity to a restart *below* severity by subtracting one level
      TaskState restart_below_severity;
      restart_below_severity.FromInt(task->GetRestartAtSeverity().ToInt() - 1);
      std::string restart_below_severity_string, restart_below_severity_level_string;
      restart_below_severity.ToString(restart_below_severity_string, restart_below_severity_level_string);

      task_config.monitoring_.restart_below_severity_       = restart_below_severity_string;
      task_config.monitoring_.restart_below_severity_level_ = restart_below_severity_level_string;

      task_config.monitoring_.restart_by_beaconing_         = false;
      task_config.monitoring_.max_beacon_response_time_     = std::chrono::microseconds(0);
      
      task_config.imported_ = false;

      config.tasks_.push_back(task_config);
    }
    
    // save groups (called functions in v1.0 configs)
    for (auto& group : group_list)
    {
      // And again solve that 32->64->32bit problem
      eCAL::Sys::Config::CConfiguration::Function function_config((unsigned int)group->GetId());

      function_config.name_     = group->GetName();
      function_config.imported_ = false;
      
      // First add a list of all Tasks that is used by any state. One _could_
      // simply compute that list instead of saving it to the config, as any
      // information we are adding here is 100% redundant. But the v1.0 config
      // says we have to save a list of all tasks, so we do.
      auto all_tasks = group->GetAllTasks();
      for (auto& task : all_tasks)
      {
        eCAL::Sys::Config::CConfiguration::Function::Task task_config((unsigned int)task->GetId());
        function_config.tasks_.push_back(task_config);
      }
       
      // Add all group states. The position in the list of our states represent the priority of the function config. The user had to set that priority manually in the old ecalsys version.
      int priority = 0;
      auto group_state_list = group->GetGroupStateList();
      for (auto& group_state : group_state_list)
      {
        // nobody cares about the IDs, so we just reuse the priority
        eCAL::Sys::Config::CConfiguration::Function::State function_state_config(priority);
        function_state_config.name_ = group_state->GetName();
        function_state_config.prio_ = priority;
        auto color = group_state->GetColor();
        function_state_config.colour_
          = "rgb("
          + std::to_string(color.red)
          + ", "
          + std::to_string(color.green)
          + ", "
          + std::to_string(color.blue)
          + ")";
        priority++;

        // Add the tasks to the group_states.
        auto minimal_task_states_list = group_state->GetMinimalStatesList();
        for (auto& minmimal_task_state : minimal_task_states_list)
        {
          std::string minimal_severity_string, minimal_severity_level_string;
          minmimal_task_state.second.ToString(minimal_severity_string, minimal_severity_level_string);

          eCAL::Sys::Config::CConfiguration::Function::State::Task 
            function_state_task_config((unsigned int)minmimal_task_state.first->GetId(), minimal_severity_string, minimal_severity_level_string);
          function_state_config.tasks_.push_back(function_state_task_config);
        }

        function_config.states_.push_back(function_state_config);
      }
      config.functions_.push_back(function_config);
    }
    
    // Save options
    eCAL::Sys::Config::CConfiguration::Options options_config;
    options_config.all_targets_reachable_ = options.check_target_reachability;
    options_config.stop_all_on_close_     = options.kill_all_on_close;
    options_config.use_all_on_this_host_  = options.use_localhost_for_all_tasks;
    options_config.use_only_local_host_   = options.local_tasks_only;
    config.SetOptions(options_config);

    // Write everything to a file
    return eCAL::Sys::Config::Save(path, config);
  }
  else
  {
    throw std::invalid_argument("Target version undefined");
  }
}