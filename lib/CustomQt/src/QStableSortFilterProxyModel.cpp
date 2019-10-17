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

#include "CustomQt/QStableSortFilterProxyModel.h"

QStableSortFilterProxyModel::QStableSortFilterProxyModel(QObject* parent)
  : QSortFilterProxyModel(parent)
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  , recursive_filtering_enabled(false)
#endif
{}

QStableSortFilterProxyModel::~QStableSortFilterProxyModel()
{}

bool QStableSortFilterProxyModel::lessThan(const QModelIndex &rLeft, const QModelIndex &rRight) const
{
  QVariant const leftData = sourceModel()->data(rLeft, sortRole());
  QVariant const rightData = sourceModel()->data(rRight, sortRole());

  if (leftData == rightData)
  {
    // Use the unsorted order as tie breaker
    return rLeft.row() < rRight.row();
  }
  else
  {
    return QSortFilterProxyModel::lessThan(rLeft, rRight);
  }
}

bool QStableSortFilterProxyModel::filterDirectAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  // This is a compatibility function to simplify backporting of Qt 5.10's recursive filtering option
  return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool QStableSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
  if (recursive_filtering_enabled)
  {
    if (filterDirectAcceptsRow(source_row, source_parent))
    {
      return true;
    }
    else
    {
      QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
      if (index.isValid() && sourceModel()->hasChildren(index))
      {
        for (int i = 0; i < sourceModel()->rowCount(index); i++)
        {
          if (filterAcceptsRow(i, index))
          {
            return true;
          }
        }
      }
      return false;
    }
  }
  else
  {
    return filterDirectAcceptsRow(source_row, source_parent);
  }
#else // QT_VERSION
  return filterDirectAcceptsRow(source_row, source_parent);
#endif // QT_VERSION
}


#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
void QStableSortFilterProxyModel::setRecursiveFilteringEnabled(bool recursive)
{
  bool invalidate = (recursive_filtering_enabled != recursive);
  recursive_filtering_enabled = recursive;
  if (invalidate)
  {
    invalidateFilter();
  }
}

bool QStableSortFilterProxyModel::isRecursiveFilteringEnabled() const
{
  return recursive_filtering_enabled;
}

void QStableSortFilterProxyModel::setSourceModel(QAbstractItemModel *source_model)
{
  QAbstractItemModel* old_source = sourceModel();
  if (old_source)
  {
    disconnect(old_source, &QAbstractItemModel::rowsInserted, this, &QStableSortFilterProxyModel::invalidateFilterIfRecursiveFilteringEnabled);
    disconnect(old_source, &QAbstractItemModel::rowsRemoved, this,  &QStableSortFilterProxyModel::invalidateFilterIfRecursiveFilteringEnabled);
    disconnect(old_source, &QAbstractItemModel::dataChanged, this,  &QStableSortFilterProxyModel::invalidateFilterIfRecursiveFilteringEnabled);
  }

  QSortFilterProxyModel::setSourceModel(source_model);

  connect(source_model, &QAbstractItemModel::rowsInserted, this, &QStableSortFilterProxyModel::invalidateFilterIfRecursiveFilteringEnabled);
  connect(source_model, &QAbstractItemModel::rowsRemoved, this,  &QStableSortFilterProxyModel::invalidateFilterIfRecursiveFilteringEnabled);
  connect(source_model, &QAbstractItemModel::dataChanged, this,  &QStableSortFilterProxyModel::invalidateFilterIfRecursiveFilteringEnabled);
}

void QStableSortFilterProxyModel::invalidateFilterIfRecursiveFilteringEnabled()
{
  if (isRecursiveFilteringEnabled())
  {
    invalidateFilter();
  }
}
#endif
