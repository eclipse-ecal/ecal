/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "task_tree_item.h"

#include <QColor>
#include <QFont>
#include "globals.h"

#include "tree_item_types.h"
#include "tree_data_roles.h"

TaskTreeItem::TaskTreeItem(std::shared_ptr<EcalSysTask> task)
  : QAbstractTreeItem()
  , task_(task)
  , import_checked_(true)
  , import_checkbox_enabled_(true)
{
}


TaskTreeItem::~TaskTreeItem()
{
}

QVariant TaskTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant TaskTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  // Text Content
  if (role == Qt::ItemDataRole::DisplayRole)
  {
    if (column == Columns::ID)
    {
      return (unsigned long long) task_->GetId();
    }
    else if (column == Columns::TASK_NAME)
    {
      QString name(task_->GetName().c_str());
      if (name.isEmpty())
        return "Unnamed Task";
      else
        return name;
    }
    else if (column == Columns::TARGET_NAME)
    {
      return QString(task_->GetTarget().c_str());
    }
    else if (column == Columns::RUNNER_NAME)
    {
      auto runner = task_->GetRunner();
      if (runner)
        return runner->GetName().c_str();
      else
        return "";
    }
    else if (column == Columns::PATH)
    {
      return task_->GetAlgoPath().c_str();
    }
    else if (column == Columns::WORKING_DIR)
    {
      return task_->GetWorkingDir().c_str();
    }
    else if (column == Columns::COMMAND_LINE_ARGS)
    {
      return task_->GetCommandLineArguments().c_str();
    }
    else if (column == Columns::VISIBILITY)
    {
      switch (task_->GetVisibility())
      {
      case eCAL::Process::eStartMode::hidden:
        return "Hidden";
      case eCAL::Process::eStartMode::maximized:
        return "Maximized";
      case eCAL::Process::eStartMode::minimized:
        return "Minimized";
      default:
        return "Normal";
      }
    }
    else if (column == Columns::LAUNCH_ORDER)
    {
      return task_->GetLaunchOrder();
    }
    else if (column == Columns::TIMEOUT_AFTER_START)
    {
      return QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(task_->GetTimeoutAfterStart()).count()) + " ms";
    }
    else if (column == Columns::MONITORING_ENABLED)
    {
      return task_->IsMonitoringEnabled();
    }
    else if (column == Columns::RESTART_BY_SEVERITY_ENABLED)
    {
      return task_->IsRestartBySeverityEnabled();
    }
    else if (column == Columns::RESTART_AT_SEVERITY)
    {
      std::string state, level;
      TaskState restart_at_severity = task_->GetRestartAtSeverity();
      restart_at_severity.ToString(state, level);
      return QString(state.c_str()) +
        (restart_at_severity.severity != eCAL::Process::eSeverity::unknown ?
          " (Lv " + QString::number(static_cast<int>(restart_at_severity.severity_level)) + ")" :
          "");
    }
    else if (column == Columns::CURRENT_PID)
    {
      auto pid_list = task_->GetPids();
      QStringList pid_strings;
      for (auto process_id : pid_list)
      {
        pid_strings.push_back(QString::number(process_id));
      }
      return pid_strings.join(", ");
    }
    else if (column == Columns::CURRENT_HOST)
    {
      return QString(task_->GetHostStartedOn().c_str());
    }
    else if (column == Columns::STATE)
    {
      std::string state, level;
      task_->GetMonitoringTaskState().ToString(state, level);
      return QString(state.c_str()) +
        (task_->GetMonitoringTaskState().severity != eCAL::Process::eSeverity::unknown ?
          " (Lv " + QString::number(static_cast<int>(task_->GetMonitoringTaskState().severity_level)) + ")" :
          "");
    }
    else if (column == Columns::INFO)
    {
      return QString(task_->GetMonitoringTaskState().info.c_str());
    }
    else
    {
      return QVariant(); // Invalid QVariant
    }
  }

  // Font
  else if (role == Qt::ItemDataRole::FontRole)
  {
    QFont font;

    if (column == Columns::TASK_NAME)
    {
      QString name(task_->GetName().c_str());
      if (name.isEmpty())
      {
        font.setItalic(true);
      }
    }

    auto options = Globals::EcalSysInstance()->GetOptions();

    // Strike out everything that is not on this host
    if (options.local_tasks_only
      && (QString::compare(task_->GetTarget().c_str(), eCAL::Process::GetHostName().c_str(), Qt::CaseSensitivity::CaseInsensitive) != 0))
    {
      font.setStrikeOut(true);
    }
    // Strike out the Hostnames of all Tasks that are not local
    else if ((options.use_localhost_for_all_tasks)
      && (column == Columns::TARGET_NAME)
      && (QString::compare(task_->GetTarget().c_str(), eCAL::Process::GetHostName().c_str(), Qt::CaseSensitivity::CaseInsensitive) != 0))
    {
      font.setStrikeOut(true);
    }
      
    return font;

  }

  // Background State Colors
  else if (role == Qt::ItemDataRole::BackgroundRole)
  {
    if (column == Columns::STATE)
    {
      TaskState state = task_->GetMonitoringTaskState();

      switch (state.severity)
      {
      case eCAL::Process::eSeverity::healthy:
        return QColor(80, 225, 120);
      case eCAL::Process::eSeverity::warning:
        return QColor(240, 240, 50);
      case eCAL::Process::eSeverity::critical:
        return QColor(250, 130, 0);
      case eCAL::Process::eSeverity::failed:
        return QColor(240, 20, 20);
      default:
        return QVariant(); // Invalid QVariant
      }
    }
  }

  // Grey out Tasks / Host names based on the current options
  else if (role == Qt::ItemDataRole::ForegroundRole)
  {
    auto options = Globals::EcalSysInstance()->GetOptions();

    if (options.local_tasks_only)
    {
      // Grey out everything that is not on this host
      if (QString::compare(task_->GetTarget().c_str(), eCAL::Process::GetHostName().c_str(), Qt::CaseSensitivity::CaseInsensitive) != 0)
      {
        return QColor(128, 128, 128);
      }
      else
      {
        return QVariant(); // Invalid QVariant
      }
    }
    else if (options.use_localhost_for_all_tasks)
    {
      // Grey out the Hostnames of all Tasks that are not local
      if (column == Columns::TARGET_NAME
        && QString::compare(task_->GetTarget().c_str(), eCAL::Process::GetHostName().c_str(), Qt::CaseSensitivity::CaseInsensitive) != 0)
      {
        return QColor(128, 128, 128);
      }
      else
      {
        return QVariant(); // Invalid QVariant
      }
    }
  }

  // Sorting
  else if (role == ItemDataRoles::SortRole)
  {
    if (column == Columns::IMPORT_CHECKED)
    {
      return data(column, Qt::ItemDataRole::CheckStateRole);
    }
    if (column == Columns::TIMEOUT_AFTER_START)
    {
      return (long long) task_->GetTimeoutAfterStart().count();
    }
    if (column == Columns::RESTART_AT_SEVERITY)
    {
      return task_->GetRestartAtSeverity().ToInt();
    }
    if (column == Columns::STATE)
    {
      return task_->GetMonitoringTaskState().ToInt();
    }
    else if (column == Columns::CURRENT_PID)
    {
      auto pid_list = task_->GetPids();
      if (pid_list.empty())
      {
        return 0;
      }
      else
      {
        return pid_list[0];
      }
    }
    else
    {
      return data(column, Qt::ItemDataRole::DisplayRole);
    }
  }

  // Filtering
  else if (role == ItemDataRoles::FilterRole) //-V547
  {
    if (column == Columns::TASK_NAME)
    {
      // As we return "Unnamed Task" for tasks without name as display role, we correct it back to an empty string for filtering
      return QString(task_->GetName().c_str());
    }
    else if (column == Columns::IMPORT_CHECKED)
    {
      // Enable filtering based on the enabled-state of the checkbox. Yes, we should probably find a proper solution. But it works fine.
      return isImportCheckboxEnabled() ? "enabled" : "disabled";
    }
    else
    {
      return data(column, Qt::ItemDataRole::DisplayRole);
    }
  }

  else if (role == Qt::ItemDataRole::CheckStateRole)
  {
    if (column == Columns::IMPORT_CHECKED)
    {
      return (isImportChecked() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    }
  }

  else if (role == Qt::ItemDataRole::ToolTipRole)
  {
    if (column == Columns::IMPORT_CHECKED)
    {
      return isImportCheckboxEnabled() ? "" : import_disabled_reason_;
    }
    else
    {
      return data(column, Qt::ItemDataRole::DisplayRole);
    }
  }

  return QVariant(); // Invalid QVariant

}

