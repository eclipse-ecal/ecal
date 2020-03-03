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

#include "plugin/plugin_manager.h"
#include <QAbstractItemModel>

class PluginTableModel : public QAbstractItemModel
{
  Q_OBJECT
public:

  PluginTableModel(QObject *parent);

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

  Qt::ItemFlags flags(const QModelIndex & index) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  QModelIndex parent(const QModelIndex &index) const;

  void rediscover();

private:
  enum class Columns : int
  {
    NAME,
    VERSION,
    AUTHOR,
    IID,
    PRIORITY,
    PATH,
    TOPICS,

    COLUMN_COUNT
  };

  const QMap<Columns, QString> header_data
  {
    { Columns::NAME,     "Name" },
    { Columns::VERSION,  "Version" },
    { Columns::AUTHOR,   "Author" },
    { Columns::IID,      "IID" },
    { Columns::PRIORITY, "Priority" },
    { Columns::PATH,     "Path" },
    { Columns::TOPICS,   "Topics" }
  };

};
