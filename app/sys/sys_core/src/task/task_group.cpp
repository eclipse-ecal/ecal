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

#include "ecalsys/task/task_group.h"

#include "ecalsys/task/ecal_sys_task.h"

TaskGroup::TaskGroup()
  : m_id(0)
  , m_name("")
  , m_group_state_list()
{}


TaskGroup::~TaskGroup()
{}

std::shared_ptr<TaskGroup::GroupState> TaskGroup::Evaluate()
{
  std::lock_guard<std::mutex> group_lock(m_mutex);
  for (auto& state : m_group_state_list)
  {
    if (state->Evaluate())
    {
      return state;
    }
  }
  return std::shared_ptr<TaskGroup::GroupState>(nullptr);
}

std::set<std::shared_ptr<EcalSysTask>> TaskGroup::GetAllTasks()
{
  std::set<std::shared_ptr<EcalSysTask>> task_set;
  for (auto& group_state : GetGroupStateList())
  {
    for (auto& minimal_task_state : group_state->GetMinimalStatesList())
    {
      task_set.emplace(minimal_task_state.first);
    }
  }
  return task_set;
}

void TaskGroup::SetId(uint32_t id)
{
  std::lock_guard<std::mutex> group_lock(m_mutex);
  m_id = id;
}

uint32_t TaskGroup::GetId()
{
  std::lock_guard<std::mutex> group_lock(m_mutex);
  return m_id;
}

void TaskGroup::SetName(const std::string& name)
{
  std::lock_guard<std::mutex> group_lock(m_mutex);
  m_name = name;
}

std::string TaskGroup::GetName()
{
  std::lock_guard<std::mutex> group_lock(m_mutex);
  return m_name;
}

std::list<std::shared_ptr<TaskGroup::GroupState>> TaskGroup::GetGroupStateList()
{
  std::lock_guard<std::mutex> group_lock(m_mutex);
  return std::list<std::shared_ptr<TaskGroup::GroupState>>(m_group_state_list);
}

void TaskGroup::SetGroupStateList(const std::list<std::shared_ptr<TaskGroup::GroupState>>& group_state_list)
{
  std::lock_guard<std::mutex> group_lock(m_mutex);
  m_group_state_list = group_state_list;
}

////////////////////////////////////////////////////////////////////////////////
//// Group State                                                            ////
////////////////////////////////////////////////////////////////////////////////

TaskGroup::GroupState::GroupState()
  : m_name("")
  , m_color(200, 200, 200)
  , m_minimal_states_list()
{
}

TaskGroup::GroupState::~GroupState()
{}

bool TaskGroup::GroupState::Evaluate()
{
  std::lock_guard<std::mutex> state_lock(m_mutex);
  for (auto& minimal_state : m_minimal_states_list)
  {
    // If current state is worse than the minimal state
    TaskState current_state = minimal_state.first->GetMonitoringTaskState();
    if (current_state.severity == eCAL_Process_eSeverity::proc_sev_unknown || current_state > minimal_state.second)
    {
      return false;
    }
  }
  return true;
}

void TaskGroup::GroupState::SetName(const std::string& name)
{
  std::lock_guard<std::mutex> state_lock(m_mutex);
  m_name = name;
}

std::string TaskGroup::GroupState::GetName()
{
  std::lock_guard<std::mutex> state_lock(m_mutex);
  return m_name;
}

void TaskGroup::GroupState::SetColor(Color color)
{
  std::lock_guard<std::mutex> state_lock(m_mutex);
  m_color = color;
}

TaskGroup::GroupState::Color TaskGroup::GroupState::GetColor()
{
  std::lock_guard<std::mutex> state_lock(m_mutex);
  return m_color;
}

std::list<std::pair<std::shared_ptr<EcalSysTask>, TaskState>> TaskGroup::GroupState::GetMinimalStatesList()
{
  std::lock_guard<std::mutex> state_lock(m_mutex);
  return m_minimal_states_list;
}

void TaskGroup::GroupState::SetMinimalStatesList(const std::list<std::pair<std::shared_ptr<EcalSysTask>, TaskState>>& minimal_states_list)
{
  std::lock_guard<std::mutex> state_lock(m_mutex);
  m_minimal_states_list = minimal_states_list;
}
