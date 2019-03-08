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

#include "CustomQt/QAbstractTreeItem.h"
#include "CustomQt/QAbstractTreeModel.h"

#include <QStringList>

#include <algorithm>

QAbstractTreeModel::QAbstractTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
  root_item_ = new QAbstractTreeItem();
}

QAbstractTreeModel::~QAbstractTreeModel()
{
  delete root_item_;
}

int QAbstractTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  return 1;
}

QVariant QAbstractTreeModel::data(const QModelIndex &index, int role) const
{
  QAbstractTreeItem *tree_item = item(index);
  if (tree_item)
  {
    int item_column = mapColumnToItem(index.column(), tree_item->type());
    return (item_column >= 0 ? tree_item->data(item_column, (Qt::ItemDataRole)role) : QVariant());
  }
  else
  {
    return QVariant();
  }
}

bool QAbstractTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  QAbstractTreeItem* tree_item = item(index);
  if (tree_item)
  {
    int item_column = mapColumnToItem(index.column(), tree_item->type());
    if ((item_column >= 0) && (tree_item->setData(item_column, value, (Qt::ItemDataRole)role)))
    {
      emit dataChanged(index, index, QVector<int>{role});
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

Qt::ItemFlags QAbstractTreeModel::flags(const QModelIndex &index) const
{
  QAbstractTreeItem* tree_item = item(index);
  static const Qt::ItemFlags default_flags = Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  if (tree_item)
  {
    int item_column = mapColumnToItem(index.column(), tree_item->type());
    return (item_column >= 0 ? item(index)->flags(item_column) : default_flags);
  }
  else
  {
    return default_flags;
  }
}

QVariant QAbstractTreeModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
  if (role == Qt::DisplayRole)
  {
    return QVariant(section);
  }
  return QVariant();
}

QModelIndex QAbstractTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  QAbstractTreeItem *parentItem;

  if (!parent.isValid())
    parentItem = root_item_;
  else
    parentItem = static_cast<QAbstractTreeItem*>(parent.internalPointer());

  QAbstractTreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex QAbstractTreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  QAbstractTreeItem *childItem = static_cast<QAbstractTreeItem*>(index.internalPointer());
  QAbstractTreeItem *parentItem = childItem->parentItem();

  if (parentItem == root_item_)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

QAbstractTreeItem* QAbstractTreeModel::item(const QModelIndex& index) const
{
  if (!index.isValid())
    return nullptr;

  return static_cast<QAbstractTreeItem*>(index.internalPointer());
}

int QAbstractTreeModel::rowCount(const QModelIndex &parent) const
{
  QAbstractTreeItem *parentItem;

  if (!parent.isValid())
    parentItem = root_item_;
  else
    parentItem = static_cast<QAbstractTreeItem*>(parent.internalPointer());

  return parentItem->childCount();
}

QAbstractTreeItem* QAbstractTreeModel::root() const
{
  return root_item_;
}

void QAbstractTreeModel::setRoot(QAbstractTreeItem* root)
{
  Q_ASSERT(!(root->parentItem()));

  beginResetModel();
  delete root_item_;
  root_item_ = root;
  endResetModel();
}

void QAbstractTreeModel::insertItems(QList<QAbstractTreeItem*>& items, const QModelIndex &parent, int row)
{
  if (items.size() == 0) return;

  QAbstractTreeItem *parent_item;
  if (!parent.isValid())
    parent_item = root_item_;
  else
    parent_item = static_cast<QAbstractTreeItem*>(parent.internalPointer());

  int start_row = (row >= 0 ? row : parent_item->childCount());
  beginInsertRows(parent, start_row, start_row + items.size() - 1);

  int counter = 0;
  for (auto item : items)
  {
    parent_item->insertChild(item, start_row + counter);
    counter++;
  }

  endInsertRows();
}

void QAbstractTreeModel::removeItems(const QList<QModelIndex>& indices)
{
  // The user might want to remove an Element along with one child. As removing the parent removes all children anyways, we need to filter the list.
  auto filtered_indices = reduceToTopMostIndices(indices);

  for (QModelIndex& index : filtered_indices)
  {
    if (!index.isValid())
      continue;

    QAbstractTreeItem* item_to_remove = static_cast<QAbstractTreeItem*>(index.internalPointer());
    QAbstractTreeItem* parent = item_to_remove->parentItem();
    
    // As it is impossible to remove the root item, we assume to always have a valid item here.

    QModelIndex parent_index;
    if (parent == root_item_)
    {
      parent_index = QModelIndex();
    }
    else
    {
      parent_index = createIndex(parent->row(), 0, parent);
    }

    int row = item_to_remove->row();
    beginRemoveRows(parent_index, row, row);

    parent->removeChild(row); // Removing the child also takes care of deleting it

    endRemoveRows();
  }
}

void QAbstractTreeModel::removeAllChildren(const QModelIndex& index)
{
  QAbstractTreeItem* parent_item = item(index);
  if (!parent_item)
  {
    parent_item = root_item_;
  }

  if (parent_item->childCount() != 0)
  {
    beginRemoveRows(index, 0, parent_item->childCount() - 1);
    parent_item->removeAllChildren(); // Removing the children also takes care of deleting it
    endRemoveRows();
  }
}

void QAbstractTreeModel::moveItem(int from, int to, const QModelIndex& parent)
{
  QAbstractTreeItem* parent_item = item(parent);
  if (!parent_item)
  {
    parent_item = root_item_;
  }
  beginMoveRows(parent, from, from, parent, (from > to /*if move upwards*/ ? to : to + 1)); // As we are in the same parent, we need to add an additional row when moving an item downwards
  parent_item->moveChild(from, to);
  endMoveRows();
}


QList<QModelIndex> QAbstractTreeModel::reduceToTopMostIndices(const QList<QModelIndex>& indices)
{
  QList<QModelIndex> filtered_list;

  for (auto& index : indices)
  {
    QAbstractTreeItem* item = static_cast<QAbstractTreeItem*>(index.internalPointer());
    
    // Walk the tree upwards until we are at the root or encountered an item that also is in the indices list
    QAbstractTreeItem* current_parent = item->parentItem();
    bool index_above_exists = false;
    while (current_parent)
    {
      if (std::find_if(indices.begin(), indices.end(), [current_parent](auto i) {return current_parent == static_cast<QAbstractTreeItem*>(i.internalPointer()); }) != indices.end())
      {
        index_above_exists = true;
        break;
      }
      current_parent = current_parent->parentItem();
    }

    if (!index_above_exists)
    {
      filtered_list.push_back(index);
    }
  }
  return filtered_list;
}

QModelIndex QAbstractTreeModel::index(const QAbstractTreeItem* item, int column) const
{
  if (!(item->parentItem()))
  {
    return QModelIndex();
  }
  return createIndex(item->row(), column, (void*)item);
}

QList<QModelIndex> QAbstractTreeModel::index(const QList<QAbstractTreeItem*>& items, int column) const
{
  QList<QModelIndex> indices;
  for (QAbstractTreeItem* item : items)
  {
    indices.push_back(index(item, column));
  }
  return indices;
}

int QAbstractTreeModel::mapColumnToItem(int model_column, int /*tree_item_type*/) const
{
  return model_column;
}

void QAbstractTreeModel::updateItem(const QAbstractTreeItem* item, const QVector<int>& roles /*= QVector<int>()*/)
{
  QModelIndex top_left = index(item, 0);
  QModelIndex bottom_right = index(item, columnCount() - 1);
  emit dataChanged(top_left, bottom_right, roles);
}

void QAbstractTreeModel::updateChildrenRecursive(const QAbstractTreeItem* parent, const QVector<int>& roles /*= QVector<int>()*/)
{
  if (parent->childCount() != 0) {
    int last_row = parent->childCount() - 1;
    QModelIndex top_left     = index(parent->child(0), 0);
    QModelIndex bottom_right = index(parent->child(last_row), columnCount() - 1);
    emit dataChanged(top_left, bottom_right, roles);

    for (int i = 0; i < parent->childCount(); i++)
    {
      updateChildrenRecursive(parent->child(i), roles);
    }
  }
}

void QAbstractTreeModel::updateAll(const QVector<int>& /*roles*/)
{
  updateChildrenRecursive(root_item_);
}
