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

#include "topic_sort_filter_proxy_model.h"

#include <CustomQt/QAbstractTreeItem.h>
#include <CustomQt/QAbstractTreeModel.h>

#include "widgets/models/tree_item_type.h"

TopicSortFilterProxyModel::TopicSortFilterProxyModel(QObject* parent)
  : QStableSortFilterProxyModel(parent)
{
  setRecursiveFilteringEnabled(true);
}

TopicSortFilterProxyModel::~TopicSortFilterProxyModel()
{}

void TopicSortFilterProxyModel::setRegExpLists(const QList<QRegExp>& exclude_list, const QList<QRegExp>& include_list)
{
  exclude_regexp_list_ = exclude_list;
  include_regexp_list_ = include_list;
  invalidateFilter();
}

bool TopicSortFilterProxyModel::filterDirectAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  QModelIndex index = sourceModel()->index(source_row, filterKeyColumn(), source_parent);

  if (!index.isValid())
    return false;

  QString data = sourceModel()->data(index, filterRole()).toString();

  // Group Items are never accepted directly, they only stay if there is a topic item that is accepted
  QAbstractTreeItem* tree_item = ((QAbstractTreeModel*)sourceModel())->item(index);
  if (tree_item->type() == (int)TreeItemType::Group)
  {
    return false;
  }


  for (const QRegExp& regexp : exclude_regexp_list_)
  {
    if (regexp.exactMatch(data))
    {
      return false;
    }
  }

  if (include_regexp_list_.empty())
  {
    return true;
  }
  else
  {
    for (const QRegExp& regexp : include_regexp_list_)
    {
      if (regexp.exactMatch(data))
      {
        return true;
      }
    }
    return false;
  }
}
