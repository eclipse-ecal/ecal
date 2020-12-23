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

#include <list>
#include <vector>
#include <utility>
#include <string>
#include <atomic>
#include <memory>
#include <mutex>
#include <set>

#include <ecal/ecal.h>

#include "ecalsys/config/config_version.h"

#include "ecalsys/task/ecal_sys_task.h"
#include "ecalsys/task/ecal_sys_runner.h"
#include "ecalsys/task/task_group.h"

class EcalSysMonitor;
class TaskListThread;
namespace eCAL
{
  namespace sys
  {
    class ConnectionManager;
  }
}

#include "threading/threadsafe_container.h"

/**
 * @brief EcalSys is the most important class of the eCAL Sys Core bundeling all functionality.
 *
 * EcalSys is composed of:
 *    EcalsysTasks
 *    EcalsysRunners
 *    TaskGroups
 *
 * EcalsysTask:
 *    An EcalsysTask describes some kind of Process or Task that can be executed
 *    on a machine. Most importantly, an EcalsysTask contains a path and a
 *    the name of the host that the task is usually started on.
 *    While running, Ecalsys periodically updates the state that is sent by the
 *    tasks (Healthy, Warning, Critical, Failed). This state may be displayed in
 *    console or GUI.
 *    A task may or may not have an EcalsysRunner assigned to it.
 *
 * EcalsysRunner:
 *    A runner describes a loader, that can load a task. If a task has a runner
 *    assigned to it, the runner-path usually points to an executable, while
 *    task-path points to another file that will get loaded by the runner, e.g.
 *    a DLL file or a script.
 *    Although everything could just as well be expressed right within each task
 *    using command line parameters, a runner makes it easier to manage and
 *    modify the tasks.
 *
 * TaskGroup:
 *    A TaskGroup gives the user the possbility to group different tasks and
 *    calculate a meta-state for the whole group. Thereby he does not have to
 *    check the state of all Tasks individually.
 *
 * Tasks can be added to Ecalsys and removed from it using the appropriate
 * methods. They can also be started, stopped and restarted. All methods are
 * thread-safe.
 * The configuration can be loaded from and saved to a file.
 *
 * EcalSys also offers options to control the behaviour (@see {GetOptions()},
 * @see {SetOptions()}). Except for the use_localhost_for_all_tasks option,
 * the application using Ecalsys is responsible for respecting those.
 *
 * When creating an instance of EcalSys, it will automatically connect to eCAL
 * and start an extra monitoring thread. This thread will periodically query
 * the information avaliable from the monitoring API, assign the Process states
 * to all tasks from EcalSys if available and publish the __ecalsys_state__
 * topic for other nodes to use.
 */
class EcalSys
{
public:
  struct Options
  {
    bool kill_all_on_close;
    bool use_localhost_for_all_tasks;
    bool local_tasks_only;
    bool check_target_reachability;
  };

  //////////////////////////////////////////////////////////////////////////////
  //// Constructor & Destructor                                             ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Creates a new Instance of EcalSys with an empty config
   *
   * By default, EcalSys does not contain any task, runner or group. The option
   * check_target_reachability is activated.
   *
   * Ecal Sys automatically connects to eCAL and starts the monitor thread.
   */
  EcalSys();

  /**
   * @brief Creates a new Instance of EcalSys and immediately loads the given configuration file.
   *
   * If the configuration file exists and can be loaded, EcalSys is populated
   * with the data from the file.
   *
   * @param config_path The path to the configuration file to load
   *
   * @throws std::runtime_error   If loading the configuration file has failed
   */
  EcalSys(const std::string& config_path);

  ~EcalSys();

  //////////////////////////////////////////////////////////////////////////////
  //// Config handling                                                      ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Loads the given config file
   *
   * If the file can be loaded, The internal configuration will be replaced by
   * the one from the file (at least for the default-case, where append=false).
   * Already running processes will continue running.
   * If one or multiple tasks were currently starting / stopping / restarting,
   * that procedure will be interrupted.
   *
   * If loading the file fails, an exception is thrown and EcalSys will stay
   * in it's previous state.
   *
   * @param path    The file to load
   * @param append  If true, the config will be appended to the existing on. IDs may have to be re-mapped, if they already exist. Options will not be set.
   *
   * @throws std::runtime_error   If loading the configuration file has failed
   *
   * @return True, if loading the file has been successfull.
   */
  bool        LoadConfig(const std::string& path, bool append = false);

