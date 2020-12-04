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

#include "group_state_tree_item.h"

class GroupStateTreeModel :
  public QAbstractTreeModel
{
  Q_OBJECT

public:
  enum class Columns : int
  {
    NAME,

    COLUMN_COUNT
  };

  GroupStateTreeModel(QObject *parent = nullptr);
  ~GroupStateTreeModel();

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;

public slots:
  void setGroup(std::shared_ptr<TaskGroup> group);

  void updateGroupState(std::shared_ptr<TaskGroup::GroupState> group_state);

private:
  std::map<Columns, QString> columnLabels = {
    { Columns::NAME, tr("States") },
  };

  std::map<Columns, int> group_state_tree_item_column_mapping =
  {
    { Columns::NAME, (int)GroupStateTreeItem::Columns::NAME },
  };

  std::shared_ptr<TaskGroup> group_;
};

