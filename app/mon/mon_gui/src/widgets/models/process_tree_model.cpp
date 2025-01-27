/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "process_tree_model.h"
#include "tree_item_type.h"
#include "item_data_roles.h"

ProcessTreeModel::ProcessTreeModel(QObject *parent)
  : GroupTreeModel(QVector<int>{}, parent)
{}

ProcessTreeModel::~ProcessTreeModel()
{}

int ProcessTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant ProcessTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    if (section == groupColumn())
    {
      return groupColumnHeader();
    }
    else
    {
      return columnLabels.at((Columns)section);
    }
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}


int ProcessTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::Process:
    return column_mapping.at((Columns)(model_column));
  default:
    return GroupTreeModel::mapColumnToItem(model_column, tree_item_type);
  }
}

int ProcessTreeModel::groupColumn() const
{
  return (int)(Columns::GROUP);
}

void ProcessTreeModel::monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb)
{
  // Create a list of all processes to check if we have to remove them
  std::map<std::string, bool> process_still_existing;
  for(const auto& process : tree_item_map_)
  {
    process_still_existing[process.first] = false;
  }

  for (const auto& process_pb : monitoring_pb.processes())
  {
    // Create a Process ID that is unique across all hosts
    std::string process_identifier = (std::to_string(process_pb.process_id()) + "@" + process_pb.host_name());

    if (tree_item_map_.find(process_identifier) == tree_item_map_.end())
    {
      // Got a new process
      ProcessTreeItem* process_tree_item = new ProcessTreeItem(process_pb);
      insertItemIntoGroups(process_tree_item);
      tree_item_map_[process_identifier] = process_tree_item;
    }
    else
    {
      // Update an existing process
      tree_item_map_.at(process_identifier)->update(process_pb);
      process_still_existing[process_identifier] = true;
    }
  }

  // Remove obsolete items
  for (const auto& process : process_still_existing)
  {
    if (!process.second)
    {
      removeItemFromGroups(tree_item_map_.at(process.first));
      tree_item_map_.erase(process.first);
    }
  }

  updateAll();
}

QVector<QPair<int, QString>> ProcessTreeModel::getTreeItemColumnNameMapping() const
{
  QVector<QPair<int, QString>> column_name_mapping;

  for (int i = 0; i < columnCount(); i++)
  {
    int column = mapColumnToItem(i, (int)TreeItemType::Topic);
    if (column >= 0)
    {
      QString name = headerData(i, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString();
      column_name_mapping.push_back(QPair<int, QString>(column, name));
    }
  }

  return column_name_mapping;
}