  /**
   * @brief Saves the EcalSys config to a file
   *
   * If saving the config failes, an exception is thrown.
   *
   * @param path    The path to save the config to
   * @param version The version of the config to save. By default, the config is saved as Version 1.0 config.
   *
   * @throws std::runtime_error   If saving the configuration file has failed
   *
   * @return True, if saving the file has been successfull.
   */
  bool        SaveConfig(const std::string& path, ConfigVersion version = ConfigVersion::v1_0);

  /**
   * @brief Clears the internal config.
   *
   * After clearing the config, Ecalsys will not contain any task, runner or
   * group.
   * Processes that are already running will continue running.
   * If one or multiple tasks were currently starting / stopping / restarting,
   * that procedure will be interrupted.
   *
   * The Ecalsys Options are reset to only check_target_reachability being
   * set to true.
   */
  void        ClearConfig();

  /**
   * @brief Returns whether the current config has initially been loaded from a configuration file.
   * Clearing the config will reset this property (@see{ClearConfig}).
   */
  bool        IsConfigOpened();

  /** @brief Returns the path of the currently loaded configuration file. If no config is loaded, an empty string is returned. */
  std::string GetCurrentConfigPath();

  /** @brief Returns a copy of the currently set Ecalsys options. */
  Options     GetOptions();

  /** @brief Sets the Ecalsys options */
  void        SetOptions(const Options& options);

  //////////////////////////////////////////////////////////////////////////////
  //// Task / Runner / Group management                                     ////
  //////////////////////////////////////////////////////////////////////////////

  /** @brief Gets a list of all tasks currently configured in EcalSys. */
  std::list<std::shared_ptr<EcalSysTask>>   GetTaskList() const;

  /** @brief Gets a list of all runners currently configured in EcalSys. */
  std::list<std::shared_ptr<EcalSysRunner>> GetRunnerList() const;

  /** @brief Gets a list of all groups currently configured in EcalSys. */
  std::list<std::shared_ptr<TaskGroup>>     GetGroupList() const;

  /** @brief Gets Task by providing it's ID. If no runner with that ID exists, a shared nullpointer is returned. */
  std::shared_ptr<EcalSysTask>   GetTask(uint32_t task_id);

  /** @brief Gets a Runner by providing it's ID. If no runner with that ID exists, a shared nullpointer is returned. */
  std::shared_ptr<EcalSysRunner> GetRunner(uint32_t runner_id);

  /**
   * @brief Adds an existing Task to EcalSys.
   *
   * By default, a new unused ID will be assigned to the Task. If that is
   * undesired, the keep_id flag can be set. In that case however it should be
   * made sure that no other task with that ID does exist.
   *
   * @param task      The task to add
   * @param keep_id   Whether to keep the currently configured ID from the task
   */
  void AddTask     (std::shared_ptr<EcalSysTask> task, bool keep_id = false);

  /**
   * @brief Adds an existing Runner to EcalSys.
   *
   * By default, a new unused ID will be assigned to the Runner. If that is
   * undesired, the keep_id flag can be set. In that case however it should be
   * made sure that no other runner with that ID does exist.
   *
   * @param runner    The runner to add
   * @param keep_id   Whether to keep the currently configured ID from the runner
   */
  void AddRunner   (std::shared_ptr<EcalSysRunner> runner, bool keep_id = false);

  /**
   * @brief Adds an existing Group to EcalSys.
   *
   * By default, a new unused ID will be assigned to the group. If that is
   * undesired, the keep_id flag can be set. In that case however it should be
   * made sure that no other group with that ID does exist.
   *
   * @param task_group  The group to add
   * @param keep_id     Whether to keep the currently configured ID from the group
   */
  void AddTaskGroup(std::shared_ptr<TaskGroup> task_group, bool keep_id = false);

