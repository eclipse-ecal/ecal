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

#include "log_sort_filter_proxy_model.h"

#include "widgets/models/log_model.h"

#include "item_data_roles.h"

LogSortFilterProxyModel::LogSortFilterProxyModel(QObject* parent)
  : QMulticolumnSortFilterProxyModel(parent), log_level_filter_(log_level_none)
{
  setRecursiveFilteringEnabled(true);
}

LogSortFilterProxyModel::~LogSortFilterProxyModel()
{}

bool LogSortFilterProxyModel::filterDirectAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  QModelIndex index = sourceModel()->index(source_row, LogModel::Columns::LOG_LEVEL, source_parent);
  int log_level = sourceModel()->data(index, filterRole()).toInt();
  if (log_level_filter_ & log_level)
  {
    return QMulticolumnSortFilterProxyModel::filterDirectAcceptsRow(source_row, source_parent);
  }
  
  return false;
}

void LogSortFilterProxyModel::setLogLevelFilter(eCAL_Logging_eLogLevel log_levels)
{
  log_level_filter_ = log_levels;
  invalidateFilter();
}

eCAL_Logging_eLogLevel LogSortFilterProxyModel::logLevelFilter() const
{
  return log_level_filter_;
}