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

#include <mutex>
#include <chrono>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <ecal/ecal.h>

#include "ecalsys/task/ecal_sys_runner.h"
#include "ecalsys/task/task_state.h"

/**
 * @brief The EcalSysTask describes a task for eCAL Sys. It holds all it's configuration and offers functionality to start and stop the task.
 *
 * Getting and Setting properties is thread safe. For ensuring consistency of
 * the data when using multiple getters in a row, the task provides a public
 * mutex that can be locked.
 *
 */
class EcalSysTask
{
public:
  enum class StartStopState
  {
    NotStarted,
    Started_Successfully,
    Started_Failed,
    Stopped
  };

  std::recursive_mutex            mutex;                                        /**< Use this mutex, if you require to get consistent data when calling multiple getter functions. This mutex will be locked whenever one of the setter or getter function is used, so all other threads are blocked until you release the mutex. */

  EcalSysTask();

  ~EcalSysTask();

  //////////////////////////////////////////////////////////////////////////////
  //// Config Setter & Getter                                               ////
  //////////////////////////////////////////////////////////////////////////////

  /** @return The ID of this Task*/
  uint32_t                       GetId();
  
  /** @return The configured name of this task*/
  std::string                    GetName();

  /** @return The configured Target-Hostname*/
  std::string                    GetTarget();

  /** @return The configured path to the algorithm, e.g. an executable binary or shared library. If a runner is set, it will be given to the runner as argument. Otherwise, the algo will be executed as executable. */
  std::string                    GetAlgoPath();

  /** @return The configured working directory*/
  std::string                    GetWorkingDir();

  /** @return The configured runner. If no runner is set, this is a shared nullpointer. */
  std::shared_ptr<EcalSysRunner> GetRunner();

  /** @return The configured launch order of this task. If multiple tasks are started simultaniously, tasks with a higher launch order will be started later. */
  unsigned int                   GetLaunchOrder();

  /** @return The configured time that will be used to artificially prolong the time that the task needs to start. This is relevant, if multiple tasks are started simultaniously, as it will delay tasks with a higher launch order.*/
  std::chrono::nanoseconds       GetTimeoutAfterStart();

  /** @return The configured startup-visibility when starting this Task on a Windows host.*/
  eCAL_Process_eStartMode        GetVisibility();

  /** @return The configured arguments that will be given to the task when being started.*/
  std::string                    GetCommandLineArguments();

  /** @return Whether monitoring is enabled. When true, this task will be monitored by the monitoring thread and the task state will be set accordingly. This is a requirement for the restart-by-severity functionality.*/
  bool                           IsMonitoringEnabled();

  /** @return Whether this tasks may be restarted by the monitoring thread, if it's severity reaches a certain state. This option also requires monitoring to be enabled.*/
  bool                           IsRestartBySeverityEnabled();

  /** @return The configured state, at which the task will be restarted, if the restart-by-severity option is enabled. This option also requires monitoring to be enabled.*/
  TaskState                      GetRestartAtSeverity();


  /** @brief Sets the Task ID. You should propably not set the ID manually, as it is already set when adding a task to an EcalSys instance. */
  void SetId                         (uint32_t id);

  /** @brief Sets the name of the task. The name is only relevant for logging and displaying it in the console or GUI*/
  void SetName                       (const std::string& name);

  /** @brief Sets the host, that this task is supposed to start on by default. */
  void SetTarget                     (const std::string& target);

  /** @brief Sets the path to the algoright. This can e.g. be an executable (if the task does not have a runner) or some other binary or script that gets executed by the runner. */
  void SetAlgoPath                   (const std::string& algo_path);

  /** @brief Sets the working directory that will be used when starting the task. */
  void SetWorkingDir                 (const std::string& working_dir);

  /** @brief Sets the runner. Can be set to a shared nullpointer, if this task is not supposed to have a runner. */
  void SetRunner                     (const std::shared_ptr<EcalSysRunner> runner);

