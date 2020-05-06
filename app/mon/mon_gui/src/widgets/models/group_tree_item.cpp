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

#include "group_tree_item.h"

#include <QColor>
#include <QFont>

#include "tree_item_type.h"
#include "item_data_roles.h"

GroupTreeItem::GroupTreeItem(const QVariant& display_role,
                             const QVariant& filter_role,
                             const QVariant& sort_role,
                             const QVariant& font_role,
                             const QVariant& group_identifier)
  : QAbstractTreeItem()
  , display_role_    (display_role)
  , filter_role_     (filter_role)
  , sort_role_       (sort_role)
  , font_role_       (font_role)
  , group_identifier_(group_identifier)
{}

GroupTreeItem::~GroupTreeItem()
{}

QVariant GroupTreeItem::data(int column, Qt::ItemDataRole role) const
{
  if (column == 0)
  {
    switch ((int)role)
    {
    case Qt::ItemDataRole::DisplayRole:
      return display_role_;
    case ItemDataRoles::FilterRole:
      return filter_role_;
    case ItemDataRoles::SortRole:
      return sort_role_;
    case Qt::ItemDataRole::FontRole:
      return font_role_;
    case ItemDataRoles::GroupRole:
      return group_identifier_;
    default:
      return QVariant::Invalid;
    }
  }
  return QVariant::Invalid;
}

int GroupTreeItem::type() const
{
  return (int)TreeItemType::Group;
}

QVariant GroupTreeItem::name() const
{
  return display_role_;
}

QVariant GroupTreeItem::identifier() const
{
  return group_identifier_;
}