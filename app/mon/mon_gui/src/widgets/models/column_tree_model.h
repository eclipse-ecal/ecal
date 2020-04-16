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

#include <CustomQt/QAbstractTreeModel.h>

#include "column_tree_item.h"

#include <QMap>

class ColumnTreeModel : public QAbstractTreeModel
{
  Q_OBJECT

public:

  enum class Columns : int
  {
    INDEX,
    COLUMN_NAME,

    COLUMN_COUNT
  };

  ColumnTreeModel(QObject *parent = nullptr);
  ~ColumnTreeModel();

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;

private:
  std::map<Columns, QString> columnLabels =
  {
    { Columns::INDEX,       "#" },
    { Columns::COLUMN_NAME, "Column" },
  };

  std::map<Columns, int> column_tree_item_column_mapping =
  {
    { Columns::INDEX,       (int)ColumnTreeItem::Columns::INDEX },
    { Columns::COLUMN_NAME, (int)ColumnTreeItem::Columns::NAME },
  };
};