  /** @brief Sets the launch order of this task. If multiple tasks are started simultaniously, tasks with a higher launch order will be started later. */
  void SetLaunchOrder                (unsigned int launch_order);

  /** @brief Sets a time that will be used to artificially prolong the time that the task needs to start. This is relevant, if multiple tasks are started simultaniously, as it will delay tasks with a higher launch order. */
  void SetTimeoutAfterStart          (std::chrono::nanoseconds timeout);

  /** @brief Sets the visibility when starting the task on a Windows machine. */
  void SetVisibility                 (eCAL_Process_eStartMode visibility);

  /** @brief Sets the arguments that will be given to the task when being started. */
  void SetCommandLineArguments       (std::string command_line_args);

  /** @brief Sets whether this task will be monitored by the monitoring thread. If enabled, the task-state will be updated periodically. This also affects whether the restart-by-severity functions are available. */
  void SetMonitoringEnabled          (bool enabled);

  /** @brief Sets Whether this task shall be restarted by the monitoring thread, if it's severity reaches a certain state. This option also requires monitoring to be enabled.*/
  void SetRestartBySeverityEnabled   (bool enabled);

  /** @brief Sets the severity, at which the task will be restarted, if the restart-by-severity option is enabled. This option also requires monitoring to be enabled.*/
  void SetRestartAtSeverity          (const TaskState& severity);

  /**
   * @brief Returns whether the config of this task has been modified since the last time this task has been started
   *
   * If the task is not running, this function always returns false. If the task
   * is running, it returns whether any of the configuration-relevant Setters
   * have been used, since Start() or Restart() was called.
   * The Name and ID do not qualify as configuration-relevant, as they are only
   * used for displaying data in the console / GUI and saving the task to a
   * file.
   *
   * @return True, if the task has been modified since it has been started.
   */
  bool IsConfigModifiedSinceStart();

  /**
   * @brief Resets the config-modified-since-last-start property.
   *
   * This function should not be called manually. It is already called by the
   * task's startup procedure.
   */
  void ResetConfigModifiedSinceStart();

  //////////////////////////////////////////////////////////////////////////////
  //// Information on the start state                                       ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Gets the hostname of the machine that this task has last been started on
   * If the task has not been started, an empty string is returned.
   */
  std::string      GetHostStartedOn();

  /**
   * @brief Gets a list of all PIDs that belog to this task.
   * If the task has not been started, an empty list is returned. If the task
   * has been started, the PIDs belog to the host returned by
   * @see{GetHostStartedOn()}
   */
  std::vector<int> GetPids();

  /**
   * @brief Gets the current StartStopState of this task, e.g. telling whether the task has been started successfully or if it has been stopped.
   */
  StartStopState   GetStartStopState();

  /**
   * @brief Sets the hostname of the machine that this task has been started on
   * If the task is not running or has been stopped, the name should be set to
   * an empty string
   *
   * Note: This property is automatically set when starting / stopping /
   * restarting the task and should not be set manually
   */
  void SetHostStartedOn (const std::string& host_name);


  /**
   * @brief Sets the list of PIDs that belong to the (started) task (in combination with the hostname)
   *
   * Note: This property is automatically set when starting / stopping /
   * restarting the task and should not be set manually
   */
  void SetPids          (const std::vector<int>& pid_list);


  /**
   * @brief Sets the current StartStopState of the task
   *
   * Note: This property is automatically set when starting / stopping /
   * restarting the task and should not be set manually
   */
  void SetStartStopState(const StartStopState start_stop_state);


  //////////////////////////////////////////////////////////////////////////////
  //// Process Monitoring                                                   ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Returns whether this task has been found by the monitor in the last monitor iteration.
   *
   * This property is important for determining if the process of this task is
   * still running. A task that has been found once,  but not in the last
   * monitor iteration may be considered crashed.
   */
  bool FoundInLastMonitorLoop();

  /**
   * @brief Sets whether this task has been found by the monitor in the last iteration.
   *
   * Note: This property is automatically set by the eCAL Sys monitor thread
   * and should not be set manually.
   */
  void SetFoundInLastMonitorLoop(bool found_in_last_monitor_loop);

