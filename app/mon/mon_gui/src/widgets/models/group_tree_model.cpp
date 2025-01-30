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

//#ifdef _MSC_VER
//// Disable Qt 5.15 Deprecation Warning about QVariant::operator<(), which is udsed by QMap
//#pragma warning(push)
//#pragma warning (disable : 4996)
//#endif // _MSC_VER
#include "group_tree_model.h"
//#ifdef _MSC_VER
//#pragma warning(pop)
//#endif

#include "tree_item_type.h"
#include "item_data_roles.h"

#ifndef NDEBUG
  #ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4251 4800) // disable QDebug Warnings
  #endif // _MSC_VER

  #include <QDebug>

  #ifdef _MSC_VER
    #pragma warning(pop)
  #endif // _MSC_VER
#endif // NDEBUG

GroupTreeModel::GroupTreeModel(const QVector<int>& group_by_columns, QObject *parent)
  : QAbstractTreeModel(parent)
  , group_column_header_("Group")
  , group_by_columns_(group_by_columns)
{}

GroupTreeModel::~GroupTreeModel()
{}

int GroupTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::Group:
    return (model_column == groupColumn() ? 0 : -1);
  default:
    return -1;
  }
}

void GroupTreeModel::setGroupByColumns(const QVector<int>& group_by_columns)
{
  beginResetModel();

  group_by_columns_ = group_by_columns;

  // Detach all Items without deleting them (we want to reuse them)
  for (QAbstractTreeItem* tree_item : items_list_)
  {
    tree_item->detachFromParent();
  }

  // Delete everything else (At this point, only group items should be left)
  root()->removeAllChildren();
  group_map_.clear();
  auto old_item_list_ = items_list_;
  items_list_.clear();

  endResetModel();

  // Re-Add the Topic Items
  for (QAbstractTreeItem* tree_item : old_item_list_)
  {
    insertItemIntoGroups(tree_item);
  }

  // Set the column header
  QString group_header;
  if (group_by_columns.size() == 0)
  {
    group_header = "Group";
  }
  else
  {
    for (int i = 0; i < group_by_columns.size(); i++)
    {
      group_header += headerData(group_by_columns[i], Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
      if (i < (group_by_columns.size() - 1))
      {
        group_header += " / ";
      }
    }
  }
  setGroupColumnHeader(group_header);
}

QVector<int> GroupTreeModel::groupByColumns() const
{
  return group_by_columns_;
}

void GroupTreeModel::setGroupColumnHeader(const QVariant& header)
{
  group_column_header_ = header;
  headerDataChanged(Qt::Orientation::Horizontal, groupColumn(), groupColumn());
}

QVariant GroupTreeModel::groupColumnHeader() const
{
  return group_column_header_;
}

void GroupTreeModel::insertItemIntoGroups(QAbstractTreeItem* item)
{
  items_list_.push_back(item);

  if (group_by_columns_.size() == 0)
  {
    // No Grouping
    insertItem(item);
  }
  else
  {
    QVariant current_group_name        = item->data(mapColumnToItem(group_by_columns_[0], item->type()), Qt::ItemDataRole::DisplayRole);
    QVariant current_group_filter_role = item->data(mapColumnToItem(group_by_columns_[0], item->type()), (Qt::ItemDataRole)ItemDataRoles::FilterRole); //-V1016
    QVariant current_group_sort_role   = item->data(mapColumnToItem(group_by_columns_[0], item->type()), (Qt::ItemDataRole)ItemDataRoles::SortRole); //-V1016
    QVariant current_group_font_role = standardFont;
    QVariant current_group_identifier  = item->data(mapColumnToItem(group_by_columns_[0], item->type()), (Qt::ItemDataRole)ItemDataRoles::GroupRole); //-V1016


    QList<int> remaining_sub_groups;
    for (int i = 1; i < group_by_columns_.size(); i++)
    {
      remaining_sub_groups.push_back(mapColumnToItem(group_by_columns_[i], item->type()));
    }

    GroupTreeItem* group_item;
    const auto current_group_it = group_map_.find(current_group_identifier);
    if (current_group_it != group_map_.end())
    {
#ifndef NDEBUG
      qDebug().nospace() << "[" << metaObject()->className() << "]: Group " << current_group_name.toString() << " Exists already";
#endif // NDEBUG
      group_item = current_group_it->second;
    }
    else
    {
#ifndef NDEBUG
      qDebug().nospace() << "[" << metaObject()->className() << "] Adding Group " << current_group_name.toString();
#endif // NDEBUG
      group_item = new GroupTreeItem(current_group_name, current_group_filter_role, current_group_sort_role, current_group_font_role, current_group_identifier);

      group_map_[current_group_identifier] = group_item;
      insertItem(group_item);
    }

    // Iterate through group items until we find the leaf
    while (!remaining_sub_groups.isEmpty())
    {
      current_group_name        = item->data(remaining_sub_groups.front(), Qt::ItemDataRole::DisplayRole);
      current_group_filter_role = item->data(remaining_sub_groups.front(), (Qt::ItemDataRole)ItemDataRoles::FilterRole); //-V1016
      current_group_sort_role   = item->data(remaining_sub_groups.front(), (Qt::ItemDataRole)ItemDataRoles::SortRole); //-V1016
      current_group_font_role = standardFont;
      current_group_identifier  = item->data(remaining_sub_groups.front(), (Qt::ItemDataRole)ItemDataRoles::GroupRole); //-V1016

      auto subgroup_list = group_item->findChildren(
            [=](QAbstractTreeItem* child)
            {
              if (child->type() == (int)TreeItemType::Group)
              {
                GroupTreeItem* subgroup = (GroupTreeItem*)child;
                return current_group_identifier == subgroup->identifier();
              }
              else
              {
                return false;
              }
            });

      GroupTreeItem* subgroup;
      if (subgroup_list.isEmpty())
      {
        subgroup = new GroupTreeItem(current_group_name, current_group_filter_role, current_group_sort_role, current_group_font_role, current_group_identifier);
        QModelIndex group_index = index(group_item);
        insertItem(subgroup, group_index);
      }
      else
      {
        subgroup = (GroupTreeItem*)(subgroup_list.first());
      }
      remaining_sub_groups.pop_front();
      group_item = subgroup;
    }

    QModelIndex group_index = index(group_item);
    insertItem(item, group_index);
  }
}

void GroupTreeModel::removeItemFromGroups(QAbstractTreeItem* item, bool remove_empty_groups)
{
  // Remove the item and potentially unnecessary group items
  QAbstractTreeItem* item_to_remove = item;
  if (remove_empty_groups)
  {
    while (item_to_remove->parentItem() && item_to_remove->parentItem()->parentItem() && (item_to_remove->parentItem()->childCount() <= 1))
    {
      item_to_remove = item_to_remove->parentItem();
    }
  }

  if (item_to_remove->parentItem() == root())
  {
    auto it = std::find_if(std::begin(group_map_), std::end(group_map_),
                           [item_to_remove](auto& p) { return p.second == (GroupTreeItem*)item_to_remove; });

    if (it != std::end(group_map_))
    {
      group_map_.erase(it);
    }
    else
    {
#ifndef NDEBUG
      qDebug().nospace() << "[" << metaObject()->className() << "] removeItemFromGroups: Could not find group item in group map";
#endif // !NDEBUG
    }
  }

  removeItem(index(item_to_remove));
  items_list_.removeAll(item);
}

QVector<QPair<int, QString>> GroupTreeModel::getTreeItemColumnNameMapping() const
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
