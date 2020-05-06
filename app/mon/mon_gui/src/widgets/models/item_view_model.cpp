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

#include "item_view_model.h"

ItemViewModel::ItemViewModel(QAbstractTreeItem* tree_item, const QVector<QPair<int, QString>>& column_name_map, QObject *parent)
  : QAbstractItemModel(parent)
  , tree_item_(tree_item)
  , column_name_map_(column_name_map)
{}

ItemViewModel::ItemViewModel(const QVector<QPair<int, QString>>& column_name_map, QObject *parent)
  : ItemViewModel(nullptr, column_name_map, parent)
{}

ItemViewModel::~ItemViewModel()
{
  delete tree_item_;
}

QVariant ItemViewModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  if (role == Qt::ItemDataRole::TextAlignmentRole)
  {
    return Qt::AlignmentFlag::AlignLeft;
  }

  if (index.column() == 0)
  {
    // Key Column
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return column_name_map_[index.row()].second;
    }
  }
  else
  {
    // Value Column
    if (tree_item_)
    {
      return tree_item_->data(column_name_map_[index.row()].first, (Qt::ItemDataRole)role);
    }
    else
    {
      return QVariant();
    }
  }

  return QVariant();
}

QVariant ItemViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Orientation::Horizontal) && role == Qt::DisplayRole)
  {
    return (section == 0) ? "Key" : "Value";
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex ItemViewModel::index(int row, int column, const QModelIndex &/*parent*/) const
{
  return createIndex(row, column, nullptr);
}

QModelIndex ItemViewModel::parent(const QModelIndex &/*index*/) const 
{
  return QModelIndex();
}

int ItemViewModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
  {
    return column_name_map_.size();
  }
  else
  {
    return 0;
  }
}

int ItemViewModel::columnCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
  {
    return 2;
  }
  else
  {
    return 0;
  }
}

void ItemViewModel::setTreeItem(QAbstractTreeItem* tree_item)
{
  beginResetModel();
  delete tree_item_;
  tree_item_ = tree_item;
  endResetModel();
}

QAbstractTreeItem* ItemViewModel::treeItem() const
{
  return tree_item_;
}

void ItemViewModel::update()
{
  dataChanged(createIndex(0, 1, nullptr), createIndex(rowCount() - 1, 1, nullptr));
}