  /**
   * @brief Removes the given task from EcalSys
   *
   * If the process of the task is currently running, it will continue to do so.
   * The task state will not be updated any more.
   *
   * The task will also be removed from all groups that it has been assigned
   * to. A list of all affected task groups is returned.
   *
   * @param task  The task to remove
   *
   * @return A list of all groups that the task has been assigned to and that thus had to be modified.
   */
  std::list<std::shared_ptr<TaskGroup>>   RemoveTask(std::shared_ptr<EcalSysTask> task);

  /**
   * @brief Removes the given runner from EcalSys
   *
   * As the runner might have been assigned to one or multiple tasks, two
   * different strategies for removal can be chosen:
   *
   * If expand_runner_before_removal is true, the configuration of the runner
   * will be copied to all tasks, that it had been assigned to. Thus, the algo
   * path and command line parameters of those tasks are modified. The tasks
   * will continue to be startable.
   *
   * If expand_runner_before_removal is false, the runner will only be removed
   * from all tasks. The tasks will not be modified (except for the runner
   * assignment, that will be an empty runner afterwards). The tasks will most
   * likely not be startable afterwards.
   *
   * In either way, all affected tasks are returned as a list.
   *
   * @param runner                        The runner to remove
   * @param expand_runner_before_removal  If true, the runner configuration is expanded and copied to all tasks that the runner had been assigned to.
   *
   * @return A list of tasks that this runner had been removed from.
   */
  std::list<std::shared_ptr<EcalSysTask>> RemoveRunner(std::shared_ptr<EcalSysRunner> runner, bool expand_runner_before_removal);

  /**
   * @brief Removes the given group from EcalSys
   *
   * @param task_group The group to remove
   */
  void                                    RemoveTaskGroup(std::shared_ptr<TaskGroup> task_group);

  //////////////////////////////////////////////////////////////////////////////
  //// Starting / Stopping / Restarting list of tasks                       ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Starts a list of tasks (optionally on a specific host) while respecting their launch order
   *
   * This function triggers the startup procedure for a list of tasks. It does
   * not block but return immediatelly.
   *
   * Tasks that are already running or that are scheduled for starting /
   * stopping / restarting will not be started.
   *
   * Within the given list of tasks, the launch order is respected. Tasks with
   * a higher launch order are guaranteed to start after tasks with a lower
   * launch order. If one or multiple tasks have a timeout-after-start
   * configured, this will delay the startup of all tasks with higher launch
   * order. Tasks with the same launch order are started in parallel.
   *
   * If the target_override is set to any hostname, the tasks will be started on
   * that host.
   * If the target_override is left empty, the use_localhost_for_all_tasks
   * option is used to determine the task's target. If
   * use_localhost_for_all_tasks is true, localhost will be used, othwise the
   * task will be started on the configured target.
   *
   * @param task_list         The list of tasks that shall be started
   * @param target_override   The host to start the tasks on. If empty, the tasks's target and the use_localhost_for_all_tasks option will be used to determine the target.
   */
  void StartTaskList               (const std::list<std::shared_ptr<EcalSysTask>>& task_list, const std::string& target_override = "");

  /**
   * @brief Stops a list of tasks
   *
   * The Tasks can be stopped by an eCAL Shutdown request or they can be killed
   * by means of the operating system. A shutdown request aims to give the tasks
   * a chance to perform a gracefull shutdown. As this has to be implemented
   * by each task, not all task might react to the request.
   *
   * This function does not block but return immediatelly.
   *
   * Task that are already stopped or that are scheduled for a start / stop /
   * restart operation are not stopped.
   *
   * @param task_list         The list of tasks that shall be stopped
   * @param request_shutdown  Whether a eCAL shutdown request shall be sent to the task
   * @param kill_process      Whether the task shall be hard-killed
   * @param by_name           Whether the task shall be killed by it's name rather than the known PID (only needed when killing non-eCAL Task from the command line where their PID is unknown). Defaults to false.
   * @param wait_for_shutdown Time to wait for a gracefull shutdown, if both a shutdown request shall be sent and the task shall be killed afterwards. Defaults to 3 seconds.
   */
  void StopTaskList                (const std::list<std::shared_ptr<EcalSysTask>>& task_list, bool request_shutdown, bool kill_process, bool by_name = false, std::chrono::nanoseconds wait_for_shutdown = std::chrono::seconds(3));