  /**
   * @brief Returns whether this task has ever been found by the monitor thread.
   *
   * This property is important for determining if the process of this task is
   * still running. A task that has been found once,  but not in the last
   * monitor iteration may be considered crashed.
   */
  bool FoundInMonitorOnce();

  /**
   * @brief Sets whether this task has ever been found by the monitor.
   *
   * Note: This property is automatically set by the eCAL Sys monitor thread
   * and should not be set manually.
   */
  void SetFoundInMonitorOnce(bool found_in_monitor_once);

  /**
   * @brief Sets the task state as found by the monitor.
   *
   * Note: This property is automatically set by the eCAL Sys monitor thread
   * and should not be set manually.
   */
  void SetMonitoringTaskState(const TaskState& task_state);

  /**
   * @brief Returns the task state as found by the monitor.
   *
   * If the task has not been started, an unknown state is returned.
   * If the task has crashed (i.e. it has been found by the monitor once, but
   * not any more), Failed Lv. 5 is returned.
   */
  TaskState GetMonitoringTaskState();

  /**
   * @brief Returns a best-guess of whether the process of this task is running.
   *
   * The approach of this guess is quite conservative:
   *  - If the task has been started successfully, we assume it is still running
   *    unless we have a good reason that it is not (it has been found by the
   *    monitor but is not any more).
   *  - Otherwise, we assume that the task is not running.
   */
  bool IsProcessRunning();

private:
  // Config
  uint32_t                              m_id;                                   /**< The ID of this task when saving it to a file */
  std::string                           m_name;                                 /**< The name of this task */
  std::string                           m_target;                               /**< The configured target for this task.*/
  std::shared_ptr<EcalSysRunner>        m_runner;                               /**< The Runner of the current Task (might be null!)*/
  std::string                           m_algo_path;                            /**< Path to an algorithm, e.g. an executable binary or shared library. If a runner is set, it will be given to the runner as argument. Otherwise, the algo will be executed as executable. */
  std::string                           m_command_line_arguments;               /**< Arguments that are given to the process when being started*/
  std::string                           m_working_directory;                    /**< The working directory when starting the process */
  unsigned int                          m_launch_order;                         /**< The order in which tasks will start when started simultaneously. */
  std::chrono::nanoseconds              m_timeout_after_start;                  /**< After being started, the task may wait a certain amount of time and thus delay the start of other tasks that are started at the same time but have a higher launch order number.*/
  eCAL_Process_eStartMode               m_visibility;                           /**< The visibility when starting this task on a Windows system */

  bool                                  m_monitoring_enabled;                   /**< When true, this task will be monitored by the monitoring thread and the task state will be set accordingly. This is a requirement for the restart-by-severity functionality.*/
  bool                                  m_restart_by_severity_enabled;          /**< When true, the task will be killed and restarted if it's severity reaches a certain state. */
  TaskState                             m_restart_at_severity;                  /**< When the task reaches this state, it will be killed and restarted, if m_restart_by_severity_enabled is true. */

  bool                                  m_config_modified_since_start;          /**< Whether any of the config-related setters has been used since the task has been started the last time */

  // Information on the start state
  std::string                           m_host_started_on;                      /**< The hostname of the target this task has last been started on */
  std::vector<int>                      m_pids;                                 /**< The Process IDs of this task's processes. Usually only holds 1 element */
  StartStopState                        m_start_stop_state;                     /**< Whether this task has been started, stopped, not started or whether the start has failed */

  // Monitoring related variables
  std::atomic<bool>                     m_found_in_last_monitor_loop;           /**< Whether the monitor thread found this task the last time it checked the monitoring */
  std::atomic<bool>                     m_found_in_monitor_once;                /**< Whether the monitor thread has ever found this task in the monitoring */
  TaskState                             m_task_state;                           /**< The State (severity, level, info) of this task */
};
