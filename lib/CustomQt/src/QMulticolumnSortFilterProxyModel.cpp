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

#include "CustomQt/QMulticolumnSortFilterProxyModel.h"

#include <numeric>

QMulticolumnSortFilterProxyModel::QMulticolumnSortFilterProxyModel(QObject* parent)
  : QStableSortFilterProxyModel(parent)
{}

QMulticolumnSortFilterProxyModel::~QMulticolumnSortFilterProxyModel()
{}

void QMulticolumnSortFilterProxyModel::setFilterKeyColumns(const QVector<int>& columns)
{
  if (!columns.empty())
  {
    QSortFilterProxyModel::setFilterKeyColumn(columns[0]);
  }
  else
  {
    QSortFilterProxyModel::setFilterKeyColumn(-1);
  }
  filter_columns_ = columns;
  invalidateFilter();
}

QVector<int> QMulticolumnSortFilterProxyModel::filterKeyColumns() const
{
  return filter_columns_;
}

bool QMulticolumnSortFilterProxyModel::filterDirectAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  QRegExp filter_regexp = filterRegExp();

  for (int column : filter_columns_)
  {
    QModelIndex index = sourceModel()->index(source_row, column, source_parent);
    if (index.isValid())
    {
      QString data = sourceModel()->data(index, filterRole()).toString();
      if (data.contains(filter_regexp))
      {
        return true;
      }
    }
  }
  return false;
}
