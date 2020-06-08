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

#include "function_list_model.h"

//////////////////////////////////////////////////////
// Constructor & destructor
//////////////////////////////////////////////////////

FunctionListModel::FunctionListModel(QObject *parent)
  : QAbstractItemModel(parent)
{
  // Populate with data
  auto function_map = EcalParser::GetFunctions();
  function_list_.reserve(function_map.size() + 1);

  // Add general help item
  function_list_.emplace_back(std::make_pair(QString("=== General help ==="), nullptr));

  // Add function help items
  for (const auto& function_pair : function_map)
  {
    function_list_.emplace_back(std::make_pair(QString::fromStdString(function_pair.first), function_pair.second));
  }
}

FunctionListModel::~FunctionListModel()
{}

//////////////////////////////////////////////////////
// Reimplemented from QAbstractItemModel
//////////////////////////////////////////////////////

QVariant FunctionListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant::Invalid;

  const int     row    = index.row();
  const Columns column = (Columns)index.column();

  if (column == Columns::NAME)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return function_list_[row].first;
    }
  }

  else if (column == Columns::DESCRIPTION)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      if (function_list_[row].second)
        return QString::fromStdString(function_list_[row].second->Description());
    }
  }

  return QVariant::Invalid;
}

QVariant FunctionListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((role == Qt::ItemDataRole::DisplayRole)
    && (orientation == Qt::Orientation::Horizontal))
  {
    return column_labels_.at((Columns)section);
  }
  return QVariant::Invalid;
}

QModelIndex FunctionListModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
  return createIndex(row, column, nullptr);
}

QModelIndex FunctionListModel::index(const QString& function_name, int column) const
{
  for (size_t i = 0; i < function_list_.size(); i++)
  {
    if (function_name == function_list_[i].first)
      return index((int)i, column);
  }
  return QModelIndex();
}

QModelIndex FunctionListModel::parent(const QModelIndex& /*index*/) const
{
  return QModelIndex();
}

int FunctionListModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
  {
    // the invisible root node has children
    return (int)function_list_.size();
  }

  // The entries do not have children
  return 0;
}

int FunctionListModel::columnCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
  {
    // the invisible root node has children
    return (int)Columns::COLUMN_COUNT;
  }

  // The entries do not have children
  return 0;
}


//////////////////////////////////////////////////////
// Auxiliary functions
//////////////////////////////////////////////////////

std::pair<QString, EcalParser::Function*> FunctionListModel::getFunction(int row) const
{
  return function_list_[row];
}
