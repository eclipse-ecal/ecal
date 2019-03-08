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

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <CustomQt/QAbstractTreeItem.h>

class ItemViewModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit ItemViewModel(QAbstractTreeItem* tree_item, const QVector<QPair<int, QString>> & column_name_map, QObject *parent = 0);
  explicit ItemViewModel(const QVector<QPair<int, QString>> & column_name_map, QObject *parent = 0);

  ~ItemViewModel();

  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  void setTreeItem(QAbstractTreeItem* tree_item);
  QAbstractTreeItem* treeItem() const;

public slots:
  void update();

private:
  QAbstractTreeItem* tree_item_;
  QVector<QPair<int, QString>> column_name_map_;
};