  /**
   * @brief Restarts a list of tasks by stopping and starting them again.
   *
   * The Tasks can be stopped by an eCAL Shutdown request or they can be killed
   * by means of the operating system. When starting the tasks, the launch
   * order of the tasks is respected.
   *
   * Also see @see{StopTaskList()} and @see{StartTaskList()}.
   *
   * @param task_list         The list of tasks that shall be restarted
   * @param request_shutdown  Whether a eCAL shutdown request shall be sent to the task
   * @param kill_process      Whether the task shall be hard-killed
   * @param target_override   The host to start the tasks on. If empty, the tasks's target and the use_localhost_for_all_tasks option will be used to determine the target.
   * @param by_name           Whether the task shall be killed by it's name rather than the known PID (only needed when killing non-eCAL Task from the command line where their PID is unknown). Defaults to false.
   * @param wait_for_shutdown Time to wait for a gracefull shutdown, if both a shutdown request shall be sent and the task shall be killed afterwards. Defaults to 3 seconds.
   */
  void RestartTaskList             (const std::list<std::shared_ptr<EcalSysTask>>& task_list, bool request_shutdown, bool kill_process, const std::string& target_override = "", bool by_name = false, std::chrono::nanoseconds wait_for_shutdown = std::chrono::seconds(3));

  /**
   * @brief Start all tasks
   *
   * Starts all tasks with respect to the options (local-tasks-only and use-localhost-for-all-tasks)
   */
  void StartTasks();

  /**
   * @brief Stops all tasks
   *
   * Stops all tasks while respecting the options (local-tasks-only).
   * The Tasks are stopped "normally", i.e. by an eCAL shutdown signal and a
   * subsequent task kill
   */
  void StopTasks();

  /**
   * @brief Restarts all tasks
   *
   * Restarts all tasks with respect to the options (local-tasks-only and
   * use-localhost-for-all-tasks).
   * The Tasks are stopped "normally", i.e. by an eCAL shutdown signal and a
   * subsequent task kill
   */
  void RestartTasks();

  /**
   * @brief Returns true if the task is currently stopping / starting / restarting or if it is scheduled for one of those actions.
   */
  bool IsTaskActionRunning        (const std::shared_ptr<EcalSysTask> task);

  /**
   * @brief Cancels all currently running start / stop / restart actions.
   *
   * Note that those actions may still need some time to actually stop. Use
   * @see{WaitForTaskActions()} to wait for them to finish.
   */
  void InterruptAllTaskActions();

  /**
   * @brief Blocks until all start / stop / restart actions are finished.
   */
  void WaitForTaskActions     ();

  //////////////////////////////////////////////////////////////////////////////
  //// Monitoring                                                           ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Returns true, if the monitor has found an eCAL sys client running on the given host.
   *
   * As for all monitor-related methods, the monitor might not have received
   * any updates yet, if this function is called right after construction.
   */
  bool IseCALSysClientRunningOnHost(const std::string& host_name) const;

  /**
   * @brief Returns a list of all eCAL Hosts, i.e. all visible hosts that have at least one running eCAL process.
   *
   * As for all monitor-related methods, the monitor might not have received
   * any updates yet, if this function is called right after construction.
   */
  std::set<std::string> GetAllHosts();

  /**
   * @brief Returns a list of all hosts that the monitor has found a running eCALSysGUI or eCALSys instance on.
   *
   * As for all monitor-related methods, the monitor might not have received
   * any updates yet, if this function is called right after construction.
   */
  std::vector<std::pair<std::string, int>> GetHostsRunningEcalSys();

