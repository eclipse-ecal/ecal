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

#include "host_tree_model.h"
#include "tree_item_type.h"
#include "item_data_roles.h"

HostTreeModel::HostTreeModel(QObject *parent)
  : GroupTreeModel(QVector<int>{}, parent)
{}

HostTreeModel::~HostTreeModel()
{}

int HostTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant HostTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    if (section == groupColumn())
    {
      return groupColumnHeader();
    }
    else
    {
      return column_labels_.at((Columns)section);
    }
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}


int HostTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::Host:
    return item_column_mapping_.at((Columns)(model_column));
  default:
    return GroupTreeModel::mapColumnToItem(model_column, tree_item_type);
  }
}

int HostTreeModel::groupColumn() const
{
  return (int)(Columns::GROUP);
}

void HostTreeModel::monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb)
{
  // Create a list of all hosts to check if we have to remove them
  std::map<std::string, bool> host_still_existing;
  for(const auto& host : tree_item_map_)
  {
    host_still_existing[host.first] = false;
  }

  for (const auto& process : monitoring_pb.processes())
  {
    std::string host_name = process.host_name();

    if (tree_item_map_.find(host_name) == tree_item_map_.end())
    {
      // Got a new host
      HostTreeItem* host_tree_item = new HostTreeItem(host_name.c_str());
      insertItemIntoGroups(host_tree_item);
      tree_item_map_[host_name] = host_tree_item;
    }
    else
    {
      // Update an existing host
      tree_item_map_.at(host_name)->update(monitoring_pb);
      host_still_existing[host_name] = true;
    }
  }

  // Remove obsolete items
  for (const auto& host : host_still_existing)
  {
    if (!host.second)
    {
      removeItemFromGroups(tree_item_map_.at(host.first));
      tree_item_map_.erase(host.first);
    }
  }

  updateAll();
}

QVector<QPair<int, QString>> HostTreeModel::getTreeItemColumnNameMapping() const
{
  QVector<QPair<int, QString>> column_name_mapping;

  for (int i = 0; i < columnCount(); i++)
  {
    int column = mapColumnToItem(i, (int)TreeItemType::Host);
    if (column >= 0)
    {
      QString name = headerData(i, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString();
      column_name_mapping.push_back(QPair<int, QString>(column, name));
    }
  }

  return column_name_mapping;
}
