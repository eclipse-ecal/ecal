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

#pragma once
#include "QStableSortFilterProxyModel.h"

#include <QVector>

/**
 * @brief The QMulticolumnSortFilterProxyModel is a proxy model that filters for multiple columns simultaneously
 *
 * If any column is accepted by the filter, the row will appear in the output.
 * The columns can be set by @code{setFilterKeyColumns(const QVector<int>&)}.
 * The default is an empty list, i.e. no row will be accepted, as the filter
 * cannot match any column.
 *
 * Note that when calling the "old" @code{setFilterKeyColumn(int)} function,
 * the corresponding object must not be of type @see{QStableSortFilterProxyModel}
 * or @see{QSortFilterProxyModel}, as the function is not virtual and can
 * therefore not be overwritten by this class.
 *
 */
class QMulticolumnSortFilterProxyModel : public QStableSortFilterProxyModel
{
public:

  QMulticolumnSortFilterProxyModel(QObject* parent = 0);
  ~QMulticolumnSortFilterProxyModel();

////////////////////////////////////////////
// Filtering
////////////////////////////////////////////
public:

  void setFilterKeyColumn(int column) { setFilterKeyColumns(QVector<int>{column}); } // QSortFilterProxyModel::setFilterKeyColumn is not virtual, unfortunatelly

  /**
   * @brief Sets the columns that are used for filtering
   *
   * If the filter matches any column, the row will appear in the output. The
   * default is an empty list, i.e. no row will be accepted, as the filter
   * cannot match any column.
   *
   * @param column the columns that are used for filtering
   */
  void setFilterKeyColumns(const QVector<int>& columns);

  /**
   * @brief Returns the columns that are used for filtering
   * @return
   */
  QVector<int> filterKeyColumns() const;

  Q_DECL_DEPRECATED int filterKeyColumn() const { return QSortFilterProxyModel::filterKeyColumn(); } // QSortFilterProxyModel::setFilterKeyColumn is not virtual, unfortunatelly

protected:
  bool filterDirectAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

////////////////////////////////////////////
// Sorting
////////////////////////////////////////////
public:
  bool lessThan(const QModelIndex &rLeft, const QModelIndex &rRight) const override;

  virtual void setAlwaysSortedColumn(int column);
  virtual void setAlwaysSortedColumn(int column, Qt::SortOrder forced_sort_order);
  virtual int  alwaysSortedColumn() const;

////////////////////////////////////////////
// Member Variables
////////////////////////////////////////////
private:
  QVector<int>  filter_columns_;                                                /**< The columns that are used for filtering */

  int           always_sorted_column_;                                          /**< A column that will always be kept sorted. */
  bool          always_sorted_force_sort_order_;                                /**< Whether we force a sort order for the always sorted column and ignore the user-set sort order. */
  Qt::SortOrder always_sorted_sort_order_;                                      /**< A column that will always be kept sorted. */
};

