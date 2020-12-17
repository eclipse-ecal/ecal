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
#include <string>
#include <tuple>
#include <atomic>
#include <memory>
#include <mutex>
#include <utility>
#include <set>

#include <ecal/ecal.h>

#include "ecalsys/task/ecal_sys_task.h"
#include "ecalsys/task/task_state.h"

/**
 * @brief A Class representing a group of Tasks
 *
 * A Task group can be used for evaluating a "complete" severity of all tasks in
 * the group. Therefore, a TaskGroup consists of multiple possible GroupStates
 * each containing a list of Tasks and their minimal severity level that is
 * required for this state being the active one.
 */
class TaskGroup
{
public:
  //////////////////////////////////////////////////////////////////////////////
  //// Nested GroupState class                                              ////
  //////////////////////////////////////////////////////////////////////////////

  /**
   * @brief A Class representing one possible state of a TaskGroup
   *
   * A GroupState consists of tasks and the minimum health (severity) of each
   * task. A group can evaluate to either being active or not.
   */
  class GroupState
  {
  public:
    ////////////////////////////////////////////////////////////////////////////
    //// Nested Color struct                                                ////
    ////////////////////////////////////////////////////////////////////////////
    struct Color
    {
      uint8_t red;
      uint8_t green;
      uint8_t blue;
      Color(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
      Color(int r, int g, int b) : Color((uint8_t)r, (uint8_t)g, (uint8_t)b) {}
      Color() : Color(0, 0, 0) {}
      bool operator ==(const Color &c) const { return (c.red == red) && (c.green == green) && (c.blue == blue); }
      bool operator !=(const Color &c) const { return !(operator ==(c)); }
    };

    GroupState();
    ~GroupState();

    /**
     * @brief Evaluate if this state is active
     *
     * The evaluation is done by comparing all tasks that are assigned to this
     * state with their according minimum health level. The evaluation results
     * to true, if:
     *  1. No state has unknown severity
     *  2. All states are healthier than (or equal to) their minimum severity
     *
     * @return true if this state is active
     */
    bool Evaluate();

    /**
     * @brief Sets the name of this state
     * @param name the new name
     */
    void SetName(const std::string& name);

    /**
     * @brief Gets the name of this state
     * @return the name of this state
     */
    std::string GetName();

    /**
     * @brief Sets the color of this state thas is displayed in the GUI when the state is active
     * @param color The new color
     */
    void SetColor(Color color);
    
    /**
     * @brief Gets the color of this state that is displayed in the GUI when the state is active
     * @return The color that should be displayed in the GUI whe this GroupState is active
     */
    TaskGroup::GroupState::Color GetColor();
    
    /**
     * @brief Returns the list of (task, minimal_severity) pairs as copy
     *
     * @return a list of (task, minimal severity) pairs
     */
    std::list<std::pair<std::shared_ptr<EcalSysTask>, TaskState>> GetMinimalStatesList();

    /**
     * @brief Sets the list of (task, minimal_severity) pairs of this TaskGroup
     *
     * This list will be evaluated when evaluating this TaskGroupState.
     *
     * @param minimal_states_list The new list of minimal severity for all tasks
     */
    void SetMinimalStatesList(const std::list<std::pair<std::shared_ptr<EcalSysTask>, TaskState>>& minimal_states_list);

  private:
    std::mutex            m_mutex;          /**< Mutex for thread safety */

    std::string           m_name;           /**< The name of this group state */
    Color                 m_color;          /**< The color to display in the GUI when this state is active*/
    std::list<std::pair<std::shared_ptr<EcalSysTask>, TaskState>> m_minimal_states_list;  /**< A list of (task, minimal_state_for_task) pairs */
  };



  //////////////////////////////////////////////////////////////////////////////
  //// Actual TaskGroup                                                     ////
  //////////////////////////////////////////////////////////////////////////////

  TaskGroup();

  ~TaskGroup();

  /**
   * @brief Evaluates the possible states and returns the active state
   *
   * If there is no state active, a Nullpointer will be returned. The states are
   * evaluated in the order of their appearence in the state list
   * (@see GetGroupStateList()). The first state that is active is considered to be
   * the current state.
   *
   * @return The active state or a nullpointer if no state is active
   */
  std::shared_ptr<GroupState> Evaluate();

  /**
   * @brief Creates a set of all tasks that are referenced by any possible group state
   * @return a set of all Tasks
   */
  std::set<std::shared_ptr<EcalSysTask>> GetAllTasks();

  /**
   * @brief Sets the name of this TaskGroup
   * @param name The new name of this TaskGroup
   */
  void SetName(const std::string& name);

  /**
   * @brief Gets the name of this TaskGroup
   * @return The name of this TaskGroup
   */
  std::string GetName();

  /**
   * @brief Sets the ID of the TaskGroup.
   *
   * The ID should not be set manually, as it will be set by Ecalsys when the
   * group is added.
   * The ID actually is not used for anything and is just there to keep
   * compatibility.
   *
   * @param id the new ID
   */
  void SetId(uint32_t id);

  /**
   * @brief Gets the ID of the TaskGroup.
   *
   * The ID actually is not used for anything and is just there to keep
   * compatibility.
   *
   * @return the ID of the TaskGroup
   */
  uint32_t GetId();

  /**
   * @brief Returns the list of possible states as copy.
   *
   * When evaluating the Taskgroup with @see Evaluate(), the order of the States
   * are used to determine which state is the current state. The first state in
   * the list beeing active is considered to be the active state.
   *
   * @return the list of possible states
   */
  std::list<std::shared_ptr<GroupState>> GetGroupStateList();

  /**
   * @brief Sets the list of possible states
   *
   * When evaluating the Taskgroup with @see Evaluate(), the order of the States
   * are used to determine which state is the current state. The first state in
   * the list beeing active is considered to be the active state.
   *
   * @param group_state_list The new list of possible states
   */
  void SetGroupStateList(const std::list<std::shared_ptr<GroupState>>& group_state_list);

private:
  uint32_t m_id;                                                                /**< The ID of this TaskGroup when saving it to a file*/

  std::mutex m_mutex;                                                           /**< A mutex for thread safe operation on the name, group_state_list etc.*/
  std::string m_name;                                                           /**< The name of this TaskGroup*/
  std::list<std::shared_ptr<GroupState>> m_group_state_list;                    /**< A list of possible states*/
};