bool TaskTreeItem::setData(int column, const QVariant& data, Qt::ItemDataRole role)
{
  if (role == Qt::ItemDataRole::CheckStateRole)
  {
    if (column == (int)Columns::IMPORT_CHECKED)
    {
      import_checked_ = data.toBool();
      return true;
    }
  }
  return QAbstractTreeItem::setData(column, data, role);
}

Qt::ItemFlags TaskTreeItem::flags(int column) const
{
  if (column == (int)Columns::IMPORT_CHECKED)
  {
    if (import_checkbox_enabled_)
      return QAbstractTreeItem::flags(column) | Qt::ItemFlag::ItemIsUserCheckable;
    else
      return (QAbstractTreeItem::flags(column) | Qt::ItemFlag::ItemIsUserCheckable) & ~Qt::ItemFlag::ItemIsEnabled;
  }

  return QAbstractTreeItem::flags(column);
}

int TaskTreeItem::type() const
{
  return (int)TreeItemType::Task;
}

std::shared_ptr<EcalSysTask> TaskTreeItem::getTask() const
{
  return task_;
}

bool TaskTreeItem::isImportChecked() const
{
  return import_checked_;
}

void TaskTreeItem::setImportCheckboxEnabled(bool enabled)
{
  import_checkbox_enabled_ = enabled;
}

bool TaskTreeItem::isImportCheckboxEnabled() const
{
  return import_checkbox_enabled_;
}

void TaskTreeItem::setImportDisabledReason(const QString& reason)
{
  import_disabled_reason_ = reason;
}
