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

#include "channel_tree_model.h"

#include "tree_item_type.h"
#include "item_data_roles.h"

#include "q_ecal_play.h"

#include <QCheckBox>

ChannelTreeModel::ChannelTreeModel(QObject *parent)
  : QAbstractTreeModel(parent)
  , edit_enabled_(true)
{}

ChannelTreeModel::~ChannelTreeModel()
{}

int ChannelTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant ChannelTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    return columnLabels.at((Columns)section);
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}

QVariant ChannelTreeModel::data(const QModelIndex &index, int role) const
{
  QAbstractTreeItem* tree_item = item(index);
  if (!tree_item || (tree_item->type() != (int)TreeItemType::Channel))
  {
    return QAbstractTreeModel::data(index, role);
  }

  ChannelTreeItem* channel_item = static_cast<ChannelTreeItem*>(tree_item);
  int column = index.column();

  if (role == Qt::ItemDataRole::DisplayRole)
  {
    if (column == (int)Columns::CHANNEL_NAME)
    {
      QString source_name = channel_item->data((int)ChannelTreeItem::Columns::SOURCE_CHANNEL_NAME, (Qt::ItemDataRole)role).toString();
      QString target_name = channel_item->data((int)ChannelTreeItem::Columns::TARGET_CHANNEL_NAME, (Qt::ItemDataRole)role).toString();

      if (source_name == target_name)
      {
        return target_name;
      }
      else
      {
        return target_name + " (" + source_name + ")";
      }
    }
    else if (column == (int)Columns::MESSAGE_COUNTER)
    {
      auto counter_it = message_counters_.find(channel_item->sourceChannelName().toStdString());
      if (counter_it != message_counters_.end())
      {
        return counter_it->second;
      }
      else
      {
        return 0LL;
      }
    }
  }
  else if (role == Qt::ItemDataRole::EditRole)
  {
    if (column == (int)Columns::CHANNEL_NAME)
    {
     return channel_item->data((int)ChannelTreeItem::Columns::TARGET_CHANNEL_NAME, (Qt::ItemDataRole)role).toString();
    }
  }
  else if (role == Qt::ItemDataRole::CheckStateRole)
  {
    if (column == (int)Columns::ENABLED)
    {
      return channel_item->data((int)ChannelTreeItem::Columns::ENABLED, (Qt::ItemDataRole)role);
    }
  }
  else if (role == Qt::ItemDataRole::TextAlignmentRole)
  {
    if (column == (int)Columns::MESSAGE_COUNTER)
    {
      return Qt::AlignmentFlag::AlignRight;
    }
  }
  else if (role == ItemDataRoles::FilterRole)
  {
    return data(index, Qt::ItemDataRole::DisplayRole);
  }
  else if (role == ItemDataRoles::SortRole)
  {
    if ((column == (int)Columns::CHANNEL_NAME)
      || (column == (int)Columns::MESSAGE_COUNTER))
    {
      return data(index, Qt::ItemDataRole::DisplayRole);
    }
  }

  return QAbstractTreeModel::data(index, role);
}

bool ChannelTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  QAbstractTreeItem* tree_item = item(index);
  if (!tree_item || (tree_item->type() != (int)TreeItemType::Channel))
  {
    bool success = QAbstractTreeModel::setData(index, value, role);
    if (success)
      emit dataChanged(index, index, QVector<int>{role});
    return success;
  }

  ChannelTreeItem* channel_item = static_cast<ChannelTreeItem*>(tree_item);
  int column = index.column();

  if (role == Qt::ItemDataRole::EditRole)
  {
    if (column == (int)Columns::CHANNEL_NAME)
    {
      bool success = channel_item->setData((int)ChannelTreeItem::Columns::TARGET_CHANNEL_NAME, value, (Qt::ItemDataRole)role);
      if (success)
        emit dataChanged(index, index, QVector<int>{role});
      return success;
    }
  }
  else if (role == Qt::ItemDataRole::CheckStateRole)
  {
    if (column == (int)Columns::CHANNEL_NAME)
    {
      bool success = channel_item->setData((int)ChannelTreeItem::Columns::ENABLED, value, (Qt::ItemDataRole)role);
      if (success)
        emit dataChanged(index, index, QVector<int>{role});
      return success;
    }
  }

  bool success = QAbstractTreeModel::setData(index, value, role);
  if (success)
    emit dataChanged(index, index, QVector<int>{role});
  return success;
}

