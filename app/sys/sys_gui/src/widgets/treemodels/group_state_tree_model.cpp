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

#include "group_state_tree_model.h"

#include "CustomQt/QAbstractTreeItem.h"
#include "tree_item_types.h"

#include "globals.h"

GroupStateTreeModel::GroupStateTreeModel(QObject *parent)
  :QAbstractTreeModel(parent)
{
  group_ = std::shared_ptr<TaskGroup>(nullptr);
}


GroupStateTreeModel::~GroupStateTreeModel()
{
}

void GroupStateTreeModel::setGroup(std::shared_ptr<TaskGroup> group)
{
  if ((group && !group_) || (!group && group_) || !(group == group_))
  {
    group_ = group;

    removeAllChildren();

    if (group)
    {
      // Load the new data
      auto state_list = group->GetGroupStateList();
      QList<QAbstractTreeItem*> task_item_list;
      for (auto& state : state_list)
      {
        GroupStateTreeItem* state_item = new GroupStateTreeItem(state);
        task_item_list.push_back(state_item);
      }
      insertItems(task_item_list);
    }
  }
}

int GroupStateTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant GroupStateTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    return columnLabels.at((Columns)section);
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}

int GroupStateTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::GroupState:
    return group_state_tree_item_column_mapping.at((Columns)(model_column));
  default:
    return -1;
  }
}

void GroupStateTreeModel::updateGroupState(std::shared_ptr<TaskGroup::GroupState> group_state)
{
  QList<QAbstractTreeItem*> items = findItems(
    [&group_state](QAbstractTreeItem* group_state_tree_item)
    {
      return ((group_state_tree_item->type() == (int)TreeItemType::GroupState) && static_cast<GroupStateTreeItem*>(group_state_tree_item)->getGroupState() == group_state);
    }
  );
  for (auto& group_state_tree_item : items)
  {
    updateItem(group_state_tree_item);
  }
}