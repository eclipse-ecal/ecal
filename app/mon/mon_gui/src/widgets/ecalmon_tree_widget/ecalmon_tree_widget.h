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

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMap>
#include "ui_ecalmon_tree_widget.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "widgets/models/group_tree_model.h"
#include <CustomQt/QMulticolumnSortFilterProxyModel.h>

class EcalmonTreeWidget : public QWidget
{
  Q_OBJECT

public:

  struct GroupSetting
  {
    QString name;
    QList<int> group_by_columns;
    int auto_expand;
  };

  EcalmonTreeWidget(QWidget *parent = Q_NULLPTR);
  ~EcalmonTreeWidget();

  void setModel(GroupTreeModel* model);
  void setAdditionalProxyModel(QSortFilterProxyModel* proxy_model);

  void setFilterColumns(const QVector<int>& columns);
  void setGroupSettings(const QList<GroupSetting>& preconfigured_groups, const QList<int>& group_enabled_columns);
  void setVisibleColumns(const QVector<int>& columns);
  void setDefaultForcedColumn(int column);

signals:
  void topicsUpdated(const eCAL::pb::Monitoring& monitoring_pb);

public slots:
  virtual void monitorUpdated(const eCAL::pb::Monitoring& enable);
  void setAlternatingRowColors(bool alternating_colors_enabled);
  virtual void resetLayout();

private slots:
  void contextMenu(const QPoint &pos);

  void copyCurrentIndexToClipboard();
  void copySelectedRowToClipboard();

protected:
  Ui::EcalmonTreeWidget ui_;

  virtual QModelIndex mapToSource(const QModelIndex& proxy_index) const;
  virtual QModelIndex mapFromSource(const QModelIndex& source_index) const;

  QList<QAbstractTreeItem*> getSelectedItems() const;

  void groupByColumns(const QVector<int>& columns);

  virtual void fillContextMenu(QMenu& menu, const QList<QAbstractTreeItem*>& selected_items);

  virtual void saveGuiSettings(const QString& group);
  virtual void loadGuiSettings(const QString& group);
  virtual void saveInitialState();

private:
  static const int GROUP_BY_COLUMN_LIST_ROLE = Qt::ItemDataRole::UserRole + 1;
  static const int GROUP_BY_AUTOEXPAND_ROLE  = Qt::ItemDataRole::UserRole + 2;
  static const int GROUP_BY_CUSTOM_ROLE      = Qt::ItemDataRole::UserRole + 3;

  GroupTreeModel*                   group_tree_model_;                          /**< The source group tree model */
  QMulticolumnSortFilterProxyModel* filter_proxy_model_;                        /**< The Model for the filter input and normal sorting */
  QSortFilterProxyModel*            additional_proxy_model_;                    /**< Any other filter model, e.g. for the "show all" checkbox. May stay a nullptr. */

  QStandardItemModel* filter_combobox_model_;
  QStandardItemModel* group_by_combobox_model_;

  QVector<int>    filter_columns_;                                              /**< Columns that are enabled for filtering */
  QList<int>      group_by_enabled_columns_;                                    /**< Columns that are enabled for grouping */
  int             default_forced_column_;                                       /**< The default forced column(-s) for the tree view. Used when grouping by nothing. */
  QMap<int, bool> autohidden_columns_visibility;                                /**< Contains information wether an autohidden column was visibile before (needed for restoring the state) */

  QByteArray initial_tree_state_;
  int        initial_group_by_index_;
  bool       initial_show_all_checkbox_state_;

  void groupByCustom();
};
