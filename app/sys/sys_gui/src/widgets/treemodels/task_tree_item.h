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

#include <QObject>

#include "CustomQt/QAbstractTreeItem.h"

#include <memory>
#include <ecalsys/task/ecal_sys_task.h>

class TaskTreeItem :
  public QAbstractTreeItem
{
public:
  enum class Columns : int
  {
    // Config
    ID,
    TASK_NAME,
    TARGET_NAME,
    RUNNER_NAME,
    PATH,
    WORKING_DIR,
    COMMAND_LINE_ARGS,
    VISIBILITY,
    LAUNCH_ORDER,
    TIMEOUT_AFTER_START,
    MONITORING_ENABLED,
    RESTART_BY_SEVERITY_ENABLED,
    RESTART_AT_SEVERITY,

    // Current State
    CURRENT_PID,
    CURRENT_HOST,
    STATE,
    INFO,

    // Single-purpose information
    IMPORT_CHECKED,

    COLUMN_COUNT
  };

  TaskTreeItem(std::shared_ptr<EcalSysTask> task);
  ~TaskTreeItem();

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  bool setData(int column, const QVariant& data, Qt::ItemDataRole role = Qt::ItemDataRole::EditRole);

  Qt::ItemFlags flags(int column) const;

  int type() const;

  std::shared_ptr<EcalSysTask> getTask() const;

  bool isImportChecked() const;

  void setImportCheckboxEnabled(bool enabled);
  bool isImportCheckboxEnabled() const;
  void setImportDisabledReason(const QString& reason);

private:
  std::shared_ptr<EcalSysTask> task_;

  bool import_checked_;
  bool import_checkbox_enabled_;
  QString import_disabled_reason_;
};

