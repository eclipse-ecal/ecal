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

#include "runner_tree_item.h"

#include <QColor>
#include <QFont>

#include "tree_item_types.h"
#include "tree_data_roles.h"

RunnerTreeItem::RunnerTreeItem(std::shared_ptr<EcalSysRunner> runner)
  : QAbstractTreeItem()
  , runner_(runner)
  , import_checked_(true)
{
}


RunnerTreeItem::~RunnerTreeItem()
{
}

QVariant RunnerTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant RunnerTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if (role == Qt::ItemDataRole::DisplayRole)
  {
    if (column == Columns::ID)
    {
      if (runner_)
      {
        return (unsigned long long) runner_->GetId();
      }
      else
      {
        return "";
      }
    }
    else if (column == Columns::NAME)
    {
      if (runner_)
      {
        QString name = runner_->GetName().c_str();
        if (!name.isEmpty())
        {
          return name;
        }
        else
        {
          return "Unnamed Runner";
        }
      }
      else
      {
        return "- No Runner -";
      }
    }
    else if (column == Columns::PATH)
    {
      if (runner_)
      {
        return runner_->GetPath().c_str();
      }
      else
      {
        return "";
      }
    }
    else if (column == Columns::DEFAULT_ALGO_DIR)
    {
      if (runner_)
      {
        return runner_->GetDefaultAlgoDir().c_str();
      }
      else
      {
        return "";
      }
    }
    else if (column == Columns::COMMAND_ARG)
    {
      if (runner_)
      {
        return runner_->GetLoadCmdArgument().c_str();
      }
      else
      {
        return "";
      }
    }
  }

  else if (role == Qt::ItemDataRole::FontRole)
  {
    if (column == Columns::NAME && (!runner_ || runner_->GetName() == ""))
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
    if (runner_)
    {
      switch (column)
      {
      case Columns::NAME:
        return runner_->GetName().c_str();
      default:
        return data(column, Qt::ItemDataRole::DisplayRole);
      }
    }
    else
    {
      return QVariant();
    }
  }

  else if (role == ItemDataRoles::FilterRole) //-V547
  {
    if (runner_)
    {
      switch (column)
      {
      case Columns::NAME:
        // not return "Unnamed Runner" when the runner has no name
        return runner_->GetName().c_str();
      default:
        return data(column, Qt::ItemDataRole::DisplayRole);
      }
    }
    else
    {
      return QVariant();
    }
  }

  else if (role == Qt::ItemDataRole::CheckStateRole)
  {
    if (column == Columns::IMPORT_CHECKED)
    {
      return import_checked_ ? Qt::Checked : Qt::Unchecked;
    }
  }

  else if (role == Qt::ItemDataRole::ToolTipRole)
  {
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  return QVariant::Invalid;
}


bool RunnerTreeItem::setData(int column, const QVariant& data, Qt::ItemDataRole role)
{
  if (role == Qt::ItemDataRole::CheckStateRole)
  {
    if (column == (int)Columns::IMPORT_CHECKED)
    {
      import_checked_ = data.toBool();
      return true;
    }
  }
  return QAbstractTreeItem::setData(column, data, role);
}

Qt::ItemFlags RunnerTreeItem::flags(int column) const
{
  if (column == (int)Columns::IMPORT_CHECKED)
    return (QAbstractTreeItem::flags(column) | Qt::ItemFlag::ItemIsUserCheckable) & ~Qt::ItemFlag::ItemIsEnabled;
  return QAbstractTreeItem::flags(column);
}

std::shared_ptr<EcalSysRunner> RunnerTreeItem::getRunner() const
{
  return runner_;
}


int RunnerTreeItem::type() const
{
  return (int)TreeItemType::Runner;
}

bool RunnerTreeItem::isImportChecked() const
{
  return import_checked_;
}
