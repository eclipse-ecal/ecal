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

#include "import_from_cloud_runner_tree_model.h"

#include "CustomQt/QAbstractTreeItem.h"
#include "tree_item_types.h"

#include "globals.h"

ImportFromCloudRunnerTreeModel::ImportFromCloudRunnerTreeModel(QObject *parent)
  :QAbstractTreeModel(parent)
{
}


ImportFromCloudRunnerTreeModel::~ImportFromCloudRunnerTreeModel()
{
}

void ImportFromCloudRunnerTreeModel::reload(bool add_empty_runner)
{
  reload(Globals::EcalSysInstance()->GetRunnerList(), add_empty_runner);
}

void ImportFromCloudRunnerTreeModel::reload(const std::list<std::shared_ptr<EcalSysRunner>>& runner_list, bool add_empty_runner)
{
  removeAllChildren();

  // Load the new data
  QList<QAbstractTreeItem*> runner_item_list;

  if (add_empty_runner)
  {
    // Add empty runner at first place
    runner_item_list.push_back(new RunnerTreeItem(std::shared_ptr<EcalSysRunner>(nullptr)));
  }

  for (auto& runner : runner_list)
  {
    RunnerTreeItem* runner_item = new RunnerTreeItem(runner);
    runner_item_list.push_back(runner_item);
  }
  insertItems(runner_item_list);
}


int ImportFromCloudRunnerTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant ImportFromCloudRunnerTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    return columnLabels.at((Columns)section);
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}

int ImportFromCloudRunnerTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::Runner:
    return runner_tree_item_column_mapping.at((Columns)(model_column));
  default:
    return -1;
  }
}

int ImportFromCloudRunnerTreeModel::rowOf(std::shared_ptr<EcalSysRunner> runner)
{
  QList<QAbstractTreeItem*> items = findItems(
    [&runner](QAbstractTreeItem* runner_tree_item)
  {
    return ((runner_tree_item->type() == (int)TreeItemType::Runner) && static_cast<RunnerTreeItem*>(runner_tree_item)->getRunner() == runner);
  }
  );
  if (!items.empty())
  {
    return items.first()->row();
  }
  else
  {
    return -1;
  }
}

std::shared_ptr<EcalSysRunner> ImportFromCloudRunnerTreeModel::getRunnerByRow(int row)
{
  QAbstractTreeItem* tree_item = root()->child(row);
  return static_cast<RunnerTreeItem*>(tree_item)->getRunner();
}

void ImportFromCloudRunnerTreeModel::updateRunner(std::shared_ptr<EcalSysRunner> runner)
{
  QList<QAbstractTreeItem*> items = findItems(
    [&runner](QAbstractTreeItem* runner_tree_item)
  {
    return ((runner_tree_item->type() == (int)TreeItemType::Runner) && static_cast<RunnerTreeItem*>(runner_tree_item)->getRunner() == runner);
  }
  );
  for (auto& runner_tree_item : items)
  {
    updateItem(runner_tree_item);
  }
}

void ImportFromCloudRunnerTreeModel::addRunner(std::shared_ptr<EcalSysRunner> new_runner)
{
  RunnerTreeItem* runner_item = new RunnerTreeItem(new_runner);
  insertItem(runner_item);
}

void ImportFromCloudRunnerTreeModel::removeRunner(std::shared_ptr<EcalSysRunner> runner)
{
  QList<QAbstractTreeItem*> items = findItems(
    [&runner](QAbstractTreeItem* runner_tree_item)
  {
    return ((runner_tree_item->type() == (int)TreeItemType::Runner) && static_cast<RunnerTreeItem*>(runner_tree_item)->getRunner() == runner);
  }
  );
  removeItems(items);
}
