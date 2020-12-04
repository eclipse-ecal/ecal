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

#include "group_state_min_task_state_tree_model.h"

#include <list>
#include <memory>

#include "CustomQt/QAbstractTreeItem.h"
#include "tree_item_types.h"

#include "globals.h"

GroupStateMinTaskStateTreeModel::GroupStateMinTaskStateTreeModel(QObject *parent)
  : QAbstractTreeModel(parent)
  , group_state_(nullptr)
{
}


GroupStateMinTaskStateTreeModel::~GroupStateMinTaskStateTreeModel()
{
}

void GroupStateMinTaskStateTreeModel::setGroupState(std::shared_ptr<TaskGroup::GroupState> group_state)
{
  removeAllChildren();
  group_state_ = group_state;

  if (group_state_)
  {
    // Load the new data
    auto min_state_list = group_state_->GetMinimalStatesList();
    QList<QAbstractTreeItem*> min_state_item_list;
    for (auto& min_state : min_state_list)
    {
      GroupStateMinTaskStateTreeItem* min_state_item = new GroupStateMinTaskStateTreeItem(min_state);
      min_state_item_list.push_back(min_state_item);
    }
    insertItems(min_state_item_list);
  }
}

int GroupStateMinTaskStateTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant GroupStateMinTaskStateTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    return columnLabels.at((Columns)section);
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}

int GroupStateMinTaskStateTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::GroupStateMinTaskState:
    return min_task_state_tree_item_column_mapping.at((Columns)(model_column));
  default:
    return -1;
  }
}

bool GroupStateMinTaskStateTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  QAbstractTreeItem* tree_item = item(index);
  if (!tree_item || tree_item->type() != (int)TreeItemType::GroupStateMinTaskState)
  {
    return false;
  }

  int item_column = mapColumnToItem(index.column(), tree_item->type());
  bool success = tree_item->setData(item_column, value, (Qt::ItemDataRole)role);


  if (success)
  {
    // Reflect the changes back to the original Groupstate. Unfortunatelly,
    // the child-items each operate on a copy of the according pair, so we need
    // to set a new list from this model instead of delegating that to the items
    std::list<std::pair<std::shared_ptr<EcalSysTask>, TaskState>> new_min_task_state_list;
    for (int i = 0; i < rowCount(); i++)
    {
      GroupStateMinTaskStateTreeItem* min_task_state_tree_item = static_cast<GroupStateMinTaskStateTreeItem*>(root()->child(i));
      new_min_task_state_list.push_back(min_task_state_tree_item->getMinTaskState());
    }
    group_state_->SetMinimalStatesList(new_min_task_state_list);

    dataChanged(index, index);
  }
  return success;
}

void GroupStateMinTaskStateTreeModel::addMinTaskState(const std::pair<std::shared_ptr<EcalSysTask>, TaskState>& min_state)
{
  GroupStateMinTaskStateTreeItem* min_state_tree_item = new GroupStateMinTaskStateTreeItem(min_state);
  insertItem(min_state_tree_item);
}

void GroupStateMinTaskStateTreeModel::updateTask(std::shared_ptr<EcalSysTask> task)
{
  QList<QAbstractTreeItem*> items = findItems(
    [&task](QAbstractTreeItem* item)
    {
      return ((item->type() == (int)TreeItemType::GroupStateMinTaskState) && static_cast<GroupStateMinTaskStateTreeItem*>(item)->getMinTaskState().first == task);
    }
  );
  for (auto& item : items)
  {
    updateItem(item);
  }
}

void GroupStateMinTaskStateTreeModel::removeTask(std::shared_ptr<EcalSysTask> task)
{
  QList<QAbstractTreeItem*> items = findItems(
    [&task](QAbstractTreeItem* item)
    {
      return ((item->type() == (int)TreeItemType::GroupStateMinTaskState) && static_cast<GroupStateMinTaskStateTreeItem*>(item)->getMinTaskState().first == task);
    }
  );
  for (auto& item : items)
  {
    removeItem(item);
  }
}