  /**
   * @brief Tells the monitor to assign the last known state to the given tasks
   *
   * The given list of tasks may or may not already be in the list of ecalsys
   * tasks. This is usefull e.g. when importing tasks from cloud. The user
   * might want to view an updated list of states, although he has not added
   * the tasks to the EcalSys config, yet.
   *
   * As for all monitor-related methods, the monitor might not have received
   * any updates yet, if this function is called right after construction.
   *
   * @param task_list The list of tasks to update
   */
  void UpdateTaskStates(const std::list<std::shared_ptr<EcalSysTask>>& task_list);

  /**
   * @brief Tells the monitor to query all visibile eCAL Processes and assign them to a matching EcalSys task.
   *
   * The matching will set the Hostname, the PID and the Task State of all tasks
   * that a match is found for.
   *
   * The matching is as conservative as possible. If a task already has a
   * hostname and PID assignment that still match the configuration, this match
   * is preserved.
   * 
   * As for all monitor-related methods, the monitor might not have received
   * any updates yet, if this function is called right after construction.
   */
  void UpdateFromCloud();

  /**
   * @brief Creates new EcalsysTasks for all eCAL Processes that are visibile.
   *
   * The new tasks are not added to Ecalsys; this step is left to the
   * application. Thus, this method does not perform any changes to the current
   * configuration.
   * 
   * Note that the tasks are also not compared to any existing task. Thus it is
   * likely, that one or multiple of the tasks returned by this function are
   * already included in the EcalSys instance. The application should compare
   * the hostname and PIDs of these tasks to all existing running tasks in order
   * to prevent duplicates, when adding the tasks to the model.
   *
   * As for all monitor-related methods, the monitor might not have received
   * any updates yet, if this function is called right after construction.
   * 
   * @return A list of tasks representing all visibile eCAL Processes
   */
  std::list<std::shared_ptr<EcalSysTask>> GetTasksFromCloud();

  /**
   * @brief Sets a callback function that gets called every time the monitor has been updated.
   */
  void SetMonitorUpdateCallback(const std::function<void(void)>& callback);

  /**
   * @brief Clears the monitor update callback
   */
  void RemoveMonitorUpdateCallback();

private:
  std::list<std::shared_ptr<EcalSysTask>>    m_task_list;                       /**< The list of all Tasks */
  mutable std::recursive_mutex               m_task_list_mutex;                 /**< A mutex protecting the task list */

  std::list<std::shared_ptr<EcalSysRunner>>  m_runner_list;                     /**< The list of all Runners */
  mutable std::recursive_mutex               m_runner_list_mutex;               /**< A mutex protecting the runner list */

  std::list<std::shared_ptr<TaskGroup>>      m_group_list;                      /**< The list of all tasks groups */
  mutable std::recursive_mutex               m_group_list_mutex;                /**< A mutex protecting the group list */

  std::unique_ptr<EcalSysMonitor>            m_monitor_thread;                  /**< The monitor thread that is responsible for querying the monitor information, setting task states and providing some specialized methods to get monitor information. */

  std::shared_ptr<eCAL::sys::ConnectionManager> m_connection_manager;           /**< A manager for connections to remote hosts (and localhost) **/
  ThreadSafeContainer <std::shared_ptr<TaskListThread>> m_task_list_action_thread_container; /**< A container for the threads that start / stop / restart list of tasks. Once a thread is finished, it will get cleaned by the next thread. */

  

  std::atomic<bool>    m_is_config_opened;                                      /**< Whether the the current configuration has eventually been loaded from a file */
  std::string          m_current_config_path;                                   /**< The path of the current configuration file. May be empty, of no file has been loaded or the config has been cleared since then. */
  Options              m_options;                                               /**< The current options  */
  std::recursive_mutex m_config_mutex;                                          /**< A mutex protecting all configuraiton-relevant data, i.e. m_is_config_opened, m_current_config_path, m_options */

  /**
   * @brief Logs the name and version of eCAL Sys to a log file or the console
   */
  void LogAppNameVersion() const;

  /**
   * @brief Clears all threads from the m_task_list_action_thread_container that are not running any more.
   */
  void RemoveFinishedTaskListThreads();
};
