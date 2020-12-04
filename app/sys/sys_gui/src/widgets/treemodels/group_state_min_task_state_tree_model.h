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

#include "group_state_min_task_state_tree_item.h"

class GroupStateMinTaskStateTreeModel :
  public QAbstractTreeModel
{
  Q_OBJECT

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

  GroupStateMinTaskStateTreeModel(QObject *parent = nullptr);
  ~GroupStateMinTaskStateTreeModel();

  void setGroupState(std::shared_ptr<TaskGroup::GroupState> group_state);

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;

public slots:
  void addMinTaskState(const std::pair<std::shared_ptr<EcalSysTask>, TaskState>& min_state);
  void removeTask(std::shared_ptr<EcalSysTask> task);
  void updateTask(std::shared_ptr<EcalSysTask> task);

private:
  std::shared_ptr<TaskGroup::GroupState> group_state_;

  std::map<Columns, QString> columnLabels = {
    { Columns::ID,                                      tr("ID") },
    { Columns::TASK_NAME,                               tr("Selected Tasks") },
    { Columns::TARGET_NAME,                             tr("Target") },
    { Columns::RUNNER_NAME,                             tr("Runner") },
    { Columns::SEVERITY,                                tr("Min. Severity") },
    { Columns::SEVERITY_LEVEL,                          tr("Min. Level") },
    { Columns::PATH,                                    tr("Path") },
    { Columns::WORKING_DIR,                             tr("Working dir") },
    { Columns::COMMAND_LINE_ARGS,                       tr("Command line") },
    { Columns::VISIBILITY,                              tr("Visibility") },
    { Columns::LAUNCH_ORDER,                            tr("Launch Order") },
    { Columns::TIMEOUT_AFTER_START,                     tr("Timeout after start") },
    { Columns::MONITORING_ENABLED,                      tr("Monitoring") },
    { Columns::RESTART_BY_SEVERITY_ENABLED,             tr("Restart by severity") },
    { Columns::RESTART_AT_SEVERITY,                     tr("Restart at severity") },
    { Columns::CURRENT_PID,                             tr("PID") },
    { Columns::CURRENT_HOST,                            tr("Current Host") },
    { Columns::STATE,                                   tr("State") },
    { Columns::INFO,                                    tr("Info") }
  };

  std::map<Columns, int> min_task_state_tree_item_column_mapping =
  {
    { Columns::ID,                                      (int)GroupStateMinTaskStateTreeItem::Columns::ID },
    { Columns::TASK_NAME,                               (int)GroupStateMinTaskStateTreeItem::Columns::TASK_NAME },
    { Columns::TARGET_NAME,                             (int)GroupStateMinTaskStateTreeItem::Columns::TARGET_NAME },
    { Columns::RUNNER_NAME,                             (int)GroupStateMinTaskStateTreeItem::Columns::RUNNER_NAME },
    { Columns::SEVERITY,                                (int)GroupStateMinTaskStateTreeItem::Columns::SEVERITY },
    { Columns::SEVERITY_LEVEL,                          (int)GroupStateMinTaskStateTreeItem::Columns::SEVERITY_LEVEL },
    { Columns::PATH,                                    (int)GroupStateMinTaskStateTreeItem::Columns::PATH },
    { Columns::WORKING_DIR,                             (int)GroupStateMinTaskStateTreeItem::Columns::WORKING_DIR },
    { Columns::COMMAND_LINE_ARGS,                       (int)GroupStateMinTaskStateTreeItem::Columns::COMMAND_LINE_ARGS },
    { Columns::VISIBILITY,                              (int)GroupStateMinTaskStateTreeItem::Columns::VISIBILITY },
    { Columns::LAUNCH_ORDER,                            (int)GroupStateMinTaskStateTreeItem::Columns::LAUNCH_ORDER },
    { Columns::TIMEOUT_AFTER_START,                     (int)GroupStateMinTaskStateTreeItem::Columns::TIMEOUT_AFTER_START },
    { Columns::MONITORING_ENABLED,                      (int)GroupStateMinTaskStateTreeItem::Columns::MONITORING_ENABLED },
    { Columns::RESTART_BY_SEVERITY_ENABLED,             (int)GroupStateMinTaskStateTreeItem::Columns::RESTART_BY_SEVERITY_ENABLED },
    { Columns::RESTART_AT_SEVERITY,                     (int)GroupStateMinTaskStateTreeItem::Columns::RESTART_AT_SEVERITY },
    { Columns::CURRENT_PID,                             (int)GroupStateMinTaskStateTreeItem::Columns::CURRENT_PID },
    { Columns::CURRENT_HOST,                            (int)GroupStateMinTaskStateTreeItem::Columns::CURRENT_HOST },
    { Columns::STATE,                                   (int)GroupStateMinTaskStateTreeItem::Columns::STATE },
    { Columns::INFO,                                    (int)GroupStateMinTaskStateTreeItem::Columns::INFO }
  };
};

