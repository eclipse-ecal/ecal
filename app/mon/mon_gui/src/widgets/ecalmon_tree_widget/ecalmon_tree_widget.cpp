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

#include "ecalmon_tree_widget.h"

#include "widgets/column_selector_dialog/column_selector_dialog.h"
#include "widgets/models/item_data_roles.h"
#include "widgets/models/tree_item_type.h"

#include "ecalmon_globals.h"
#include "util.h"

#include <QSettings>
#include <QMenu>
#include <QClipboard>

////////////////////////////////////////////////////////////////////////////////
//// Constructor & Destructor                                               ////
////////////////////////////////////////////////////////////////////////////////

EcalmonTreeWidget::EcalmonTreeWidget(QWidget *parent)
  : QWidget(parent)
  , group_tree_model_(nullptr)
  , additional_proxy_model_(nullptr)
  , default_forced_column_(0)
{
  ui_.setupUi(this);

  // By default hide the show-all checkbox
  ui_.show_all_checkbox->setHidden(true);

  // Set a nicer icon for the filter lineedit
  ui_.filter_lineedit->setClearIcon(QIcon(":/ecalicons/FILTER_CANCEL"));

  // Filter Lineedit
  filter_proxy_model_ = new QMulticolumnSortFilterProxyModel(this);
  filter_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  filter_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  filter_proxy_model_->setRecursiveFilteringEnabled(true);
  filter_proxy_model_->setFilterRole(ItemDataRoles::FilterRole);
  filter_proxy_model_->setSortRole(ItemDataRoles::SortRole);
  ui_.tree_view->setModel(filter_proxy_model_);
  connect(ui_.filter_lineedit, &QLineEdit::textChanged, [this](const QString& text) { filter_proxy_model_->setFilterFixedString(text); });

  // Filter Combobox
  filter_combobox_model_ = new QStandardItemModel(this);
  ui_.filter_combobox->setModel(filter_combobox_model_);
  connect(ui_.filter_combobox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      [this]()
      {
        int filter_column = ui_.filter_combobox->currentData(Qt::UserRole).toInt();
        if (filter_column < 0)
          filter_proxy_model_->setFilterKeyColumns(filter_columns_);
        else
          filter_proxy_model_->setFilterKeyColumn(filter_column);
      });

  // Group By Combobox
  group_by_combobox_model_ = new QStandardItemModel(this);
  ui_.group_by_combobox->setModel(group_by_combobox_model_);
  connect(ui_.group_by_combobox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      [this](int current_index)
      {
        if (current_index >= 0)
        {
          if (ui_.group_by_combobox->currentData(GROUP_BY_CUSTOM_ROLE).toBool())
          {
            groupByCustom();
          }
          else
          {
            QVector<int> column_list;
            for (auto& qvariant_column : ui_.group_by_combobox->currentData(GROUP_BY_COLUMN_LIST_ROLE).toList())
            {
              column_list.push_back(qvariant_column.toInt());
            }
            int auto_expand_level = ui_.group_by_combobox->currentData(GROUP_BY_AUTOEXPAND_ROLE).toInt();
            ui_.tree_view->setAutoExpandDepth(auto_expand_level);
            groupByColumns(column_list);
          }
        }
      });

  // Buttons
  connect(ui_.expand_button, &QPushButton::clicked, [this]() { ui_.tree_view->expandAll(); });
  connect(ui_.collapse_button, &QPushButton::clicked, [this]() { ui_.tree_view->collapseAll(); });
  connect(ui_.autosize_button, &QPushButton::clicked,
      [this]()
      {
        for (int i = 0; i < ui_.tree_view->model()->columnCount(); i++)
        {
          if (!ui_.tree_view->isColumnHidden(i))
          {
            ui_.tree_view->resizeColumnToContents(i);
          }
        }
      });

  // Context menu
  ui_.tree_view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.tree_view, &QTreeView::customContextMenuRequested, this, &EcalmonTreeWidget::contextMenu);

  connect(ui_.tree_view, &QAdvancedTreeView::keySequenceCopyPressed, this, &EcalmonTreeWidget::copySelectedRowToClipboard);
}

EcalmonTreeWidget::~EcalmonTreeWidget()
{}


////////////////////////////////////////////////////////////////////////////////
//// Configuration used by derived classes                                  ////
////////////////////////////////////////////////////////////////////////////////

void EcalmonTreeWidget::setModel(GroupTreeModel* model)
{
  group_tree_model_ = model;
  filter_proxy_model_->setSourceModel(model);
}

void EcalmonTreeWidget::setAdditionalProxyModel(QSortFilterProxyModel* proxy_model)
{
  additional_proxy_model_ = proxy_model;

  additional_proxy_model_->setSourceModel(group_tree_model_);
  filter_proxy_model_    ->setSourceModel(additional_proxy_model_);
}

