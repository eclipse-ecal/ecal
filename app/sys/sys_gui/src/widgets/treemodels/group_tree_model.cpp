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

#include "group_tree_model.h"

#include <algorithm>

#include "CustomQt/QAbstractTreeItem.h"
#include "tree_item_types.h"

#include "group_tree_item.h"
#include "task_tree_item.h"
#include "globals.h"

GroupTreeModel::GroupTreeModel(QObject *parent)
  :QAbstractTreeModel(parent)
{
}

void GroupTreeModel::reload()
{
  reload(Globals::EcalSysInstance()->GetGroupList());
}

void GroupTreeModel::reload(const std::list<std::shared_ptr<TaskGroup>>& group_list)
{
  removeAllChildren();

  QList<QAbstractTreeItem*> group_item_list;
  for (auto& group : group_list)
  {
    GroupTreeItem* group_item = new GroupTreeItem(group);

    auto task_list = group->GetAllTasks();
    for (auto& task : task_list)
    {
      TaskTreeItem* task_item = new TaskTreeItem(task);
      group_item->insertChild(task_item);
    }
    group_item_list.push_back(group_item);
  }

  insertItems(group_item_list);
}

GroupTreeModel::~GroupTreeModel()
{
}

int GroupTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant GroupTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    return columnLabels.at((Columns)section);
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}


int GroupTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::Task:
    return task_tree_item_column_mapping.at((Columns)(model_column));
  case TreeItemType::Group:
    return group_tree_item_column_mapping.at((Columns)(model_column));
  default:
    return -1;
  }
}

QModelIndex GroupTreeModel::index(std::shared_ptr<TaskGroup> group, int column) const
{
  QList<QAbstractTreeItem*> group_items = findItems(
    [&group](QAbstractTreeItem* item)
    {
      return ((item->type() == (int)TreeItemType::Group) && static_cast<GroupTreeItem*>(item)->getGroup() == group);
    }
  );
  if (group_items.size() != 0)
  {
    return index(group_items.first(), column);
  }
  else
  {
    return QModelIndex();
  }
}

void GroupTreeModel::updateTask(std::shared_ptr<EcalSysTask> task)
{
  QList<QAbstractTreeItem*> items = findItems(
    [&task](QAbstractTreeItem* item)
    {
      return ((item->type() == (int)TreeItemType::Task) && static_cast<TaskTreeItem*>(item)->getTask() == task);
    }
  );
  for (auto& item : items)
  {
    updateItem(item);
  }
}

void GroupTreeModel::addGroup(std::shared_ptr<TaskGroup> group)
{
  GroupTreeItem* group_item = new GroupTreeItem(group);

  auto task_list = group->GetAllTasks();
  for (auto& task : task_list)
  {
    TaskTreeItem* task_item = new TaskTreeItem(task);
    group_item->insertChild(task_item);
  }
  insertItem(group_item);
}

void GroupTreeModel::removeGroup(std::shared_ptr<TaskGroup> group)
{
  QList<QAbstractTreeItem*> group_item_list = findItems(
    [&group](QAbstractTreeItem* item)
    {
      return ((item->type() == (int)TreeItemType::Group) && static_cast<GroupTreeItem*>(item)->getGroup() == group);
    }
  );
  for (auto& group_item : group_item_list)
  {
    removeItem(group_item);
  }
}

void GroupTreeModel::updateGroup(std::shared_ptr<TaskGroup> group)
{
  QList<QAbstractTreeItem*> group_item_list = findItems(
    [&group](QAbstractTreeItem* item)
    {
      return ((item->type() == (int)TreeItemType::Group) && static_cast<GroupTreeItem*>(item)->getGroup() == group);
    }
  );
  for (auto& group_item : group_item_list)
  {
    // Update the group-item itself
    updateItem(group_item);

    // check if any tasks have been added or removed
    auto new_tasks = static_cast<GroupTreeItem*>(group_item)->getGroup()->GetAllTasks();
    QList<QAbstractTreeItem*> missing_tasks;
    for (int i = 0; i < group_item->childCount(); i++)
    {
      TaskTreeItem* task_tree_item = static_cast<TaskTreeItem*>(group_item->child(i));
      auto found_iterator = std::find(new_tasks.begin(), new_tasks.end(), task_tree_item->getTask());
      if (found_iterator == new_tasks.end())
      {
        // The task is missing => keep a list of all missing tasks
        missing_tasks.push_back(task_tree_item);
      }
      else
      {
        // The task is not missing => remove it from the list of all tasks, so we get a list of new tasks afterwards
        new_tasks.erase(found_iterator);
      }
    }

    for (auto& task : new_tasks)
    {
      TaskTreeItem* new_task_item = new TaskTreeItem(task);
      insertItem(new_task_item, index(group_item));
    }

    removeItems(missing_tasks);
  }
}