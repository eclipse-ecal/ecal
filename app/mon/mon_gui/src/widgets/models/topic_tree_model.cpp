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

#include <QFont>
#include <QTimer>

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
  for (const auto& topic : topic_tree_item_map_)
  {
    topic_still_existing[topic.first] = false;
  }

  for (const auto& topic : monitoring_pb.topics())
  {
    std::string topic_id = topic.topic_id();

    if (topic_tree_item_map_.find(topic_id) == topic_tree_item_map_.end())
    {
      // Got a new topic
      TopicTreeItem* topic_tree_item = new TopicTreeItem(topic);
      insertItemIntoGroups(topic_tree_item);
      STopicTreeEntry tree_entry;
      tree_entry.tree_item = topic_tree_item;
      topic_tree_item_map_[topic_id] = tree_entry;
      auto fontrole = topic_tree_item->data(1, Qt::ItemDataRole::FontRole);
      auto font = qvariant_cast<QFont>(fontrole);
      font.setBold(true);
      topic_tree_item->setFont(font);

      QTimer::singleShot(10000, [this, topic_id]()
      {
        if(topic_tree_item_map_.find(topic_id) != topic_tree_item_map_.end())
        {
          topic_tree_item_map_[topic_id].new_topic_timer = true;
        }
      });
    }
    else
    {
      auto& tree_entry = topic_tree_item_map_.at(topic_id);
      auto *topic_tree_item = tree_entry.tree_item;

      if (tree_entry.striked_out)
      {
        auto fontrole = topic_tree_item->data(1, Qt::ItemDataRole::FontRole);
        auto font = qvariant_cast<QFont>(fontrole);
        font.setStrikeOut(false);
        font.setBold(false);
        topic_tree_item->setFont(font);
        tree_entry.striked_out = false;
        tree_entry.default_font = true;
      }

      if(tree_entry.new_topic_timer && !tree_entry.default_font)
      {
        auto fontrole = topic_tree_item->data(1, Qt::ItemDataRole::FontRole);
        auto font = qvariant_cast<QFont>(fontrole);
        font.setBold(false);
        topic_tree_item->setFont(font);
        tree_entry.default_font = true;
      }

      // Update an existing topic
      tree_entry.topic_removed = false;
      tree_entry.deleted_topic_timer = false;
      topic_tree_item->update(topic);
      topic_still_existing[topic_id] = true;
    }
  }

  // Remove obsolete items
  for (const auto& topic : topic_still_existing)
  {
    if (!topic.second)
    {
      auto& tree_entry = topic_tree_item_map_.at(topic.first);
      TopicTreeItem* topic_tree_item = tree_entry.tree_item;

      if (!tree_entry.striked_out)
        {
          auto fontrole = topic_tree_item->data(1, Qt::ItemDataRole::FontRole);
          auto font = qvariant_cast<QFont>(fontrole);
          font.setStrikeOut(true);
          topic_tree_item->setFont(font);
          tree_entry.striked_out = true;
          tree_entry.topic_removed = true;
        }

      if (!tree_entry.deleted_topic_timer)
      {
        tree_entry.deleted_topic_timer = true;
        QTimer::singleShot(10000, [this, topic_tree_item, topic]()
        {
          if (topic_tree_item_map_.find(topic.first) != topic_tree_item_map_.end() && topic_tree_item_map_[topic.first].topic_removed)
          {
            removeItemFromGroups(topic_tree_item);
            topic_tree_item_map_.erase(topic.first);
          }
        });
      }
    }
  }

  updateAll();
}


QVector<QPair<int, QVariant>> TopicTreeModel::getTreeItemColumnNameMapping() const
{
  QVector<QPair<int, QVariant>> column_name_mapping;

  for (int i = 0; i < columnCount(); i++)
  {
    int column = mapColumnToItem(i, (int)TreeItemType::Topic);
    if (column >= 0)
    {
      const QVariant name = headerData(i, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole);
      column_name_mapping.push_back(QPair<int, QVariant>(column, name));
    }
  }

  return column_name_mapping;
}
