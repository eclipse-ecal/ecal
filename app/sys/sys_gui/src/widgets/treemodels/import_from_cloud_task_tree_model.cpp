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

#include "import_from_cloud_task_tree_model.h"

#include "CustomQt/QAbstractTreeItem.h"
#include "tree_item_types.h"

#include "globals.h"

ImportFromCloudTaskTreeModel::ImportFromCloudTaskTreeModel(QObject *parent)
  :QAbstractTreeModel(parent)
{
}


ImportFromCloudTaskTreeModel::~ImportFromCloudTaskTreeModel()
{
}

void ImportFromCloudTaskTreeModel::reload()
{
  reload(Globals::EcalSysInstance()->GetTaskList());
}

void ImportFromCloudTaskTreeModel::reload(const std::list<std::shared_ptr<EcalSysTask>>& task_list)
{
  removeAllChildren();

  // Load the new data
  QList<QAbstractTreeItem*> task_item_list;
  for (auto& task : task_list)
  {
    TaskTreeItem* task_item = new TaskTreeItem(task);
    task_item_list.push_back(task_item);
  }
  insertItems(task_item_list);
}


int ImportFromCloudTaskTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant ImportFromCloudTaskTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    return columnLabels.at((Columns)section);
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}

void ImportFromCloudTaskTreeModel::setHeaderLabel(Columns column, const QString& label)
{
  columnLabels[column] = label;
  emit headerDataChanged(Qt::Orientation::Horizontal, (int) column, (int) column);
}

int ImportFromCloudTaskTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::Task:
    return task_tree_item_column_mapping.at((Columns)(model_column));
  default:
    return -1;
  }
}

QModelIndex ImportFromCloudTaskTreeModel::index(const std::shared_ptr<EcalSysTask>& task, int column)
{
  QList<QAbstractTreeItem*> task_items = findItems(
    [&task](QAbstractTreeItem* item)
    {
      return ((item->type() == (int)TreeItemType::Task) && static_cast<TaskTreeItem*>(item)->getTask() == task);
    }
  );
  if (task_items.size() > 0)
  {
    return index(task_items.first(), column);
  }
  else
  {
    return QModelIndex();
  }
}

void ImportFromCloudTaskTreeModel::updateTask(std::shared_ptr<EcalSysTask> task)
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

void ImportFromCloudTaskTreeModel::addTask(std::shared_ptr<EcalSysTask> new_task)
{
  TaskTreeItem* task_item = new TaskTreeItem(new_task);
  insertItem(task_item);
}

void ImportFromCloudTaskTreeModel::removeTask(std::shared_ptr<EcalSysTask> task)
{
  QList<QAbstractTreeItem*> items = findItems(
    [&task](QAbstractTreeItem* item)
    {
      return ((item->type() == (int)TreeItemType::Task) && static_cast<TaskTreeItem*>(item)->getTask() == task);
    }
  );
  for (auto& item : items)
  {
    removeItem(item);
  }
}