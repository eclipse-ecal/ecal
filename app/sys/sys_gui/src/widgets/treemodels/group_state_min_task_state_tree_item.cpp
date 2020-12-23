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

#include "group_state_min_task_state_tree_item.h"

#include <QColor>
#include <QFont>

#include "tree_item_types.h"
#include "tree_data_roles.h"

GroupStateMinTaskStateTreeItem::GroupStateMinTaskStateTreeItem(std::pair<std::shared_ptr<EcalSysTask>, TaskState> min_task_state)
  : QAbstractTreeItem()
  , task_tree_item_(min_task_state.first)
  , min_task_state_(min_task_state)
{
}


GroupStateMinTaskStateTreeItem::~GroupStateMinTaskStateTreeItem()
{
}

QVariant GroupStateMinTaskStateTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant GroupStateMinTaskStateTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if (column == Columns::TASK_NAME)
  {
    // For the Task Name Column we simply reuse TaskTreeItem's code
    return task_tree_item_.data(TaskTreeItem::Columns::TASK_NAME, role);
  }

  // For everything except the SEVERITY and SEVERITY_LEVEL role we re-use the TaskTreeItem's code
  switch (column)
  {
  case Columns::ID:
    return task_tree_item_.data(TaskTreeItem::Columns::ID, role);
    break;
  case Columns::LAUNCH_ORDER:
    return task_tree_item_.data(TaskTreeItem::Columns::LAUNCH_ORDER, role);
    break;
  case Columns::TASK_NAME:
    return task_tree_item_.data(TaskTreeItem::Columns::TASK_NAME, role);
    break;
  case Columns::TARGET_NAME:
    return task_tree_item_.data(TaskTreeItem::Columns::TARGET_NAME, role);
    break;
  case Columns::RUNNER_NAME:
    return task_tree_item_.data(TaskTreeItem::Columns::RUNNER_NAME, role);
    break;
  case Columns::PATH:
    return task_tree_item_.data(TaskTreeItem::Columns::PATH, role);
    break;
  case Columns::WORKING_DIR:
    return task_tree_item_.data(TaskTreeItem::Columns::WORKING_DIR, role);
    break;
  case Columns::COMMAND_LINE_ARGS:
    return task_tree_item_.data(TaskTreeItem::Columns::COMMAND_LINE_ARGS, role);
    break;
  case Columns::VISIBILITY:
    return task_tree_item_.data(TaskTreeItem::Columns::VISIBILITY, role);
    break;
  case Columns::TIMEOUT_AFTER_START:
    return task_tree_item_.data(TaskTreeItem::Columns::TIMEOUT_AFTER_START, role);
    break;
  case Columns::MONITORING_ENABLED:
    return task_tree_item_.data(TaskTreeItem::Columns::MONITORING_ENABLED, role);
    break;
  case Columns::RESTART_BY_SEVERITY_ENABLED:
    return task_tree_item_.data(TaskTreeItem::Columns::RESTART_BY_SEVERITY_ENABLED, role);
    break;
  case Columns::RESTART_AT_SEVERITY:
    return task_tree_item_.data(TaskTreeItem::Columns::RESTART_AT_SEVERITY, role);
    break;
  case Columns::CURRENT_PID:
    return task_tree_item_.data(TaskTreeItem::Columns::CURRENT_PID, role);
    break;
  case Columns::CURRENT_HOST:
    return task_tree_item_.data(TaskTreeItem::Columns::CURRENT_HOST, role);
    break;
  case Columns::STATE:
    return task_tree_item_.data(TaskTreeItem::Columns::STATE, role);
    break;
  case Columns::INFO:
    return task_tree_item_.data(TaskTreeItem::Columns::INFO, role);
    break;
  default:
    break;
  }
  

  if (role == Qt::ItemDataRole::DisplayRole)
  {
    if (column == Columns::SEVERITY)
    {
      std::string severity, severity_level;
      min_task_state_.second.ToString(severity, severity_level);
      return QString(severity.c_str());
    }
    else if (column == Columns::SEVERITY_LEVEL)
    {
      std::string severity, severity_level;
      min_task_state_.second.ToString(severity, severity_level);
      return QString(severity_level.c_str());
    }
    else
    {
      return QVariant();
    }
  }



  if ((role == ItemDataRoles::FilterRole) || (role == ItemDataRoles::SortRole)) //-V560
  {
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  if (role == Qt::ItemDataRole::EditRole)
  {
    if (column == Columns::SEVERITY)
    {
      return (int)min_task_state_.second.severity;
    }
    else if (column == Columns::SEVERITY_LEVEL)
    {
      return (int)min_task_state_.second.severity_level;
    }
  }

  return QVariant::Invalid;
}

bool GroupStateMinTaskStateTreeItem::setData(int column, const QVariant& data, Qt::ItemDataRole role)
{
  if (role == Qt::EditRole)
  {
    if (column == (int)Columns::SEVERITY)
    {
      min_task_state_.second.severity = (eCAL_Process_eSeverity)(data.toInt());
      return true;
    }
    else if (column == (int)Columns::SEVERITY_LEVEL)
    {
      min_task_state_.second.severity_level = (eCAL_Process_eSeverity_Level)(data.toInt());
      return true;
    }
  }

  return false;
}

Qt::ItemFlags GroupStateMinTaskStateTreeItem::flags(int column) const
{
  switch ((Columns)column)
  {
  case Columns::TASK_NAME:
    return QAbstractTreeItem::flags(column);
  case Columns::SEVERITY:
    return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable;
  case Columns::SEVERITY_LEVEL:
    return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable;
  default:
    return QAbstractTreeItem::flags(column);
  }
}

std::pair<std::shared_ptr<EcalSysTask>, TaskState> GroupStateMinTaskStateTreeItem::getMinTaskState() const
{
  return min_task_state_;
}


int GroupStateMinTaskStateTreeItem::type() const
{
  return (int)TreeItemType::GroupStateMinTaskState;
}