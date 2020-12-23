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

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include "ecalsys/ecal_sys.h"
#include "ecalsys/task/ecal_sys_task.h"
#include "ecalsys/task/ecal_sys_runner.h"
#include "ecalsys/task/task_group.h"
#include "ecalsys/task/task_state.h"
#include "ecalsys/config/config_version.h"

#include "ecal/ecal_process_mode.h"

namespace py = pybind11;

PYBIND11_MODULE(ecal_sys_python, Sys) {
  using namespace pybind11::literals;
  
  Sys.doc() = "eCAL Sys functionality";
  /*
  wrapping status:
    EcalSys                                       DONE
      EcalSysTask                                 DONE
        eCAL_Process_eStartMode                   DONE
        StartStopState                            DONE
        TaskState                                 DONE
          eCAL_Process_eSeverity                  DONE
          eCAL_Process_eSeverity_Level            DONE
      EcalSysRunner                               DONE
      EcalSys::Options                            DONE
      TaskGroup                                   DONE
        GroupState                                DONE
        Color                                     DONE
  */


  py::enum_<ConfigVersion>(Sys, "config_version")
    .value("v1_0", ConfigVersion::v1_0);

  py::class_<EcalSys::Options>(Sys, "Options")
    .def(py::init<>())
    .def_readwrite("kill_all_on_close", &EcalSys::Options::kill_all_on_close, "bool")
    .def_readwrite("use_localhost_for_all_tasks", &EcalSys::Options::use_localhost_for_all_tasks, "bool")
    .def_readwrite("local_tasks_only", &EcalSys::Options::local_tasks_only, "bool")
    .def_readwrite("check_target_reachability", &EcalSys::Options::check_target_reachability, "bool");

  py::enum_<eCAL_Process_eStartMode>(Sys, "eCAL_Process_eStartMode")
    .value("proc_smode_normal", eCAL_Process_eStartMode::proc_smode_normal)
    .value("proc_smode_hidden", eCAL_Process_eStartMode::proc_smode_hidden)
    .value("proc_smode_minimized", eCAL_Process_eStartMode::proc_smode_minimized)
    .value("proc_smode_maximized", eCAL_Process_eStartMode::proc_smode_maximized);

  py::enum_<eCAL_Process_eSeverity>(Sys, "eCAL_Process_eSeverity",
    "\n"
    "\n   Process severity"
    "\n")
    .value("proc_sev_unknown", eCAL_Process_eSeverity::proc_sev_unknown)
    .value("proc_sev_healthy", eCAL_Process_eSeverity::proc_sev_healthy)
    .value("proc_sev_warning", eCAL_Process_eSeverity::proc_sev_warning)
    .value("proc_sev_critical", eCAL_Process_eSeverity::proc_sev_critical)
    .value("proc_sev_failed", eCAL_Process_eSeverity::proc_sev_failed);

  py::enum_<eCAL_Process_eSeverity_Level>(Sys, "eCAL_Process_eSeverity_Level",
    "\n"
    "\n  Process Severity Level"
    "\n"
    "\n enumerations for ECAL_API::SetState functionality"
    "\n where the lowest process severity is generally proc_sev_level1"
    "\n")
    .value("proc_sev_level1", eCAL_Process_eSeverity_Level::proc_sev_level1)
    .value("proc_sev_level2", eCAL_Process_eSeverity_Level::proc_sev_level2)
    .value("proc_sev_level3", eCAL_Process_eSeverity_Level::proc_sev_level3)
    .value("proc_sev_level4", eCAL_Process_eSeverity_Level::proc_sev_level4)
    .value("proc_sev_level5", eCAL_Process_eSeverity_Level::proc_sev_level5);

  //declaration of wrapped classes
  py::class_<EcalSysTask, std::shared_ptr<EcalSysTask>> ecal_sys_task(Sys, "eCALSysTask");
  py::class_<EcalSysRunner, std::shared_ptr<EcalSysRunner>>ecal_sys_runner(Sys, "eCALSysRunner");
  py::class_<TaskGroup, std::shared_ptr<TaskGroup>>ecal_sys_task_group(Sys, "TaskGroup");
  py::class_<EcalSys> ecal_sys(Sys, "eCALSys");
  py::class_<TaskState> task_state(Sys, "TaskState");
  py::class_<TaskGroup::GroupState, std::shared_ptr<TaskGroup::GroupState>> group_state(ecal_sys_task_group, "GroupState");

  py::enum_<EcalSysTask::StartStopState>(ecal_sys_task, "StartStopState")
    .value("NotStarted", EcalSysTask::StartStopState::NotStarted)
    .value("Started_Successfully", EcalSysTask::StartStopState::Started_Successfully)
    .value("Started_Failed", EcalSysTask::StartStopState::Started_Failed)
    .value("Stopped", EcalSysTask::StartStopState::Stopped);

  ecal_sys_runner
    .def(py::init<uint32_t, const std::string&, const std::string&, const std::string&, const std::string&>(),
      "\n"
      "\n  Crates a new eCAL Sys Runner with the given values"
      "\n Args:"
      "\n\tparam1 (int): The ID of the runner"
      "\n\tparam2 (string): The name of the runner (mainly for displaying it in the GUI)"
      "\n\tparam3 (string): Path to the runner-executable"
      "\n\tparam4 (string): Default path where the .dll / .so files are located"
      "\n\tparam5 (string): Command line argument for loading files"
      "\n")
    .def(py::init<>(),
      "\n"
      "\n  Creates a new eCAL Sys Runner with empty values and a randomly generated id."
      "\n")
    .def("set_id", &EcalSysRunner::SetId,
      "\n"
      "\n  Sets the ID of the runner"
      "\n Args:"
      "\n\tparam (int): The new ID of the runner"
      "\n")
    .def("set_name", &EcalSysRunner::SetName,
      "\n"
      "\n  Sets the name of the runner, that is displayed in the GUI"
      "\n Args:"
      "\n\tparam (string): The new name of the runner"
      "\n")
    .def("set_path", &EcalSysRunner::SetPath,
      "\n"
      "\n  Sets the path to the runner's executable"
      "\n Args:"
      "\n\tparam (string): The new executable path"
      "\n")
    .def("set_default_algo_dir", &EcalSysRunner::SetDefaultAlgoDir,
      "\n"
      "\n  Sets the default directory where the .dll / .so files associdated with this runner are located"
      "\n Args:"
      "\n\tparam (string): the new default algorithm directory"
      "\n")
    .def("set_load_cmd_argument", &EcalSysRunner::SetLoadCmdArgument,
      "\n"
      "\n  Sets the command line argument that is used to load files with this runner (e.g. \"--dll\")"
      "\n Args:"
      "\n\tparam (string): the new command line"
      "\n")
    .def("get_id", &EcalSysRunner::GetId,
      "\n"
      "\n  Gets the ID of this runner"
      "\n Returns: \n\tint: the ID of this runner"
      "\n")
    .def("get_name", &EcalSysRunner::GetName,
      "\n"
      "\n  Gets the name of this runner that is displayed in the GUI"
      "\n Returns: \n\tstring: the name of this runner"
      "\n")
    .def("get_path", &EcalSysRunner::GetPath,
      "\n"
      "\n  Gets the path to the runner's executable"
      "\n Returns: \n\tstring: the path to the runner's executable"
      "\n")
    .def("get_default_algo_dir", &EcalSysRunner::GetDefaultAlgoDir,
      "\n"
      "\n  Gets the default path to the directory where the .dll / .so / etc. files are located"
      "\n Returns: \n\tstring: the default algorithm directory"
      "\n")
    .def("get_load_cmd_argument", &EcalSysRunner::GetLoadCmdArgument,
      "\n"
      "\n  Gets the command line argument for loading files"
      "\n Returns: \n\tstring: the command line argument for loading files"
      "\n");


  
  
  py::class_<TaskGroup::GroupState::Color>(group_state, "Color",
    "\n  Color value displayed in the eCALSysGUI")
    .def(py::init<>())
    .def(py::init<int, int, int>(),
      "\n Args:"
      "\n\t param1 (int): red"
      "\n\t param2 (int): green"
      "\n\t param3 (int): blue")
    .def(py::self == py::self)
    .def(py::self != py::self);

  group_state
    .def(py::init<>())
    .def("evaluate", &TaskGroup::GroupState::Evaluate,
      "\n"
      "\n  Evaluate if this state is active"
      "\n"
      "\n The evaluation is done by comparing all tasks that are assigned to this"
      "\n state with their according minimum health level. The evaluation results"
      "\n to true, if:"
      "\n  1. No state has unknown severity"
      "\n  2. All states are healthier than (or equal to) their minimum severity"
      "\n"
      "\n Returns: \n\tbool: if this state is active"
      "\n")
    .def("set_name", &TaskGroup::GroupState::SetName,
      "\n"
      "\n  Sets the name of this state"
      "\nArgs:"
      "\n\tparam (string):the new name"
      "\n")
    .def("get_name", &TaskGroup::GroupState::GetName,
      "\n"
      "\n  Gets the name of this state"
      "\n Returns: \n\tstring: the name of this state"
      "\n")
    .def("set_color", &TaskGroup::GroupState::SetColor,
      "\n"
      "\n  Sets the color of this state thas is displayed in the GUI when the state is active"
      "\nArgs:"
      "\n\nparam Color: The new color"
      "\n")
    .def("get_color", &TaskGroup::GroupState::GetColor,
      "\n"
      "\n  Gets the color of this state that is displayed in the GUI when the state is active"
      "\n Returns: \n\t(Color): The color that should be displayed in the GUI whe this GroupState is active"
      "\n")
    .def("get_minimal_states_list", &TaskGroup::GroupState::GetMinimalStatesList,
      "\n"
      "\n  Returns the list of (task, minimal_severity) pairs as copy"
      "\n"
      "\n Returns: \n\tlist(pair(EcalSysTask,TaskState): a list of (task, minimal severity) pairs"
      "\n")
    .def("set_minimal_states_list", &TaskGroup::GroupState::SetMinimalStatesList,
      "\n"
      "\n  Sets the list of (task, minimal_severity) pairs of this TaskGroup"
      "\n"
      "\n This list will be evaluated when evaluating this TaskGroupState."
      "\nArgs:"
      "\n\tparam (list(pair(EcalSysTask,TaskState)): The new list of minimal severity for all tasks"
      "\n");

  ecal_sys_task_group
    .def(py::init<>())
    .def("evaluate", &TaskGroup::Evaluate,
      "\n"
      "\n  Evaluates the possible states and returns the active state"
      "\n"
      "\n If there is no state active, a Nullpointer will be returned. The states are"
      "\n evaluated in the order of their appearence in the state list"
      "\n get_group_state_list. The first state that is active is considered to be"
      "\n the current state."
      "\n"
      "\n Returns: \n\tGroupState: The active state or a nullpointer if no state is active"
      "\n")
    .def("get_all_tasks", &TaskGroup::GetAllTasks,
      "\n"
      "\n  Creates a set of all tasks that are referenced by any possible group state"
      "\n Returns: \n\tset(EcalSysTask): a set of all Tasks"
      "\n")
    .def("set_name", &TaskGroup::SetName,
      "\n"
      "\n  Sets the name of this TaskGroup"
      "\n Args:"
      "\n\tparam (string): The new name of this TaskGroup"
      "\n")
    .def("get_name", &TaskGroup::GetName,
      "\n"
      "\n"
      "\n  Gets the name of this TaskGroup"
      "\n Returns: \n\tstring: The name of this TaskGroup"
      "\n")
    .def("set_id", &TaskGroup::SetId,
      "\n"
      "\n  Sets the ID of the TaskGroup."
      "\n"
      "\n The ID should not be set manually, as it will be set by Ecalsys when the"
      "\n group is added."
      "\n The ID actually is not used for anything and is just there to keep"
      "\n compatibility."
      "\n Args:"
      "\n\tparam (int): the new ID"
      "\n")
    .def("get_id", &TaskGroup::GetId,
      "\n"
      "\n  Gets the ID of the TaskGroup."
      "\n"
      "\n The ID actually is not used for anything and is just there to keep"
      "\n compatibility."
      "\n"
      "\n Returns: \n\tint: the ID of the TaskGroup"
      "\n")
    .def("get_group_state_list", &TaskGroup::GetGroupStateList,
      "\n"
      "\n  Returns the list of possible states as copy."
      "\n"
      "\n When evaluating the Taskgroup with @see Evaluate(), the order of the States"
      "\n are used to determine which state is the current state. The first state in"
      "\n the list beeing active is considered to be the active state."
      "\n"
      "\n Returns: \n\t list(GroupState): the list of possible states"
      "\n")
    .def("set_group_state_list", &TaskGroup::SetGroupStateList,
      "\n"
      "\n  Sets the list of possible states"
      "\n"
      "\n When evaluating the Taskgroup with @see Evaluate(), the order of the States"
      "\n are used to determine which state is the current state. The first state in"
      "\n the list beeing active is considered to be the active state."
      "\n Args:"
      "\n\tparam (list(GroupState)): The new list of possible states"
      "\n");

 
    task_state
    .def(py::init<>(),
      "\n"
      "\n Creates a new State (Unknown Level1) without info message."
      "\n")
    .def(py::init<eCAL_Process_eSeverity, eCAL_Process_eSeverity_Level>(),
      "\n"
      "\n Creates a new State with the given severity and without info message."
      "\n Args:"
      "\n\tparam1 (eCAL_Process_eSeverity): The severity of the new state"
      "\n\tparam2 (eCAL_Process_eSeverity_Level): The severity level of the new state"
      "\n")
    .def(py::init<int>(),
      "\n"
      "\n Creates a new State by parsing the given combined severity and without info message. see from_int}"
      "\n Args:"
      "\n\tparam (int): The combined severity / severity level as integer"
      "\n")
    .def("from_string", &TaskState::FromString,
      "\n"
      "\n Parses the given strings and sets the severity and severity level accordingly"
      "\n"
      "\n The Strings are case-insensitive and can be:"
      "\n  Severity:       Unknown, Healthy, Warning, Critical, Failed"
      "\n  Severity Level: Level1, Level2, Level3, Level4, Level5"
      "\n                       ^ (there may be a space after each \"Level\")"
      "\n"
      "\n The default is Unknown Level1, if the string could not be parsed."
      "\n Args:"
      "\n\tparam1 (string): The Severity as string"
      "\n\tparam2 (string): The Severity Level as string"
      "\n"
      "\n Returns: \n\tbool: wether both strings could be parsed correctly"
      "\n")
    .def("to_string", py::overload_cast<std::string&, std::string&> (&TaskState::ToString, py::const_),
      "\n"
      "\n Converts the Severity and Severity Level to a string"
      "\n Args:"
      "\n\tparam1[OUT] (string): The object to save the string representation of the severity to"
      "\n\tparam2[OUT] (string): The object to save the string representation of the severity level to"
      "\n")
    .def("from_int", &TaskState::FromInt,
      "\n"
      "\n Sets the severity and serverity_level to the value parsed from the combined severity integer."
      "\n"
      "\n The combined severity can range from 0 to 24. Thus, there are 25 values"
      "\n representing all 25 combinations of severities and severity levels:"
      "\n"
      "\n  0: Unknown Level0"
      "\n  1: Unknown Level1"
      "\n    ..."
      "\n 23: Failed  Level4"
      "\n 24: Failed  Level5"
      "\n Args:"
      "\n\tparam (int): The combined severity and severity level as interger"
      "\n")
    .def("to_int", &TaskState::ToInt,
      "\n"
      "\n Converts the severity and serverity_level to one number ranging from 0 to 24."
      "\n"
      "\n The combined severity can range from 0 to 24. Thus, there are 25 values"
      "\n representing all 25 combinations of severities and severity levels:"
      "\n"
      "\n  0: Unknown Level0"
      "\n  1: Unknown Level1"
      "\n    ..."
      "\n 23: Failed  Level4"
      "\n 24: Failed  Level5"
      "\n"
      "\n Returns: \n\tint: the combined severity and severity level as int"
      "\n")
    .def("to_string", py::overload_cast<>(&TaskState::ToString),
      "\n"
      "\n Creates a String representation consisting of the Severity, the severity level and the info."
      "\n"
      "\n Example:"
      "\n    Healthy Level1 (I feel good)"
      "\n"
      "\n Returns: \n\tstring: A string representation of the state"
      "\n")
    .def(py::self == py::self)
    .def(py::self != py::self)
    .def(py::self < py::self)
    .def(py::self > py::self)
    .def(py::self <= py::self)
    .def(py::self >= py::self);





    ecal_sys_task
      .def(py::init<>())
      .def("get_id", &EcalSysTask::GetId,
        "\n Returns: \n\t int: The ID of this Task")
      .def("get_name", &EcalSysTask::GetName,
        "\n Returns: \n\t string: The configured name of this task")
      .def("get_algo_path", &EcalSysTask::GetAlgoPath,
        "\nIf a runner is set, it will be given to the runner as argument. Otherwise, the algo will be executed as executable. "
        "\n Returns: \n\t string: The configured path to the algorithm, e.g. an executable binary or shared library.")
      .def("get_target", &EcalSysTask::GetTarget,
        "\n Returns: \n\t string: The configured Target-Hostname")
      .def("get_working_dir", &EcalSysTask::GetWorkingDir,
        "\n Returns: \n\t string: The configured working directory")
      .def("get_runner", &EcalSysTask::GetRunner,
        "\n Returns: \n\t EcalSysRunner : The configured runner. If no runner is set, this is a shared nullpointer. ")
      .def("get_launch_order", &EcalSysTask::GetLaunchOrder,
        "\n Returns: \n\t int : The configured launch order of this task. If multiple tasks are started simultaniously, tasks with a higher launch order will be started later. ")
      .def("get_timeout_after_start", &EcalSysTask::GetTimeoutAfterStart,
        "\n Returns: \n\t nanoseconds : The configured time that will be used to artificially prolong the time that the task needs to start. This is relevant, if multiple tasks are started simultaniously, as it will delay tasks with a higher launch order.")
      .def("get_visibility", &EcalSysTask::GetVisibility,
        "\n Returns: \n\t eCAL_Process_eStartMode : The configured startup-visibility when starting this Task on a Windows host.") //eCAL_Process_eStartMode enum wrap
      .def("get_command_line_arguments", &EcalSysTask::GetCommandLineArguments,
        "\n Returns: \n\t string : The configured arguments that will be given to the task when being started.")
      .def("is_monitor_enabled", &EcalSysTask::IsMonitoringEnabled,
        "\n Returns: \n\t bool: Whether monitoring is enabled. When true, this task will be monitored by the monitoring thread and the task state will be set accordingly. This is a requirement for the restart-by-severity functionality.")
      .def("is_restart_by_severity_enabled", &EcalSysTask::IsRestartBySeverityEnabled,
        "\n Returns: \n\t bool: Whether this tasks may be restarted by the monitoring thread, if it's severity reaches a certain state. This option also requires monitoring to be enabled.")
      .def("get_restart_at_severity", &EcalSysTask::GetRestartAtSeverity,
        "\n Returns: \n\t TaskState: The configured state, at which the task will be restarted, if the restart-by-severity option is enabled. This option also requires monitoring to be enabled.") // TaskState class wrap
      .def("set_id", &EcalSysTask::SetId,
        "\n  Sets the Task ID. You should propably not set the ID manually, as it is already set when adding a task to an EcalSys instance. "
        "\nArgs: \n\t param1 (int): ID of task")
      .def("set_name", &EcalSysTask::SetName,
        "\n  Sets the name of the task. The name is only relevant for logging and displaying it in the console or GUI"
        "\nArgs: \n\t param1 (string): name of task")
      .def("set_target", &EcalSysTask::SetTarget,
        "\n  Sets the host, that this task is supposed to start on by default. "
        "\nArgs: \n\t param1 (string): host")
      .def("set_algo_path", &EcalSysTask::SetAlgoPath,
        "\n  Sets the path to the algoright. This can e.g. be an executable (if the task does not have a runner) or some other binary or script that gets executed by the runner. "
        "\nArgs: \n\t param1 (string): path")
      .def("set_working_dir", &EcalSysTask::SetWorkingDir,
        "\n  Sets the working directory that will be used when starting the task. "
        "\nArgs: \n\t param1 (string): directory")
      .def("set_runner", &EcalSysTask::SetRunner,
        "\n  Sets the runner. Can be set to a shared nullpointer, if this task is not supposed to have a runner. "
        "\nArgs: \n\t param1 (EcalSysRunner): Runner to be set")
      .def("set_launch_order", &EcalSysTask::SetLaunchOrder,
        "\n  Sets the launch order of this task. If multiple tasks are started simultaniously, tasks with a higher launch order will be started later. "
        "\nArgs: \n\t param1 (int): launch order")
      .def("set_timeout_after_start", &EcalSysTask::SetTimeoutAfterStart,
        "\n  Sets a time that will be used to artificially prolong the time that the task needs to start. This is relevant, if multiple tasks are started simultaniously, as it will delay tasks with a higher launch order. "
        "\nArgs: \n\t param1 (nanoseconds): timeout")
      .def("set_visibility", &EcalSysTask::SetVisibility,
        "\n  Sets the visibility when starting the task on a Windows machine. "
        "\nArgs: \n\t param1 (eCAL_Process_eStartMode): visibility")
      .def("set_command_line_arguments", &EcalSysTask::SetCommandLineArguments,
        "\n  Sets the arguments that will be given to the task when being started. "
        "\nArgs: \n\t param1 (string): command line arguments")
      .def("set_monitoring_enabled", &EcalSysTask::SetMonitoringEnabled,
        "\n  Sets whether this task will be monitored by the monitoring thread. If enabled, the task-state will be updated periodically. This also affects whether the restart-by-severity functions are available. "
        "\nArgs: \n\t param1 (bool): enabled")
      .def("set_restart_by_severity_enabled", &EcalSysTask::SetRestartBySeverityEnabled,
        "\n  Sets Whether this task shall be restarted by the monitoring thread, if it's severity reaches a certain state. This option also requires monitoring to be enabled."
        "\nArgs: \n\t param1 (bool): enabled")
      .def("set_restart_at_severity", &EcalSysTask::SetRestartAtSeverity,
        "\n  Sets the severity, at which the task will be restarted, if the restart-by-severity option is enabled. This option also requires monitoring to be enabled."
        "\nArgs: \n\t param1 (TaskState): severity")
      .def("start", &EcalSysTask::Start,
        "\n Start the task"
        "\n"
        "\n Starts the task. If the task is already running, nothing will happen."
        "\n"
        "\n By default, this function does not block and return immediatelly. Thus,"
        "\n this function always returns a success (true), if the task is not already"
        "\n starting / stopping / restarting."
        "\n When the blocking option is set, the \"actual\" success state is returned."
        "\n"
        "\n The task is by default started on the configured target host. This can"
        "\n temporarily be overridden by setting the target_override."
        "\n"
        "\n As return value it is reported, whether starting the task has been"
        "\n successfull. If the blocking parameter has been set to false, this function"
        "\n returns before the task has finished starting, thus always returning true"
        "\n unless the task is currently already starting / stopping / restarting."
        "\n\nArgs:"
        "\n\tparam1 (string): target_override An optional hostname. When set, the task is started on this host. When empty, the task is started on the configured target."
        "\n\tparam2 (bool): When true, this function will block until the task has been started."
        "\n"
        "\n Returns: \n\t bool: Whether starting the task has been successfull."
        "\n",
        "target_override"_a = "",
        "blocking"_a = false)
      .def("stop", &EcalSysTask::Stop,
        "\n"
        "\n Stops the task"
        "\n"
        "\n Stops the task. If the task is not running, nothing will happen."
        "\n"
        "\n A Task can be stopped by an eCAL Shutdown request or it can be killed by"
        "\n means of the operating system. A shutdown request aims to give the task"
        "\n a chance to perform a gracefull shutdown. As this has to be implemented"
        "\n by each task, not all task might react to the request."
        "\n"
        "\n By default, the hostname and Process ID are used to stop the process. If"
        "\n the by_name option is set, the process is stopped by it's executable name."
        "\n This should only be used in very rare occasions, as it will also stop all"
        "\n other processes on that machine with the same name. E.g. stopping a process"
        "\n by name that has a runner, will also stop all other instances of that"
        "\n runner."
        "\n"
        "\n As return value, it is reported whether stopping the process has been"
        "\n successfull. If blocking was set to false, the only way for this function"
        "\n to report a failure is if the task is currently already starting /"
        "\n stopping / restarting."
        "\n Note that a return value true (= success) does not tell whether the process"
        "\n has actually stopped! Especially if only a shutdown request shall be sent,"
        "\n sending this request may be successfull, although the task does not react"
        "\n to it."
        "\nArgs:"
        "\n\tparam1 (bool): Whether an eCAL Shutdown request will be sent to the task."
        "\n\tparam2 (bool): Whether the process will be hard-killed (after sending the shutdown request, if that option is also set)"
        "\n\tparam3 (bool): Whether the process should be identified by it's executable's name only. This may cause collateral damage and also stop other processes!"
        "\n\tparam4 (bool): Whether this function shall block, until the starting procedure has finished. This affects the return value."
        "\n\tparam5 (nanoseconds): How much time will be given to the task after the shutdown request to perform a gracefull shutdown, before killing it. Only relevant, if both request_shutdown and kill_process are set."
        "\n"
        "\n Returns: \n\t bool: Whether stopping the task has been successfull."
        "\n",
        "request_shutdown"_a,
        "kill_process"_a,
        "by_name"_a = false,
        "blocking"_a = false,
        "wait_for_shutdown"_a = std::chrono::seconds(3))
      .def("restart", &EcalSysTask::Restart,
        "\n"
        "\n Restarts the task"
        "\n"
        "\n Restarts the task by stopping it first and starting it afterwards. If the"
        "\n Task is not running, it will only be started."
        "\n"
        "\n A Task can be stopped by an eCAL Shutdown request or it can be killed by"
        "\n means of the operating system. A shutdown request aims to give the task"
        "\n a chance to perform a gracefull shutdown. As this has to be implemented"
        "\n by each task, not all task might react to the request."
        "\n"
        "\n By default, the hostname and Process ID are used to stop the process."
        "\n"
        "\n As return value, it is reported whether both stopping and starting the"
        "\n process has been successfull. If blocking was set to false, the only way"
        "\n for this function to report a failure is if the task is currently already"
        "\n starting / stopping / restarting."
        "\n"
        "\n Also see start and stop"
        "\nArgs:"
        "\n\tparam1 (bool): Whether an eCAL Shutdown request will be sent to the task."
        "\n\tparam2 (bool): Whether the process will be hard-killed (after sending the shutdown request, if that option is also set)"
        "\n\tparam3 (bool): Whether the process should be identified by it's executable's name only. This may cause collateral damage and also stop other processes!"
        "\n\tparam4 (string): An optional hostname. When set, the task is started on this host. When empty, the task is started on the configured target."
        "\n\tparam5 (bool): Whether this function shall block, until the starting procedure has finished. This affects the return value."
        "\n"
        "\n Returns: \n\t bool: Whether both stopping and starting has been successfull."
        "\n",
        "request_shutdown"_a,
        "kill_process"_a,
        "target_override"_a = "",
        "blocking"_a = false)
      .def("cancel_start_stop_procedure", &EcalSysTask::CancelStartStopProcedure,
        "\n"
        "\n Interrupts all Start / Stop / Restart procedures, effective leaving the task in an undefined state."
        "\n"
        "\n Note that the procedures may need some time to actually stop. If it is"
        "\n important to wait for them to actually stop, use"
        "\n see wait_for_start_stop_procedure."
        "\n")
      .def("is_start_stop_procedure_running", &EcalSysTask::IsStartStopProcedureRunning,
        "\n  Tells whether this task is currently starting / stopping / restarting"
        "\n"
        "\n Returns: \n\t bool: True if this task is currently starting / stopping / restarting"
        "\n")
      .def("wait_for_start_stop_procedure", &EcalSysTask::WaitForStartStopProcedure,
        "\n"
        "\n Blocks until the currently active  start/stop/restart procedure has finished"
        "\n"
        "\n If the task is not currently starting / stopping / restarting, this"
        "\n function immediatelly returns (but returns None)."
        "\n")
      .def("get_process_start_environment", &EcalSysTask::GetProcessStartEnvironment,
        "\n"
        "\n Parses the configuration of this task as well as it's runner and creates all information required for starting the task."
        "\n"
        "\n If the runner's path and load argument or the task's path and command line"
        "\n contain environment variables, those variables are expanded."
        "\nArgs:"
        "\n\tparam[out]1 (string): The path to the task's executable, either taken from a runner or from the algo path."
        "\n\tparam[out]2 (string): The paramters for the executable (a combination from the runner's parameter, the algo path and the task's command line parameters). May be empty."
        "\n\tparam[out]3 (string): The configured working directory of the task (may be empty)"
        "\n")
      .def("is_config_modified_since_start", &EcalSysTask::IsConfigModifiedSinceStart,
        "\n"
        "\n Returns whether the config of this task has been modified since the last time this task has been started"
        "\n"
        "\n If the task is not running, this function always returns false. If the task"
        "\n is running, it returns whether any of the configuration-relevant Setters"
        "\n have been used, since Start() or Restart() was called."
        "\n The Name and ID do not qualify as configuration-relevant, as they are only"
        "\n used for displaying data in the console / GUI and saving the task to a"
        "\n file."
        "\n"
        "\n Returns: \n\t bool: True, if the task has been modified since it has been started."
        "\n")
      .def("reset_config_modified_since_start", &EcalSysTask::ResetConfigModifiedSinceStart,
        "\n"
        "\n Resets the config-modified-since-last-start property."
        "\n"
        "\n This function should not be called manually. It is already called by the"
        "\n task's startup procedure."
        "\n")
      .def("get_host_started_on", &EcalSysTask::GetHostStartedOn,
        "\n"
        "\n Gets the hostname of the machine that this task has last been started on"
        "\n If the task has not been started, an empty string is returned."
        "\n\n Returns: \n\t string: hostname of the machine that this task has last been started on"
        "\n")
      .def("get_pids", &EcalSysTask::GetPids,
        "\n"
        "\n Gets a list of all PIDs that belog to this task."
        "\n If the task has not been started, an empty list is returned. If the task"
        "\n has been started, the PIDs belog to the host returned by get_host_started_on"
        "\n\n Returns: \n\t vector(int): list of all PIDs"
        "\n")
      .def("get_start_stop_state", &EcalSysTask::GetStartStopState,
        "\n"
        "\n Gets the current StartStopState of this task, e.g. telling whether the task has been started successfully or if it has been stopped."
        "\n\n Returns: \n\t StartStopState: StartStopState of this task"
        "\n")
      .def("set_host_started_on", &EcalSysTask::SetHostStartedOn,
        "\n"
        "\n  Sets the hostname of the machine that this task has been started on"
        "\n If the task is not running or has been stopped, the name should be set to"
        "\n an empty string"
        "\n"
        "\n Note: This property is automatically set when starting / stopping /"
        "\n restarting the task and should not be set manually"
        "\n Args:"
        "\n\tparam1 (string): host_name"
        "\n")
      .def("set_pids", &EcalSysTask::SetPids,
        "\n"
        "\n  Sets the list of PIDs that belong to the (started) task (in combination with the hostname)"
        "\n"
        "\n Note: This property is automatically set when starting / stopping /"
        "\n restarting the task and should not be set manually"
        "\n Args:"
        "\n\tparam1 (vector(string)): host_name"
        "\n")
      .def("set_start_stop_state", &EcalSysTask::SetStartStopState,
        "\n"
        "\n  Sets the current StartStopState of the task"
        "\n"
        "\n Note: This property is automatically set when starting / stopping /"
        "\n restarting the task and should not be set manually"
        "\n Args:"
        "\n\tparam1 (StartStopState): host_name"
        "\n")
      .def("found_in_last_monitor_loop", &EcalSysTask::FoundInLastMonitorLoop,
        "\n This property is important for determining if the process of this task is"
        "\n still running. A task that has been found once,  but not in the last"
        "\n monitor iteration may be considered crashed."
        "\n\n Returns: \n\t bool: whether this task has been found by the monitor in the last monitor iteration."
        "\n")
      .def("set_found_in_last_monitor_loop", &EcalSysTask::SetFoundInLastMonitorLoop,
        "\n"
        "\n  Sets whether this task has been found by the monitor in the last iteration."
        "\n"
        "\n Note: This property is automatically set by the eCAL Sys monitor thread"
        "\n and should not be set manually."
        "\n Args:"
        "\n\tparam1 (bool): found_in_last_monitor_loop"
        "\n")
      .def("found_in_monitor_once", &EcalSysTask::FoundInMonitorOnce,
        "\n This property is important for determining if the process of this task is"
        "\n still running. A task that has been found once,  but not in the last"
        "\n monitor iteration may be considered crashed."
        "\n\n Returns: \n\t bool: whether this task has ever been found by the monitor thread."
        "\n")
      .def("set_found_in_monitor_once", &EcalSysTask::SetFoundInMonitorOnce,
        "\n"
        "\n  Sets whether this task has ever been found by the monitor."
        "\n"
        "\n Note: This property is automatically set by the eCAL Sys monitor thread"
        "\n and should not be set manually."
        "\n Args:"
        "\n\tparam1 (bool): found_in_monitor_once"
        "\n")
      .def("set_monitoring_task_state", &EcalSysTask::SetMonitoringTaskState,
        "\n"
        "\n  Sets the task state as found by the monitor."
        "\n"
        "\n Note: This property is automatically set by the eCAL Sys monitor thread"
        "\n and should not be set manually."
        "\n Args:"
        "\n\tparam1 (TaskState): task state"
        "\n")
      .def("get_monitoring_task_state", &EcalSysTask::GetMonitoringTaskState,
        "\n If the task has not been started, an unknown state is returned."
        "\n If the task has crashed (i.e. it has been found by the monitor once, but"
        "\n not any more), Failed Lv. 5 is returned."
        "\n\n Returns: \n\t TaskState: the task state as found by the monitor."
        "\n")
      .def("is_process_running", &EcalSysTask::IsProcessRunning,
        "\n The approach of this guess is quite conservative:"
        "\n  - If the task has been started successfully, we assume it is still running"
        "\n    unless we have a good reason that it is not (it has been found by the"
        "\n    monitor but is not any more)."
        "\n  - Otherwise, we assume that the task is not running."
        "\n\n Returns: \n\t bool: a best-guess of whether the process of this task is running."
        "\n");




  ecal_sys
    .def(py::init<const std::string &>(),
      "\n"
      " Creates a new Instance of EcalSys and immediately loads the given configuration file."
      "\n"
      " If the configuration file exists and can be loaded, EcalSys is populated\n"
      " with the data from the file.\n"
      "\n"
      " \nArgs:\n \tparam1 (string): The path to the configuration file to load\n"
      "\n"
      " Raises:\n\t RuntimeError:   If loading the configuration file has failed\n"
      "\n")
    .def(py::init<>(), "  \n"
      " Creates a new Instance of EcalSys with an empty config"
      "\n"
      " By default, EcalSys does not contain any task, runner or group. "
      "\nThe option check_target_reachability is activated."
      "\n"
      " Ecal Sys automatically connects to eCAL and starts the monitor thread."
      "\n")
    .def("load_config", &EcalSys::LoadConfig,
      "\n"
      " Loads the given config file"
      "\n"
      " If the file can be loaded, The internal configuration will be replaced by\n"
      " the one from the file (at least for the default-case, where append=false).\n"
      " Already running processes will continue running.\n"
      " If one or multiple tasks were currently starting / stopping / restarting,\n"
      " that procedure will be interrupted.\n"
      "\n"
      " If loading the file fails, an exception is thrown and EcalSys will stay\n"
      " in it's previous state.\n"
      "\n"
      " \nArgs:\n"
      " \tparam1 (string): The file to load\n"
      " \tparam2 (bool): If true, the config will be appended to the existing on. IDs may have to be re-mapped, if they already exist. Options will not be set.\n"
      "\n"
      "\nReturns: \n\t bool: True, if loading the file has been successfull.\n"
      "\n"
      " Raises:\n\t RuntimeError   If loading the configuration file has failed\n"
      "\n",
      "path"_a,
      "append"_a = false
    )
    .def("save_config", &EcalSys::SaveConfig,
      "\n"
      " Saves the EcalSys config to a file"
      "\n"
      " If saving the config failes, an exception is thrown."
      "\n"
      "\nArgs: \n"
      "\t param1 (string): The path to save the config to\n"
      "\t param2 (ConfigVersion): version The version of the config to save. By default, the config is saved as Version 1.0 config.\n"
      "\n"
      "\nReturns: \n\t bool: True, if saving the file has been successfull.\n"
      "\n"
      " Raises: \n\t RuntimeError:   If saving the configuration file has failed\n"
      "\n",
      "path"_a,
      "version"_a = ConfigVersion::v1_0
    )
    .def("clear_config", &EcalSys::ClearConfig,
      " Clears the internal config."
      "\n"
      " After clearing the config, Ecalsys will not contain any task, runner or group.\n"
      " Processes that are already running will continue running.\n"
      " If one or multiple tasks were currently starting / stopping / restarting, that procedure will be interrupted.\n"
      "\n"
      " The Ecalsys Options are reset to only check_target_reachability being set to true.\n"
      "\n")
    .def("is_config_opened", &EcalSys::IsConfigOpened,
      "\n"
      " Clearing the config will reset this property (see clear_config).\n"
      "\nReturns: \n\t bool: Whether the current config has initially been loaded from a configuration file."
      "\n")
    .def("get_current_config_path", &EcalSys::GetCurrentConfigPath,
      "\nReturns: \n\t string: The path of the currently loaded configuration file. If no config is loaded, an empty string is returned. \n")
    .def("get_options", &EcalSys::GetOptions,
      "\nReturns: \n\t Options: A copy of the currently set Ecalsys options. \n")
    .def("set_options", &EcalSys::SetOptions,
      " Sets the Ecalsys options \n"
      " \n Args: \n\t param1 Options: Options to be set")
    .def("get_task_list", &EcalSys::GetTaskList,
      " Gets a list of all tasks currently configured in EcalSys. \n"
      "\nReturns: \n\t (list(EcalSysTask)): list of all tasks")
    .def("get_runner_list", &EcalSys::GetRunnerList,
      " Gets a list of all runners currently configured in EcalSys. \n"
      "\nReturns: \n\t (list(EcalSysRunner)): list of all runners")
    .def("get_group_list", &EcalSys::GetGroupList,
      " Gets a list of all groups currently configured in EcalSys. \n"
      "\nReturns: \n\t (list(TaskGroup)): list of all groups")
    .def("get_task", &EcalSys::GetTask,
      " Gets Task by providing it's ID. If no runner with that ID exists, a shared nullpointer is returned. \n"
      " \n Args: \n\t param1 (int) : ID of task"
      "\nReturns: \n\t (EcalSysTask) : Found task")
    .def("get_runner", &EcalSys::GetRunner,
      " Gets a Runner by providing it's ID. If no runner with that ID exists, a shared nullpointer is returned. \n"
      " \n Args: \n\t param1 (int) : ID of runner"
      "\nReturns: \n\t (EcalSysTask) : Found runner")
    .def("add_task", &EcalSys::AddTask,
      "\n"
      " Adds an existing Task to EcalSys.\n"
      "\n"
      " By default, a new unused ID will be assigned to the Task. If that is"
      " undesired, the keep_id flag can be set. In that case however it should be"
      " made sure that no other task with that ID does exist.\n"
      "\n Args: \n"
      " \tparam1 (EcalSysTask): The task to add\n"
      " \tparam2 (bool): Whether to keep the currently configured ID from the task\n"
      "\n",
      "task"_a,
      "keep_id"_a = false)
    .def("add_runner", &EcalSys::AddRunner,
      "\n"
      " Adds an existing Runner to EcalSys.\n"
      "\n"
      " By default, a new unused ID will be assigned to the Runner. If that is"
      " undesired, the keep_id flag can be set. In that case however it should be"
      " made sure that no other runner with that ID does exist.\n"
      "\n Args: \n"
      " \tparam1 (EcalSysRunner): The runner to add\n"
      " \tparam2 (bool): Whether to keep the currently configured ID from the runner\n"
      "\n",
      "runner"_a,
      "keep_id"_a = false)
    .def("add_task_group", &EcalSys::AddTaskGroup,
      "\n"
      " Adds an existing Group to EcalSys.\n"
      "\n"
      " By default, a new unused ID will be assigned to the group. If that is"
      " undesired, the keep_id flag can be set. In that case however it should be"
      " made sure that no other group with that ID does exist.\n"
      "\n Args: \n"
      " \tparam1 (TaskGroup): The group to add\n"
      " \tparam2 (bool): Whether to keep the currently configured ID from the group\n"
      "\n",
      "task_group"_a,
      "keep_id"_a = false)
    .def("remove_task", &EcalSys::RemoveTask,
      "\n"
      " Removes the given task from EcalSys\n"
      "\n"
      " If the process of the task is currently running, it will continue to do so."
      " The task state will not be updated any more."
      "\n"
      " The task will also be removed from all groups that it has been assigned"
      " to. A list of all affected task groups is returned.\n"
      "\n Args: \n"
      " \tparam1 (EcalSysTask): The task to remove\n"
      "\nReturns: \n"
      " \t list(TaskGroup): A list of all groups that the task has been assigned to and that thus had to be modified.\n"
      "\n")
    .def("remove_runner", &EcalSys::RemoveRunner,
      "\n"
      " Removes the given runner from EcalSys\n"
      "\n"
      " As the runner might have been assigned to one or multiple tasks, two"
      " different strategies for removal can be chosen:"
      "\n"
      " If expand_runner_before_removal is true, the configuration of the runner"
      " will be copied to all tasks, that it had been assigned to. Thus, the algo"
      " path and command line parameters of those tasks are modified. The tasks"
      " will continue to be startable."
      "\n"
      " If expand_runner_before_removal is false, the runner will only be removed"
      " from all tasks. The tasks will not be modified (except for the runner"
      " assignment, that will be an empty runner afterwards). The tasks will most"
      " likely not be startable afterwards."
      "\n"
      " In either way, all affected tasks are returned as a list.\n"
      "\n Args:"
      " \tparam1 (EcalSysRunner): The runner to remove\n"
      " \tparam2 (bool): If true, the runner configuration is expanded and copied to all tasks that the runner had been assigned to.\n"
      "\nReturns: \n"
      " \t list(EcalSysTask): A list of tasks that this runner had been removed from.\n"
      "\n")
    .def("remove_task_group", &EcalSys::RemoveTaskGroup,
      "\n"
      " Removes the given group from EcalSys\n"
      "\n Args: \n"
      " \tparam1 (TaskGroup): The group to remove\n"
      "\n")
    .def("start_task_list", &EcalSys::StartTaskList,
      " Starts a list of tasks (optionally on a specific host) while respecting their launch order"
      "\n"
      " This function triggers the startup procedure for a list of tasks. It does"
      " not block but return immediatelly."
      "\n"
      " Tasks that are already running or that are scheduled for starting "
      " stopping / restarting will not be started."
      "\n"
      " Within the given list of tasks, the launch order is respected. Tasks with"
      " a higher launch order are guaranteed to start after tasks with a lower"
      " launch order. If one or multiple tasks have a timeout-after-start"
      " configured, this will delay the startup of all tasks with higher launch"
      " order. Tasks with the same launch order are started in parallel."
      "\n"
      " If the target_override is set to any hostname, the tasks will be started on"
      " that host."
      " If the target_override is left empty, the use_localhost_for_all_tasks"
      " option is used to determine the task's target. If"
      " use_localhost_for_all_tasks is true, localhost will be used, othwise the"
      " task will be started on the configured target.\n"
      "\n Args: \n"
      " \tparam1 (list(EcalSysTask)): The list of tasks that shall be started\n"
      " \tparam2 (string): The host to start the tasks on. \n"
      " \t\tIf empty, the tasks's target and the use_localhost_for_all_tasks option will be used to determine the target.\n"
      "\n",
      "task_list"_a,
      "target_override"_a = "")
    .def("stop_task_list", &EcalSys::StopTaskList,
      "\n"
      " Stops a list of tasks"
      "\n"
      " The Tasks can be stopped by an eCAL Shutdown request or they can be killed"
      " by means of the operating system. A shutdown request aims to give the tasks"
      " a chance to perform a gracefull shutdown. As this has to be implemented"
      " by each task, not all task might react to the request."
      "\n"
      " This function does not block but return immediatelly."
      "\n"
      " Task that are already stopped or that are scheduled for a start / stop /"
      " restart operation are not stopped.\n"
      "\n Args: \n"
      " \tparam1 (list(EcalSysTask)): The list of tasks that shall be stopped\n"
      " \tparam2 (bool): Whether a eCAL shutdown request shall be sent to the task\n"
      " \tparam3 (bool): Whether the task shall be hard-killed\n"
      " \tparam4 (bool): Whether the task shall be killed by it's name rather than the known PID (only needed when killing non-eCAL Task from the command line where their PID is unknown). Defaults to false.\n"
      " \tparam5 (nanoseconds): Time to wait for a gracefull shutdown, if both a shutdown request shall be sent and the task shall be killed afterwards. Defaults to 3 seconds.\n"
      "\n",
      "task_list"_a,
      "request_shutdown"_a,
      "kill_process"_a,
      "by_name"_a = false,
      "wait_for_shutdown"_a = std::chrono::seconds(3))
    .def("restart_task_list", &EcalSys::RestartTaskList,
      "\n"
      " Restarts a list of tasks by stopping and starting them again."
      "\n"
      " The Tasks can be stopped by an eCAL Shutdown request or they can be killed"
      " by means of the operating system. When starting the tasks, the launch"
      " order of the tasks is respected."
      "\n"
      " Also see stop_task_list and see start_task_list.\n"
      "\n Args: \n"
      " \tparam1 (list(EcalSysTask)): The list of tasks that shall be restarted\n"
      " \tparam2 (bool): Whether a eCAL shutdown request shall be sent to the task\n"
      " \tparam3 (bool): Whether the task shall be hard-killed\n"
      " \tparam4 (string): The host to start the tasks on. If empty, the tasks's target and the use_localhost_for_all_tasks option will be used to determine the target.\n"
      " \tparam5 (bool): Whether the task shall be killed by it's name rather than the known PID (only needed when killing non-eCAL Task from the command line where their PID is unknown). Defaults to false.\n"
      " \tparam6 (nanoseconds): Time to wait for a gracefull shutdown, if both a shutdown request shall be sent and the task shall be killed afterwards. Defaults to 3 seconds.\n"
      "\n",
      "task_list"_a,
      "request_shutdown"_a,
      "kill_process"_a,
      "target_override"_a = "",
      "by_name"_a = false,
      "wait_for_shutdown"_a =  std::chrono::seconds(3))
    .def("is_starting_or_stopping", &EcalSys::IsStartingOrStopping,
      "\n"
      "\nReturns: \n\t bool: true if the task is currently stopping / starting / restarting or if it is scheduled for one of those actions.\n"
      "\n")
    .def("interrupt_all_start_stop_actions", &EcalSys::InterruptAllStartStopActions,
      "\n"
      " Cancels all currently running start / stop / restart actions.\n"
      "\n"
      " Note that those actions may still need some time to actually stop. Use"
      " wait_for_start_stop_action to wait for them to finish."
      "\n")
    .def("wait_for_start_stop_action", &EcalSys::WaitFotStartStopActions,
      "\n"
      " Blocks until all start / stop / restart actions are finished."
      "\n")
    .def("is_ecal_rpc_service_running_on_host", &EcalSys::IsEcalRPCServiceRunningOnHost,
      "\n"
      " As for all monitor-related methods, the monitor might not have received"
      " any updates yet, if this function is called right after construction."
      "\n"
      "\nReturns: \n\t bool: true, if the monitor has found an eCAL rpc service running on the given host.\n"
      "\n")
    .def("get_all_hosts", &EcalSys::GetAllHosts,
      " As for all monitor-related methods, the monitor might not have received\n"
      " any updates yet, if this function is called right after construction.\n"
      "\n"
      "\nReturns: \n\t set(string): a list of all eCAL Hosts, i.e. all visible hosts that have at least one running eCAL process.\n"
      "\n")
    .def("get_hosts_running_ecal_sys", &EcalSys::GetHostsRunningEcalSys,
      "\n"
      "\n As for all monitor-related methods, the monitor might not have received"
      "\n any updates yet, if this function is called right after construction."
      "\n"
      "\nReturns: \n\t  vector(pair(string,int)): a list of all hosts that the monitor has found a running eCALSysGUI or eCALSys instance on."
    )
    .def("update_task_states", &EcalSys::UpdateTaskStates,
      "\n"
      "\n  Tells the monitor to assign the last known state to the given tasks"
      "\n"
      "\n The given list of tasks may or may not already be in the list of ecalsys"
      "\n tasks. This is usefull e.g. when importing tasks from cloud. The user"
      "\n might want to view an updated list of states, although he has not added"
      "\n the tasks to the EcalSys config, yet."
      "\n"
      "\n As for all monitor-related methods, the monitor might not have received"
      "\n any updates yet, if this function is called right after construction.\n"
      "\n Args:"
      "\n \tparam1 (list(EcalSysTask)): The list of tasks to update"
      "\n")
    .def("update_from_cloud", &EcalSys::UpdateFromCloud,
      "\n"
      "\n  Tells the monitor to query all visibile eCAL Processes and assign them to a matching EcalSys task."
      "\n"
      "\n The matching will set the Hostname, the PID and the Task State of all tasks"
      "\n that a match is found for."
      "\n"
      "\n The matching is as conservative as possible. If a task already has a"
      "\n hostname and PID assignment that still match the configuration, this match"
      "\n is preserved."
      "\n"
      "\n As for all monitor-related methods, the monitor might not have received"
      "\n any updates yet, if this function is called right after construction."
      "\n")
    .def("get_tasks_from_cloud", &EcalSys::GetTasksFromCloud,
      "\n"
      "\n  Creates new EcalsysTasks for all eCAL Processes that are visibile."
      "\n"
      "\n The new tasks are not added to Ecalsys; this step is left to the"
      "\n application. Thus, this method does not perform any changes to the current"
      "\n configuration."
      "\n"
      "\n Note that the tasks are also not compared to any existing task. Thus it is"
      "\n likely, that one or multiple of the tasks returned by this function are"
      "\n already included in the EcalSys instance. The application should compare"
      "\n the hostname and PIDs of these tasks to all existing running tasks in order"
      "\n to prevent duplicates, when adding the tasks to the model."
      "\n"
      "\n As for all monitor-related methods, the monitor might not have received"
      "\n any updates yet, if this function is called right after construction."
      "\n\nReturns: \n\t list(EcalSysTask): A list of tasks representing all visibile eCAL Processes"
      "\n")
    .def("set_monitor_update_callback", &EcalSys::SetMonitorUpdateCallback,
      "\n"
      "\n  Sets a callback function that gets called every time the monitor has been updated.\n"
      "\n Args: \n\t param1 (function): Callback function")
    .def("remote_monitor_update_callback", &EcalSys::RemoveMonitorUpdateCallback,
      "\n"
      "\n  Clears the monitor update callback"
      "\n");
}
