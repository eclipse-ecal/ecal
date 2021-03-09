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

#include "ecal_sys_monitor.h"

#include <chrono>
#include <set>

#include <ecal/msg/protobuf/publisher.h>

#include "threading/interruptible_loop_thread.h"

#include "ecalsys/ecal_sys.h"
#include "ecalsys/task/ecal_sys_task.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/monitoring.pb.h"
#include "ecal/pb/sys/state.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

/**
 * @brief The EcalSysMontior thread is responsible for all monitor-related methods.
 *
 * The EcalSysMonitor is running in it's own thread and periodically performs
 * monitor related tasks:
 *    - It sets the process states of all EcalSys tasks
 *    - It restarts tasks whose restart-by-severity option is set, if the
 *      state has reached the desired severity.
 *    - It periodically sends the state of all tasks and functions to the
 *      __ecalsys_state__ topic
 *    - It offers a method for assigning eCAL Processes that are visible in the
 *      cloud to a matching EcalsysTask
 *    - It offers methods to check which hosts have an eCALSysClient or other
 *      eCALSys instances running on them
 *
 * If desired, an update callback can be set. This callback will be called after
 * the monitor has finished gathering new information.
 */
class EcalSysMonitor :
  public InterruptibleLoopThread
{
public:
  EcalSysMonitor(EcalSys& ecalsys_instance, std::chrono::nanoseconds loop_time);
  ~EcalSysMonitor();

  /**
   * @brief Returns a list with host names of all machines that have at least one eCAL process running on them
   */
  std::set<std::string> GetAllHosts();

  /**
   * @brief Returns a list with host names of all machines that have a process called "eCALSysClient" running.
   */
  std::set<std::string> GetHostsRunningeCALSysClient();

  /**
   * @brief Returns a list with host names of all machines that have a process called "eCALSys" or "eCALSysGUI" running.
   */
  std::vector<std::pair<std::string, int>> GetHostsRunningEcalSys();

  /**
   * @brief Assigns the latest process states to the given tasks
   *
   * The given list of tasks may or may not already be already included in the
   * EcalSys instance. This is useful e.g. when importing tasks from cloud. The
   * user might want to view an updated list of states, although he has not
   * added the tasks to the EcalSys config, yet.
   *
   * @param task_list The list of tasks to assign states to
   */
  void UpdateTaskStates(const std::list<std::shared_ptr<EcalSysTask>>& task_list);

  /**
   * @brief Returns a list of all tasks that are currently visible in the cloud
   *
   * The returned task list is a complete representation of all eCAL Processes,
   * that are visible for the monitor. The tasks are not added to EcalSys.
   *
   * Note that the tasks are also not compared to any existing task. Thus it is
   * likely, that one or multiple of the tasks returned by this function are
   * already included in the EcalSys instance. The application should compare
   * the host name and PIDs of these tasks to all existing running tasks in order
   * to prevent duplicates, when adding the tasks to the model.
   *
   * @return A representation of all eCAL Processes that are visible for the monitor
   */
  std::list<std::shared_ptr<EcalSysTask>> GetTasksFromCloud();

  /**
   * @brief Sets a callback function, that will be called every time the monitor has completed updating.
   * @param callback The callback
   */
  void SetMonitorUpdateCallback(const std::function<void(void)>& callback);

  /**
   * @brief Clears the monitor update callback
   */
  void RemoveMonitorUpdateCallback();

protected:
  void Loop();

private:
  EcalSys& m_ecalsys_instance;
  eCAL::protobuf::CPublisher<eCAL::pb::sys::State>  m_state_publisher;                 /**< The publisher that sends information on the current state of all tasks and groups */

  std::recursive_mutex                              m_monitoring_mutex;                /**< A mutex protecting the m_monitoring_pb variable as well as the different host-lists */
  eCAL::pb::Monitoring                              m_monitoring_pb;                   /**< The object we get from deserializing the monitoring string. As that procedure might be expensive, we save the result in this variable */
  std::set<std::string>                             m_all_hosts;                       /**< A list of all hosts that are running any eCAL based software */
  std::set<std::string>                             m_hosts_running_ecal_sys_client;   /**< A list of all hosts where we found a running eCAL sys client during monitoring */
  std::vector<std::pair<std::string, int>>          m_hosts_running_ecalsys;           /**< A list of all hosts where we found a running eCAL Sys instance. Using multiple eCAL Sys instances might cause undefined behaviour, as each instance cannot track the current state of the tasks properly. Thus, we want to warn the user about that */

  std::list<std::shared_ptr<EcalSysTask>>           m_task_list;                       /**< List of all task that gets updated each iteration. This is a member variable to make sure that all functions operate on the same list. */

  std::function<void(void)>                         m_monitor_update_callback;         /**< Whenever the monitor finished updating the tasks, it calls the callback */
  bool                                              m_monitor_update_callback_valid;   /**< Whether the callback variable holds a valid callback or if it has not been set yet / it has been removed */
  std::mutex                                        m_monitor_update_callbacks_mutex;  /**< A mutex protecting the list of callbacks, as the monitor is running in its own thread */

  /**
   * @brief Queries the available monitor information and stores them as member variables
   *
   * This method updates:
   *    m_monitoring_pb
   *    m_all_hosts
   *    m_hosts_running_ecal_sys_client
   *    m_hosts_running_ecalsys
   */
  void UpdateMonitor();

  /**
   * @brief Iterates through all tasks and restarts those, whose restart-by-severity option is set, if the severity has reached the desired restart severity.
   */
  void RestartBySeverity();

  /**
   * @brief Publishes the state of all eCAL Sys Tasks and Groups
   *
   * The published information are:
   *    - The list of all Tasks
   *    - Task States
   *    - Current Group States
   */
  void SendEcalsysState();

  /**
   * @brief Return the given argument string with the first argument removed. If the string contains too less arguments, an empty string is returned.
   */
  std::string RemoveFirstArg(const std::string& arg_string);
};
