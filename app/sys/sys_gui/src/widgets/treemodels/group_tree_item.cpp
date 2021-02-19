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

#include "group_tree_item.h"

#include <QColor>
#include <QFont>

#include "tree_item_types.h"
#include "tree_data_roles.h"

GroupTreeItem::GroupTreeItem(std::shared_ptr<TaskGroup> group)
  : QAbstractTreeItem()
  , group_(group)
{
}


GroupTreeItem::~GroupTreeItem()
{
}

QVariant GroupTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant GroupTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if (role == Qt::ItemDataRole::DisplayRole)
  {
    if (column == Columns::ID)
    {
      return (unsigned long long) group_->GetId();
    }
    else if (column == Columns::NAME)
    {
      QString name = group_->GetName().c_str();
      if (!name.isEmpty())
      {
        return name;
      }
      else
      {
        return "Unnamed Group";
      }
    }
    else if (column == Columns::STATE)
    {
      // Get the active state
      auto state = group_->Evaluate();
      if (state)
      {
        return QString(state->GetName().c_str());
      }
      else
      {
        return QVariant();
      }
    }
  }

  else if (role == Qt::ItemDataRole::BackgroundRole)
  {
    if (column == Columns::STATE)
    {
      // Get the active state
      auto state = group_->Evaluate();
      if (state)
      {
        auto state_color = state->GetColor();
        return QColor(state_color.red, state_color.green, state_color.blue);
      }
      else
      {
        return QVariant();
      }
    }
  }

  else if (role == Qt::ItemDataRole::FontRole)
  {
    if (column == Columns::NAME && group_->GetName() == "")
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

  else if (role == ItemDataRoles::SortRole)
  {
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  else if (role == ItemDataRoles::FilterRole) //-V547
  {
    if (column == Columns::NAME)
    {
      QString name = group_->GetName().c_str();
      return name;
    }
    else
    {
      return data(column, Qt::ItemDataRole::DisplayRole);
    }
  }

  else if (role == Qt::ItemDataRole::ToolTipRole)
  {
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  return QVariant::Invalid;
}

std::shared_ptr<TaskGroup> GroupTreeItem::getGroup()
{
  return group_;
}


int GroupTreeItem::type() const
{
  return (int)TreeItemType::Group;
}