Qt::ItemFlags ChannelTreeModel::flags(const QModelIndex &index) const
{
  int column = index.column();

  if (column == (int)Columns::ENABLED)
  {
    if (editEnabled())
    {
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsUserCheckable;
    }
    else
    {
      return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsUserCheckable;
    }
  }
  if (column == (int)Columns::CHANNEL_NAME)
  {
    if (editEnabled())
    {
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;
    }
    else
    {
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }
  }

  return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;

}

int ChannelTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::Channel:
    return task_tree_item_column_mapping.at((Columns)(model_column));
  default:
    return -1;
  }
}

void ChannelTreeModel::reload()
{
  removeAllChildren();
  message_counters_.clear();

  auto channel_name_set = QEcalPlay::instance()->channelNames();

  auto continuity_report = QEcalPlay::instance()->createContinuityReport();

  QList<QAbstractTreeItem*> new_channel_list;
  for (auto& name : channel_name_set)
  {
    long long expected_frames = -1;
    long long existing_frames = 0;

    auto continuity_it = continuity_report.find(name);
    if (continuity_it != continuity_report.end())
    {
      expected_frames = continuity_it->second.expected_frame_count_;
      existing_frames = continuity_it->second.existing_frame_count_;
    }

    ChannelTreeItem* channel_item = new ChannelTreeItem(name.c_str(), expected_frames, existing_frames);
    new_channel_list.push_back(channel_item);
  }
  insertItems(new_channel_list);

  updateMessageCounters();
}

void ChannelTreeModel::updateMessageCounters()
{
  message_counters_ = QEcalPlay::instance()->messageCounters();

  if (rowCount() > 0)
  {
    emit dataChanged(index(root()->child(0), (int)Columns::MESSAGE_COUNTER), index(root()->child(root()->childCount() - 1), (int)Columns::MESSAGE_COUNTER));
  }
}


void ChannelTreeModel::setAllChecked(bool checked)
{
  if (root()->childCount() < 1)
    return;

  for (int i = 0; i < root()->childCount(); i++)
  {
    QAbstractTreeItem* tree_item = root()->child(i);
    if (tree_item->type() == (int)TreeItemType::Channel)
    {
      static_cast<ChannelTreeItem*>(tree_item)->setData((int)ChannelTreeItem::Columns::ENABLED, (checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked), Qt::ItemDataRole::CheckStateRole);
    }
  }
  emit dataChanged(index(root()->child(0), (int)Columns::CHANNEL_NAME), index(root()->child(root()->childCount() - 1), (int)Columns::CHANNEL_NAME), QVector<int>{Qt::ItemDataRole::CheckStateRole});
}

void ChannelTreeModel::invertCheckedState()
{
  if (root()->childCount() < 1)
    return;

  for (int i = 0; i < root()->childCount(); i++)
  {
    QAbstractTreeItem* tree_item = root()->child(i);
    if (tree_item->type() == (int)TreeItemType::Channel)
    {
      ChannelTreeItem* channel_item = static_cast<ChannelTreeItem*>(tree_item);
      channel_item->setEnabled(!channel_item->enabled());
    }
  }
  emit dataChanged(index(root()->child(0), (int)Columns::CHANNEL_NAME), index(root()->child(root()->childCount() - 1), (int)Columns::CHANNEL_NAME), QVector<int>{Qt::ItemDataRole::CheckStateRole});
}

void ChannelTreeModel::setEditEnabled(bool enabled)
{
  edit_enabled_ = enabled;
  if (rowCount() > 0)
  {
    emit dataChanged(index(root()->child(0), (int)Columns::ENABLED), index(root()->child(root()->childCount() - 1), (int)Columns::ENABLED));
  }
}

bool ChannelTreeModel::editEnabled() const
{
  return edit_enabled_;
}