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
#include <CustomQt/QStableSortFilterProxyModel.h>

class TopicSortFilterProxyModel :
  public QStableSortFilterProxyModel
{
public:

  TopicSortFilterProxyModel(QObject* parent = 0);
  ~TopicSortFilterProxyModel();

  void setRegExpLists(const QList<QRegExp>& exclude_list, const QList<QRegExp>& include_list);

private:
  QList<QRegExp> exclude_regexp_list_;
  QList<QRegExp> include_regexp_list_;
  
  bool filterDirectAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