void EcalmonTreeWidget::setFilterColumns(const QVector<int>& columns)
{
  Q_ASSERT(group_tree_model_); // If the group tree model has not been set, yet, we are not able to get the column names for displaying them in the combobox

  filter_columns_ = columns;

  // Create the default filter, that uses all available columns
  filter_combobox_model_->clear();
  QStandardItem* all_columns_item = new QStandardItem("*");
  all_columns_item->setData(-1, Qt::ItemDataRole::UserRole);
  filter_combobox_model_->appendRow(all_columns_item);
  
  // Create individual column filters
  for (int column : filter_columns_)
  {
    QStandardItem* item = new QStandardItem(group_tree_model_->headerData(column, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString());
    item->setData(column, Qt::ItemDataRole::UserRole);
    filter_combobox_model_->appendRow(item);
  }
}

void EcalmonTreeWidget::setGroupSettings(const QList<GroupSetting>& preconfigured_groups, const QList<int>& group_enabled_columns)
{
  Q_ASSERT(group_tree_model_);

  group_by_enabled_columns_ = group_enabled_columns;

  group_by_combobox_model_->clear();

  // Add preconfigured groups
  for (auto& setting : preconfigured_groups)
  {
    QStandardItem* item = new QStandardItem(setting.name);
    QList<QVariant> qvariant_column_list;
    for (int column : setting.group_by_columns)
    {
      qvariant_column_list.push_back(column);
    }
    item->setData(qvariant_column_list, GROUP_BY_COLUMN_LIST_ROLE);
    item->setData(setting.auto_expand,  GROUP_BY_AUTOEXPAND_ROLE);
    item->setData(false,                GROUP_BY_CUSTOM_ROLE);
    group_by_combobox_model_->appendRow(item);
  }

  QStandardItem* custom_item = new QStandardItem("Custom...");
  custom_item->setData(true, GROUP_BY_CUSTOM_ROLE);
  group_by_combobox_model_->appendRow(custom_item);
}

void EcalmonTreeWidget::setVisibleColumns(const QVector<int>& columns)
{
  Q_ASSERT(group_tree_model_);

  for (int i = 0; i < group_tree_model_->columnCount(); i++)
  {
    ui_.tree_view->setColumnHidden(i, !columns.contains(i));
  }
}

void EcalmonTreeWidget::setDefaultForcedColumn(int column)
{
  Q_ASSERT(group_tree_model_);

  default_forced_column_ = column;
  if (group_tree_model_->groupByColumns().empty())
  {
    ui_.tree_view->setColumnHidden(default_forced_column_, false);
    ui_.tree_view->setForcedColumns(QVector<int> {default_forced_column_});
  }
  else
  {
    ui_.tree_view->setColumnHidden(0, false);
    ui_.tree_view->setForcedColumns(QVector<int> { 0 });
  }
}


////////////////////////////////////////////////////////////////////////////////
//// Slots controlled by the main application                               ////
////////////////////////////////////////////////////////////////////////////////

void EcalmonTreeWidget::monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb)
{
  emit topicsUpdated(monitoring_pb);
  if (group_tree_model_)
  {
    group_tree_model_->monitorUpdated(monitoring_pb);
  }
}

void EcalmonTreeWidget::setAlternatingRowColors(bool enable)
{
  ui_.tree_view->setAlternatingRowColors(enable);
}


////////////////////////////////////////////////////////////////////////////////
//// Tree View control                                                      ////
////////////////////////////////////////////////////////////////////////////////

