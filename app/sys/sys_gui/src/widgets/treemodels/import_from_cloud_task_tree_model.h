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

class ImportFromCloudTaskTreeModel :
  public QAbstractTreeModel
{
  Q_OBJECT

public:
  enum class Columns : int
  {
    IMPORT_CHECKED,

    // Config
    TASK_NAME,
    TARGET_NAME,
    RUNNER_NAME,
    PATH,
    COMMAND_LINE_ARGS,

    // Current State
    CURRENT_PID,
    CURRENT_HOST,
    STATE,
    INFO,

    COLUMN_COUNT
  };

  ImportFromCloudTaskTreeModel(QObject *parent = nullptr);
  ~ImportFromCloudTaskTreeModel();

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

signals:
  //void checkboxStateChanged(std::shared_ptr<EcalSysTask> task, bool checked);

private:
  std::map<Columns, QString> columnLabels = {
    { Columns::IMPORT_CHECKED,       tr("") },
    { Columns::TASK_NAME,            tr("Task") },
    { Columns::TARGET_NAME,          tr("Target Host") },
    { Columns::RUNNER_NAME,          tr("Runner") },
    { Columns::PATH,                 tr("Path") },
    { Columns::COMMAND_LINE_ARGS,    tr("Command line") },
    { Columns::CURRENT_PID,          tr("PID") },
    { Columns::CURRENT_HOST,         tr("Current Host") },
    { Columns::STATE,                tr("State") },
    { Columns::INFO,                 tr("Info") }
  };

  std::map<ImportFromCloudTaskTreeModel::Columns, int> task_tree_item_column_mapping = {
    { ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED,    (int)TaskTreeItem::Columns::IMPORT_CHECKED },
    { ImportFromCloudTaskTreeModel::Columns::TASK_NAME,         (int)TaskTreeItem::Columns::TASK_NAME },
    { ImportFromCloudTaskTreeModel::Columns::TARGET_NAME,       (int)TaskTreeItem::Columns::TARGET_NAME },
    { ImportFromCloudTaskTreeModel::Columns::RUNNER_NAME,       (int)TaskTreeItem::Columns::RUNNER_NAME },
    { ImportFromCloudTaskTreeModel::Columns::PATH,              (int)TaskTreeItem::Columns::PATH },
    { ImportFromCloudTaskTreeModel::Columns::COMMAND_LINE_ARGS, (int)TaskTreeItem::Columns::COMMAND_LINE_ARGS },
    { ImportFromCloudTaskTreeModel::Columns::CURRENT_PID,       (int)TaskTreeItem::Columns::CURRENT_PID },
    { ImportFromCloudTaskTreeModel::Columns::CURRENT_HOST,      (int)TaskTreeItem::Columns::CURRENT_HOST },
    { ImportFromCloudTaskTreeModel::Columns::STATE,             (int)TaskTreeItem::Columns::STATE },
    { ImportFromCloudTaskTreeModel::Columns::INFO,              (int)TaskTreeItem::Columns::INFO },
  };
};

