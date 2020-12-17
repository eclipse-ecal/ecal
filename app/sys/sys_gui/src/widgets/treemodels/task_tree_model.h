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

#include "CustomQt/QAbstractTreeModel.h"
#include "task_tree_item.h"

class TaskTreeModel :
  public QAbstractTreeModel
{
  Q_OBJECT

public:
  enum class Columns : int
  {
    // Config
    ID,
    LAUNCH_ORDER,
    TASK_NAME,
    TARGET_NAME,
    RUNNER_NAME,
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

  TaskTreeModel(QObject *parent = nullptr);
  ~TaskTreeModel();

  void reload();

  void reload(const std::list<std::shared_ptr<EcalSysTask>>& task_list);

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  using QAbstractTreeModel::index;
  QModelIndex index(const std::shared_ptr<EcalSysTask>& task, int column = 0);

  void setHeaderLabel(Columns column, const QString& label);

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;

public slots:
  void addTask(std::shared_ptr<EcalSysTask> new_task);
  void removeTask(std::shared_ptr<EcalSysTask> task);
  void updateTask(std::shared_ptr<EcalSysTask> task);

private:
  std::map<Columns, QString> columnLabels = 
  {
    { Columns::ID,                   tr("ID") },
    { Columns::LAUNCH_ORDER,         tr("Launch#") },
    { Columns::TASK_NAME,            tr("Task") },
    { Columns::TARGET_NAME,          tr("Target Host") },
    { Columns::RUNNER_NAME,          tr("Runner") },
    { Columns::PATH,                 tr("Path") },
    { Columns::WORKING_DIR,          tr("Working dir") },
    { Columns::COMMAND_LINE_ARGS,    tr("Command line") },
    { Columns::VISIBILITY,           tr("Visibility") },
    { Columns::TIMEOUT_AFTER_START,  tr("Waiting time") },
    { Columns::MONITORING_ENABLED,   tr("Monitoring") },
    { Columns::RESTART_BY_SEVERITY_ENABLED, tr("Restart by severity") },
    { Columns::RESTART_AT_SEVERITY,  tr("Restart at severity") },
    { Columns::CURRENT_PID,          tr("PID") },
    { Columns::CURRENT_HOST,         tr("Current Host") },
    { Columns::STATE,                tr("State") },
    { Columns::INFO,                 tr("Info") }
  };

  std::map<Columns, int> task_tree_item_column_mapping = 
  {
    { Columns::ID,                   (int)TaskTreeItem::Columns::ID },
    { Columns::LAUNCH_ORDER,         (int)TaskTreeItem::Columns::LAUNCH_ORDER },
    { Columns::TASK_NAME,            (int)TaskTreeItem::Columns::TASK_NAME },
    { Columns::TARGET_NAME,          (int)TaskTreeItem::Columns::TARGET_NAME },
    { Columns::RUNNER_NAME,          (int)TaskTreeItem::Columns::RUNNER_NAME },
    { Columns::PATH,                 (int)TaskTreeItem::Columns::PATH },
    { Columns::WORKING_DIR,          (int)TaskTreeItem::Columns::WORKING_DIR },
    { Columns::COMMAND_LINE_ARGS,    (int)TaskTreeItem::Columns::COMMAND_LINE_ARGS },
    { Columns::VISIBILITY,           (int)TaskTreeItem::Columns::VISIBILITY },
    { Columns::TIMEOUT_AFTER_START,  (int)TaskTreeItem::Columns::TIMEOUT_AFTER_START },
    { Columns::MONITORING_ENABLED,   (int)TaskTreeItem::Columns::MONITORING_ENABLED },
    { Columns::RESTART_BY_SEVERITY_ENABLED, (int)TaskTreeItem::Columns::RESTART_BY_SEVERITY_ENABLED },
    { Columns::RESTART_AT_SEVERITY,  (int)TaskTreeItem::Columns::RESTART_AT_SEVERITY },
    { Columns::CURRENT_PID,          (int)TaskTreeItem::Columns::CURRENT_PID },
    { Columns::CURRENT_HOST,         (int)TaskTreeItem::Columns::CURRENT_HOST },
    { Columns::STATE,                (int)TaskTreeItem::Columns::STATE },
    { Columns::INFO,                 (int)TaskTreeItem::Columns::INFO }
  };
};

