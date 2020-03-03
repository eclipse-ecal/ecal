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

#include "topic_tree_model.h"
#include "tree_item_type.h"
#include "item_data_roles.h"

TopicTreeModel::TopicTreeModel(QObject *parent)
  : GroupTreeModel(QVector<int>{}, parent)
{}

TopicTreeModel::~TopicTreeModel()
{}

int TopicTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant TopicTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
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


int TopicTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::Topic:
    return topic_tree_item_column_mapping.at((Columns)(model_column));
  default:
    return GroupTreeModel::mapColumnToItem(model_column, tree_item_type);
  }
}

int TopicTreeModel::groupColumn() const
{
  return (int)(Columns::GROUP);
}

void TopicTreeModel::monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb)
{
  // Create a list of all topics to check if we have to remove them
  std::map<std::string, bool> topic_still_existing;
  for(const auto& topic : topic_tree_item_map_)
  {
    topic_still_existing[topic.first] = false;
  }

  for (const auto& topic : monitoring_pb.topics())
  {
    std::string topic_id = topic.tid();

    if (topic_tree_item_map_.find(topic_id) == topic_tree_item_map_.end())
    {
      // Got a new topic
      TopicTreeItem* topic_tree_item = new TopicTreeItem(topic);
      insertItemIntoGroups(topic_tree_item);
      topic_tree_item_map_[topic_id] = topic_tree_item;
    }
    else
    {
      // Update an existing topic
      topic_tree_item_map_.at(topic_id)->update(topic);
      topic_still_existing[topic_id] = true;
    }
  }

  // Remove obsolete items
  for (const auto& topic : topic_still_existing)
  {
    if (!topic.second)
    {
      removeItemFromGroups(topic_tree_item_map_.at(topic.first));
      topic_tree_item_map_.erase(topic.first);
    }
  }

  updateAll();
}

QVector<QPair<int, QString>> TopicTreeModel::getTreeItemColumnNameMapping() const
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
