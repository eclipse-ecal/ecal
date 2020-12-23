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

#include "runner_tree_item.h"

class RunnerTreeModel :
  public QAbstractTreeModel
{
  Q_OBJECT

public:
  enum class Columns : int
  {
    ID,
    NAME,
    PATH,
    DEFAULT_ALGO_DIR,
    COMMAND_ARG,

    COLUMN_COUNT
  };

  RunnerTreeModel(QObject *parent = nullptr);
  ~RunnerTreeModel();

  void reload(bool add_empty_runner = true);

  void reload(const std::list<std::shared_ptr<EcalSysRunner>>& runner_list, bool add_empty_runner = true);

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  using QAbstractTreeModel::index;
  QModelIndex index(const std::shared_ptr<EcalSysRunner>& runner, int column = 0);

  int rowOf(std::shared_ptr<EcalSysRunner> runner);

  std::shared_ptr<EcalSysRunner> getRunnerByRow(int row);

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;

public slots:
  void addRunner(std::shared_ptr<EcalSysRunner> new_runner);
  void removeRunner(std::shared_ptr<EcalSysRunner> runner);
  void updateRunner(std::shared_ptr<EcalSysRunner> runner);

private:
  std::map<Columns, QString> columnLabels = {
    { Columns::ID,               tr("ID") },
    { Columns::NAME,             tr("Runner") },
    { Columns::PATH,             tr("Path") },
    { Columns::DEFAULT_ALGO_DIR, tr("Default algo dir") },
    { Columns::COMMAND_ARG,      tr("Command line") },
  };

  std::map<Columns, int> runner_tree_item_column_mapping =
  {
    { Columns::ID,               (int)RunnerTreeItem::Columns::ID },
    { Columns::NAME,             (int)RunnerTreeItem::Columns::NAME },
    { Columns::PATH,             (int)RunnerTreeItem::Columns::PATH },
    { Columns::DEFAULT_ALGO_DIR, (int)RunnerTreeItem::Columns::DEFAULT_ALGO_DIR },
    { Columns::COMMAND_ARG,      (int)RunnerTreeItem::Columns::COMMAND_ARG },
  };

};

