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

#include <QStyledItemDelegate>

#include <QStandardItemModel>

#include <set>

class HostFilterDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
  HostFilterDelegate(QAbstractItemView* parent);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

public slots:
  void setMonitorHosts       (const std::set<QString>& host_list);
  void setHostsCurrentlyInUse(const std::set<QString>& host_list);

private slots:
  void indexActivated(int index);

private:
  void setHosts(const std::set<QString>& host_list, bool monitor_visible, bool currently_in_use);
  void addAvailableHost(const QString& new_host, bool monitor_visible, bool currently_in_use);
  void cleanModel();
  int indexOf(const QString& host) const;

private:
  mutable bool first_run_;
  bool dialog_open_;
  mutable bool open_popup_;

  QStandardItemModel* available_hosts_model_;


  enum class HostItemType : int
  {
    All,
    Custom,
    Host
  };

  static const int ItemTypeRole          = Qt::ItemDataRole::UserRole;
  static const int HostFilterListRole    = Qt::ItemDataRole::UserRole + 1;
  static const int AvailableInMonitoring = Qt::ItemDataRole::UserRole + 2;
  static const int CurrentlyInUse        = Qt::ItemDataRole::UserRole + 3;
};
