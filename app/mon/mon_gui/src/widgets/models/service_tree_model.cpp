/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#include "service_tree_model.h"
#include "tree_item_type.h"
#include "item_data_roles.h"

ServiceTreeModel::ServiceTreeModel(QObject *parent)
  : GroupTreeModel(QVector<int>{}, parent)
{}

ServiceTreeModel::~ServiceTreeModel()
{}

int ServiceTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant ServiceTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
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


int ServiceTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::Service:
    return tree_item_column_mapping.at((Columns)(model_column));
  default:
    return GroupTreeModel::mapColumnToItem(model_column, tree_item_type);
  }
}

int ServiceTreeModel::groupColumn() const
{
  return (int)(Columns::GROUP);
}

void ServiceTreeModel::monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb)
{
  // Create a list of all service methods to check if we have to remove them
  std::map<std::string, bool> service_still_existing;
  for(const auto& service_tree_item : tree_item_map_)
  {
    service_still_existing[service_tree_item.first] = false;
  }

  for (const auto& service : monitoring_pb.services())
  {
    for (const auto& method : service.methods())
    {
      std::string service_identifier = ServiceTreeItem::generateIdentifier(service, method);

      if (tree_item_map_.find(service_identifier) == tree_item_map_.end())
      {
        // Got a new service-method
        ServiceTreeItem* service_tree_item = new ServiceTreeItem(service, method);
        insertItemIntoGroups(service_tree_item);
        tree_item_map_[service_identifier] = service_tree_item;
      }
      else
      {
        // Update an existing service-method
        tree_item_map_.at(service_identifier)->update(service, method);
        service_still_existing[service_identifier] = true;
      }
    }
  }

  // Remove obsolete items
  for (const auto& service_tree_item : service_still_existing)
  {
    if (!service_tree_item.second)
    {
      removeItemFromGroups(tree_item_map_.at(service_tree_item.first));
      tree_item_map_.erase(service_tree_item.first);
    }
  }

  updateAll();
}

QVector<QPair<int, QString>> ServiceTreeModel::getTreeItemColumnNameMapping() const
{
  QVector<QPair<int, QString>> column_name_mapping;

  for (int i = 0; i < columnCount(); i++)
  {
    int column = mapColumnToItem(i, (int)TreeItemType::Service);
    if (column >= 0)
    {
      QString name = headerData(i, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString();
      column_name_mapping.push_back(QPair<int, QString>(column, name));
    }
  }

  return column_name_mapping;
}
