/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "group_state_tree_item.h"

#include <QColor>
#include <QFont>

#include "tree_item_types.h"
#include "tree_data_roles.h"

GroupStateTreeItem::GroupStateTreeItem(std::shared_ptr<TaskGroup::GroupState> group_state)
  : QAbstractTreeItem()
  , group_state_(group_state)
{
}


GroupStateTreeItem::~GroupStateTreeItem()
{
}

QVariant GroupStateTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant GroupStateTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if (role == Qt::ItemDataRole::DisplayRole)
  {
    if (column == Columns::NAME)
    {
      QString name = group_state_->GetName().c_str();
      if (!name.isEmpty())
      {
        return name;
      }
      else
      {
        return "Unnamed State";
      }
    }
  }

  else if (role == Qt::ItemDataRole::FontRole)
  {
    if (column == Columns::NAME && (group_state_->GetName() == ""))
    {
      QFont italic_font;
      italic_font.setItalic(true);
      return italic_font;
    }
    else
    {
      return QVariant();
    }
  }

  else if (role == Qt::ItemDataRole::ToolTipRole)
  {
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  return QVariant::Invalid;
}

std::shared_ptr<TaskGroup::GroupState> GroupStateTreeItem::getGroupState()
{
  return group_state_;
}


int GroupStateTreeItem::type() const
{
  return (int)TreeItemType::GroupState;
}