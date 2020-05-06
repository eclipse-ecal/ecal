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

#include "column_tree_item.h"

#include <QColor>
#include <QFont>

#include "tree_item_type.h"
#include "item_data_roles.h"

ColumnTreeItem::ColumnTreeItem(const QString& name, int number)
  : QAbstractTreeItem()
  , name_(name)
  , number_(number)
{}

ColumnTreeItem::~ColumnTreeItem()
{}

QVariant ColumnTreeItem::data(int column, Qt::ItemDataRole role) const
{
  if (role == Qt::ItemDataRole::DisplayRole)
  {
    switch (column)
    {
    case (int)Columns::INDEX:
      return row() + 1;
    case (int)Columns::NAME:
      return name_;
    default:
      return QVariant();
    }
  }
  else if (role == Qt::ItemDataRole::EditRole)
  {
    switch (column)
    {
    case (int)Columns::NAME:
      return number_;
    default:
      return QVariant();
    }
  }
  else if (role == ItemDataRoles::FilterRole) //-V547
  {
    // No special values here
    return data(column, Qt::ItemDataRole::DisplayRole);
  }
  else if (role == ItemDataRoles::SortRole) //-V547
  {
    // No special values here
    return data(column, Qt::ItemDataRole::DisplayRole);
  }
  return QVariant();
}

bool ColumnTreeItem::setData(int column, const QVariant& data, Qt::ItemDataRole role)
{
  if (role == Qt::ItemDataRole::EditRole)
  {
    if (column == (int)Columns::NAME)
    {
      number_ = data.toInt();
      return true;
    }
  }
  if (role == Qt::ItemDataRole::DisplayRole)
  {
    if (column == (int)Columns::NAME)
    {
      name_ = data.toString();
      return true;
    }
  }
  return false;
}

void ColumnTreeItem::setName(const QString& name)
{
  name_ = name;
}

QString ColumnTreeItem::name() const
{
  return name_;
}

void ColumnTreeItem::setNumber(int number)
{
  number_ = number;
}

int ColumnTreeItem::number() const
{
  return number_;
}

int ColumnTreeItem::type() const
{
  return (int)TreeItemType::Column;
}

Qt::ItemFlags ColumnTreeItem::flags(int column) const
{
  if (column == (int)Columns::NAME)
  {
    return QAbstractTreeItem::flags(column) | Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsDragEnabled | Qt::ItemFlag::ItemIsDropEnabled;
  }
  else
  {
    return QAbstractTreeItem::flags(column) | Qt::ItemFlag::ItemIsDragEnabled | Qt::ItemFlag::ItemIsDropEnabled;
  }
}
