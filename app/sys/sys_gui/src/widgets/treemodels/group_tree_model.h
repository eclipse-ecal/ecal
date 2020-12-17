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

#include <memory>
#include "ecalsys/task/ecal_sys_task.h"
#include "ecalsys/task/task_group.h"

#include "CustomQt/QAbstractTreeModel.h"

#include "task_tree_item.h"
#include "group_tree_item.h"

class GroupTreeModel :
  public QAbstractTreeModel
{
  Q_OBJECT

public:

  enum class Columns : int
  {
    NAME,
    TARGET_NAME,
    STATE,
    INFO,
    COLUMN_COUNT
  };


  GroupTreeModel(QObject *parent = nullptr);
  ~GroupTreeModel();

  void reload();

  void reload(const std::list<std::shared_ptr<TaskGroup>>& group_list);

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  using QAbstractTreeModel::index;
  QModelIndex index(std::shared_ptr<TaskGroup> group, int column = 0) const;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;

public slots:
  void addGroup(std::shared_ptr<TaskGroup> group);
  void removeGroup(std::shared_ptr<TaskGroup> group);
  void updateGroup(std::shared_ptr<TaskGroup> group);

  void updateTask(std::shared_ptr<EcalSysTask> task);

private:
  std::map<Columns, QString> columnLabels =
  { 
    { Columns::NAME,        "Name"} ,
    { Columns::TARGET_NAME, "Host" } ,
    { Columns::STATE,       "State" } ,
    { Columns::INFO,        "Info" }
  };

  std::map<Columns, int> task_tree_item_column_mapping =
  {
    { Columns::NAME,        (int)TaskTreeItem::Columns::TASK_NAME },
    { Columns::TARGET_NAME, (int)TaskTreeItem::Columns::TARGET_NAME },
    { Columns::STATE,       (int)TaskTreeItem::Columns::STATE },
    { Columns::INFO,        (int)TaskTreeItem::Columns::INFO }
  };

  std::map<Columns, int> group_tree_item_column_mapping =
  {
    { Columns::NAME,        (int)GroupTreeItem::Columns::NAME },
    { Columns::TARGET_NAME, -1 },
    { Columns::STATE,       (int)GroupTreeItem::Columns::STATE },
    { Columns::INFO,        -1 }
  };
};

