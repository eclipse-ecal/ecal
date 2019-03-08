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

#include <QObject>
#include <QAbstractProxyModel>

#include <QMap>
#include <QVariant>
#include <QModelIndex>

class QGroupByProxyModel : public QAbstractProxyModel
{
  Q_OBJECT

public:
  QGroupByProxyModel(QObject* parent = NULL);
  ~QGroupByProxyModel();

  void setGroupRole(int role);

  int groupRole() const;

  void setSourceModel(QAbstractItemModel *model);

  void setGroupBy(int column);
  QString whichGroup(int row) const;

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index) const;
  QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
  QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
  QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const;
  QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;

  QString groupFromValue(QString header_string, QString value_string) const;

  void setGroups();
  void clearGroups();

private:
  int group_role_;

  //TODO: make more generic
  int group_by_column_ = 1;

  QList<QString> groups_;
  QList<QModelIndex> group_indices_;
  QMap<QString, QVector<int>*> group_row_to_source_row_;
  QVector<int> source_row_to_group_row_;
};
