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
  , always_sorted_column_          (-1)
  , always_sorted_force_sort_order_(false)
  , always_sorted_sort_order_      (Qt::SortOrder::AscendingOrder)
{}

////////////////////////////////////////////
// Filtering
////////////////////////////////////////////

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

////////////////////////////////////////////
// Sorting
////////////////////////////////////////////

bool QMulticolumnSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  if (always_sorted_column_ >= 0)
  {
    QVariant const left_data   = sourceModel()->data(sourceModel()->index(left.row(), always_sorted_column_, left.parent()), sortRole());
    QVariant const right_data  = sourceModel()->data(sourceModel()->index(right.row(), always_sorted_column_, right.parent()), sortRole());

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable : 4996)
#endif // _MSC_VER
    if (left_data != right_data)
    {
      if (!always_sorted_force_sort_order_)
      {
        // Don't fake the sort order, we have to follow the user-set one

        // Qt Deprecation note:
        // 
        // QVariant::operator< is deprecated since Qt 5.15, mainly because
        // QVariants may contain datatypes that are not comparable at all. The
        // developer is now supposed to unpack the QVariant and compare the
        // native values. In a SortFilterProxyModel we cannot do that, as we
        // don't know what this model will be used for and what the type of the
        // QVariants will be.
        // Thus, we would have to implement a best guess approach anyways. As I
        // doubt that I will hack a custom comparator that is better than the
        // deprecated QVariant one, we will just keep using it.
        // 
        // The operators still exist in Qt 6.0 beta.
        return (left_data < right_data);
      }
      else
      {
        // We want to ignore the user-set sort order, so we fake the less-than method
        if (sortOrder() == always_sorted_sort_order_)
          return (left_data < right_data); // Qt 5.15 Deprecation note: see above
        else
          return (left_data > right_data); // Qt 5.15 Deprecation note: see above
      }
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER
  }

  return QStableSortFilterProxyModel::lessThan(left, right);
}


void QMulticolumnSortFilterProxyModel::setAlwaysSortedColumn(int column)
{
  always_sorted_force_sort_order_ = false;
  always_sorted_column_           = column;

  invalidate();
}

void QMulticolumnSortFilterProxyModel::setAlwaysSortedColumn(int column, Qt::SortOrder forced_sort_order)
{
  always_sorted_force_sort_order_ = true;
  always_sorted_sort_order_       = forced_sort_order;
  always_sorted_column_           = column;

  invalidate();
}

int QMulticolumnSortFilterProxyModel::alwaysSortedColumn() const
{
  return always_sorted_column_;
}