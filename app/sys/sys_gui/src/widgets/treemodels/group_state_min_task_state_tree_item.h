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

#include "CustomQt/QAbstractTreeItem.h"
#include "task_tree_item.h"

#include <memory>
#include <ecalsys/ecal_sys.h>

class GroupStateMinTaskStateTreeItem :
  public QAbstractTreeItem
{
public:

  enum class Columns : int
  {
    ID,
    LAUNCH_ORDER,
    TASK_NAME,
    TARGET_NAME,
    RUNNER_NAME,
    SEVERITY,
    SEVERITY_LEVEL,
    PATH,
    WORKING_DIR,
    COMMAND_LINE_ARGS,
    VISIBILITY,
    TIMEOUT_AFTER_START,
    MONITORING_ENABLED,
    RESTART_BY_SEVERITY_ENABLED,
    RESTART_AT_SEVERITY,

    // Current State
    CURRENT_PID,
    CURRENT_HOST,
    STATE,
    INFO,

    COLUMN_COUNT
  };

  GroupStateMinTaskStateTreeItem(std::pair<std::shared_ptr<EcalSysTask>, TaskState> min_task_state);
  ~GroupStateMinTaskStateTreeItem();

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  bool setData(int column, const QVariant& data, Qt::ItemDataRole role = Qt::ItemDataRole::EditRole);

  Qt::ItemFlags flags(int column) const;

  std::pair<std::shared_ptr<EcalSysTask>, TaskState> getMinTaskState() const;

  int type() const;

private:
  TaskTreeItem task_tree_item_;                                                 /**< We want to reuse some code from the TaskTreeItem without inheriting from it, so we have a member here*/
  std::pair<std::shared_ptr<EcalSysTask>, TaskState> min_task_state_;
};