void EcalmonTreeWidget::groupByCustom()
{
  std::map<int, QString> column_map;
  for (auto column : group_by_enabled_columns_)
  {
    column_map[(int)column] = group_tree_model_->headerData((int)column, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString();
  }

  ColumnSelectorDialog dialog(column_map, this);
  dialog.setSelectedColumns(group_tree_model_->groupByColumns());

  int initial_auto_expand_depth = ui_.tree_view->autoExpandDepth();
  if (initial_auto_expand_depth < -1)
    initial_auto_expand_depth = -1;
  dialog.setAutoExpandDepth(initial_auto_expand_depth);

  int result = dialog.exec();
  if (result == QDialog::DialogCode::Accepted)
  {
    int auto_expand_depth = dialog.getAutoExpandDepth();
    ui_.tree_view->setAutoExpandDepth(auto_expand_depth);

    QVector<int> group_by_columns = dialog.getSelectedColumns();
    groupByColumns(group_by_columns);
  }
  ui_.group_by_combobox->blockSignals(true);
  ui_.group_by_combobox->setCurrentIndex(-1);
  ui_.group_by_combobox->blockSignals(false);
}

void EcalmonTreeWidget::groupByColumns(const QVector<int>& columns)
{
  // Restore previously autohidden columns
  for (const auto& col : group_tree_model_->groupByColumns())
  {
    if (autohidden_columns_visibility.contains(col))
    {
      ui_.tree_view->setColumnHidden(col, !autohidden_columns_visibility[col]);
    }
    else
    {
      ui_.tree_view->setColumnHidden(col, false);
    }
  }
  // Save status of currently hidden / visibile columns that will be autohidden
  autohidden_columns_visibility.clear();
  for (int col : columns)
  {
    autohidden_columns_visibility[col] = !(ui_.tree_view->isColumnHidden(col));
  }
  // Autohide columns that will appear in the group-column anyways
  for (int col : columns)
  {
    ui_.tree_view->hideColumn(col);
  }

  // Set The forced column
  if (columns.empty())
  {
    ui_.tree_view->setColumnHidden(default_forced_column_, false);
    ui_.tree_view->setColumnHidden(0, true);
    ui_.tree_view->setForcedColumns(QVector<int> {default_forced_column_});
  }
  else
  {
    ui_.tree_view->setColumnHidden(0, false);
    ui_.tree_view->setForcedColumns(QVector<int> {0});
  }

  // Set the root-decoration
  ui_.tree_view->setRootIsDecorated(!columns.empty());

  // Actually apply the grouping
  group_tree_model_->setGroupByColumns(columns);
}

QModelIndex EcalmonTreeWidget::mapToSource(const QModelIndex& proxy_index) const
{
  Q_ASSERT(group_tree_model_);

  if (additional_proxy_model_)
  {
    return additional_proxy_model_->mapToSource(filter_proxy_model_->mapToSource(proxy_index));
  }
  else
  {
    return filter_proxy_model_->mapToSource(proxy_index);
  }
}

QModelIndex EcalmonTreeWidget::mapFromSource(const QModelIndex& source_index) const
{
  Q_ASSERT(group_tree_model_);

  if (additional_proxy_model_)
  {
    return filter_proxy_model_->mapFromSource(additional_proxy_model_->mapFromSource(source_index));
  }
  else
  {
    return filter_proxy_model_->mapFromSource(source_index);
  }
}

QList<QAbstractTreeItem*> EcalmonTreeWidget::getSelectedItems() const
{
  QList<QAbstractTreeItem*> selected_items;
  auto selected_rows = ui_.tree_view->selectionModel()->selectedRows(0);
  for (auto& index : selected_rows)
  {
    selected_items.push_back(group_tree_model_->item(mapToSource(index)));
  }
  return selected_items;
}


////////////////////////////////////////////////////////////////////////////////
//// Context Menu                                                           ////
////////////////////////////////////////////////////////////////////////////////

void EcalmonTreeWidget::contextMenu(const QPoint &pos)
{
  QMenu context_menu(this);

  fillContextMenu(context_menu, getSelectedItems());

  context_menu.exec(ui_.tree_view->viewport()->mapToGlobal(pos));
}

void EcalmonTreeWidget::fillContextMenu(QMenu& menu, const QList<QAbstractTreeItem*>& selected_items)
{
  QAbstractTreeItem* item = nullptr;
  for (QAbstractTreeItem* selected_item : selected_items)
  {
    // get the first non-group item
    if (!(selected_item->type() == (int)TreeItemType::Group))
    {
      item = selected_item;
      break;
    }
  }

  if (item)
  {
    int current_column = ui_.tree_view->currentIndex().column();
    QString current_column_header = ui_.tree_view->model()->headerData(current_column, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString();

    QAction* copy_current_column_action = new QAction(tr("Copy element (") + current_column_header + ")", &menu);
    QAction* copy_everything_action = new QAction(tr("Copy row"), &menu);

    connect(copy_current_column_action, &QAction::triggered, this, &EcalmonTreeWidget::copyCurrentIndexToClipboard);
    connect(copy_everything_action, &QAction::triggered, this, &EcalmonTreeWidget::copySelectedRowToClipboard);

    menu.addAction(copy_current_column_action);
    menu.addAction(copy_everything_action);
  }
}

void EcalmonTreeWidget::copyCurrentIndexToClipboard()
{
  QAbstractTreeItem* item = nullptr;
  for (QAbstractTreeItem* selected_item : getSelectedItems())
  {
    // get the first non-group item
    if (!(selected_item->type() == (int)TreeItemType::Group))
    {
      item = selected_item;
      break;
    }
  }
  if (item)
  {
    QModelIndex current_source_index = mapToSource(ui_.tree_view->currentIndex());
    QModelIndex selected_proxy_index = mapFromSource(group_tree_model_->index(item, current_source_index.column()));

    QVariant variant_data = ui_.tree_view->model()->data(selected_proxy_index, Qt::ItemDataRole::DisplayRole);

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(QtUtil::variantToString(variant_data));
  }
}

void EcalmonTreeWidget::copySelectedRowToClipboard()
{
  QAbstractTreeItem* item = nullptr;
  for (QAbstractTreeItem* selected_item : getSelectedItems())
  {
    // get the first non-group item
    if (!(selected_item->type() == (int)TreeItemType::Group))
    {
      item = selected_item;
      break;
    }
  }

  if (item)
  {
    QString clipboard_string;

    for (int column = 0; column < ui_.tree_view->model()->columnCount(); column++)
    {
      if (!ui_.tree_view->isColumnHidden(column))
      {
        QModelIndex index = mapFromSource(group_tree_model_->index(item, column));
        QVariant variant_data = ui_.tree_view->model()->data(index, Qt::ItemDataRole::DisplayRole);
        if (!clipboard_string.isEmpty())
        {
          clipboard_string += '\t';
        }
        clipboard_string += QtUtil::variantToString(variant_data);
      }
    }

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(clipboard_string);
  }
}


////////////////////////////////////////////////////////////////////////////////
//// Save Layout                                                            ////
////////////////////////////////////////////////////////////////////////////////

void EcalmonTreeWidget::saveGuiSettings(const QString& group)
{
  Q_ASSERT(group_tree_model_);

  QSettings settings;
  settings.beginGroup(group);
  settings.setValue("tree_state", ui_.tree_view->saveState(EcalmonGlobals::Version()));
  settings.setValue("group_by_index", ui_.group_by_combobox->currentIndex());
  QList<QVariant> group_by_columns;
  for (int column : group_tree_model_->groupByColumns())
  {
    group_by_columns.push_back(column);
  }
  settings.setValue("group_by_columns", group_by_columns);
  settings.setValue("group_by_columns_autoexpand", ui_.tree_view->autoExpandDepth());

  settings.setValue("show_all", ui_.show_all_checkbox->isChecked());

  settings.endGroup();
}

void EcalmonTreeWidget::loadGuiSettings(const QString& group)
{
  Q_ASSERT(group_tree_model_);

  QSettings settings;

  settings.beginGroup(group);

  // Tree View
  ui_.tree_view->restoreState(settings.value("tree_state").toByteArray() , EcalmonGlobals::Version());

  // Group By Settings
  QVariant group_by_index_variant = settings.value("group_by_index");
  if (group_by_index_variant.isValid())
  {
    int group_by_index = group_by_index_variant.toInt();
    if (group_by_index < 0 || group_by_index >= (ui_.group_by_combobox->model()->rowCount() - 1))
    {
      // Custom Group by
      ui_.group_by_combobox->setCurrentIndex(-1);

      QVector<int> group_by_columns;
      int auto_expand = 0;
      for (auto column_variant : settings.value("group_by_columns").toList())
      {
        int column = column_variant.toInt();
        if ((column >= 0) && (column < group_tree_model_->columnCount()))
        {
          group_by_columns.push_back(column);
        }
      }
      auto_expand = settings.value("group_by_columns_autoexpand").toInt();

      groupByColumns(group_by_columns);
      ui_.tree_view->setAutoExpandDepth(auto_expand);
    }
    else
    {
      // Predefined Group by
      ui_.group_by_combobox->setCurrentIndex(group_by_index);
    }
  }

  // Show all Checkbox
  QVariant show_all_variant = settings.value("show_all");
  if (show_all_variant.isValid())
  {
    ui_.show_all_checkbox->setChecked(show_all_variant.toBool());
  }

  settings.endGroup();

  autohidden_columns_visibility.clear();
}

void EcalmonTreeWidget::saveInitialState()
{
  initial_tree_state_ = ui_.tree_view->saveState();
  initial_group_by_index_ = ui_.group_by_combobox->currentIndex();
  initial_show_all_checkbox_state_ = ui_.show_all_checkbox->isChecked();
}

void EcalmonTreeWidget::resetLayout()
{
  ui_.group_by_combobox->setCurrentIndex(initial_group_by_index_);
  ui_.show_all_checkbox->setChecked(initial_show_all_checkbox_state_);
  autohidden_columns_visibility.clear();
  ui_.tree_view->restoreState(initial_tree_state_);

  ui_.filter_lineedit->clear();
  if (filter_combobox_model_->rowCount(QModelIndex()) > 0)
  {
    ui_.filter_combobox->setCurrentIndex(0);
  }
}

