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

#include "ScenarioWidget.h"

#include <QItemSelectionModel>
#include <QSettings>

ScenarioWidget::ScenarioWidget(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);

  // Tree Model
  scenario_tree_model_ = new QStandardTreeModel(this);
  scenario_tree_model_->setColumnCount(2);
  scenario_tree_model_->setHeaderData(0, Qt::Orientation::Horizontal, "Name", Qt::ItemDataRole::DisplayRole);
  scenario_tree_model_->setHeaderData(1, Qt::Orientation::Horizontal, "Position [s]", Qt::ItemDataRole::DisplayRole);

  scenario_tree_proxy_model_ = new QStableSortFilterProxyModel(this);
  scenario_tree_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  scenario_tree_proxy_model_->setSortRole((Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 1)); //-V1016
  scenario_tree_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  scenario_tree_proxy_model_->setFilterKeyColumn(0);
  scenario_tree_proxy_model_->setSourceModel(scenario_tree_model_);

  ui_.scenario_treeview->setModel(scenario_tree_proxy_model_);
  ui_.scenario_treeview->sortByColumn(1, Qt::SortOrder::AscendingOrder);

  // Initial layout
  autoSizeColumns();

  // Set the current index to the channel name for easier keyboard lookup
  connect(ui_.scenario_treeview->selectionModel(), &QItemSelectionModel::currentColumnChanged,
      [this](const QModelIndex& current)
      {
        if (current.column() != 0)
        {
          QModelIndex new_current_index = scenario_tree_proxy_model_->index(current.row(), 0);
          ui_.scenario_treeview->selectionModel()->setCurrentIndex(new_current_index, QItemSelectionModel::NoUpdate);
        }
      });

  // Connect QEcalPlay -> this
  connect(QEcalPlay::instance(), &QEcalPlay::measurementLoadedSignal, this, &ScenarioWidget::measurementLoaded);
  connect(QEcalPlay::instance(), &QEcalPlay::measurementClosedSignal, this, &ScenarioWidget::measurementClosed);

  // Button
  connect(ui_.scenario_treeview->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() {ui_.jump_to_button->setEnabled(selectedItems().size()); });

  // Jump to
  connect(ui_.jump_to_button,    &QPushButton::clicked,     this, &ScenarioWidget::jumpToSelectedScenario);
  connect(ui_.scenario_treeview, &QTreeView::doubleClicked, this, &ScenarioWidget::jumpToSelectedScenario);

  if (QEcalPlay::instance()->isMeasurementLoaded())
  {
    measurementLoaded(QEcalPlay::instance()->measurementPath());
  }

  saveInitialLayout();
  restoreLayout();
}

ScenarioWidget::~ScenarioWidget()
{
  saveLayout();
}

void ScenarioWidget::measurementLoaded(const QString& /*path*/)
{
  measurement_boundaries_ = QEcalPlay::instance()->measurementBoundaries();
  reloadScenarioTree();
}

void ScenarioWidget::measurementClosed()
{
  scenario_tree_model_->removeAllChildren();
}

void ScenarioWidget::reloadScenarioTree()
{
  scenario_tree_model_->removeAllChildren();

  QList<QAbstractTreeItem*> new_items;
  
  for (auto& scenario : QEcalPlay::instance()->scenarios())
  {
    QStandardTreeItem* scenario_item = new QStandardTreeItem();

    double second_since_start = std::chrono::duration_cast<std::chrono::duration<double>>(scenario.time_ - measurement_boundaries_.first).count();

    scenario_item->setData(0, scenario.name_.c_str(),                      Qt::ItemDataRole::DisplayRole);
    scenario_item->setData(0, scenario.name_.c_str(),                      (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 1)); //-V1016
    scenario_item->setData(1, QString::number(second_since_start, 'f', 3), Qt::ItemDataRole::DisplayRole);
    scenario_item->setData(1, second_since_start,                          (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 1)); //-V1016
    scenario_item->setData(1,                                              Qt::AlignmentFlag::AlignRight, Qt::ItemDataRole::TextAlignmentRole);

    new_items.push_back(scenario_item);
  }

  scenario_tree_model_->insertItems(new_items);

  ui_.scenario_treeview->sortByColumn(1, Qt::SortOrder::AscendingOrder);
}

QList<QStandardTreeItem*> ScenarioWidget::selectedItems() const
{
  QList<QStandardTreeItem*> selected_items;

  for (const auto& proxy_index : ui_.scenario_treeview->selectionModel()->selectedRows())
  {
    auto source_index = scenario_tree_proxy_model_->mapToSource(proxy_index);
    auto tree_item = scenario_tree_model_->item(source_index);
    if (tree_item)
    {
      selected_items.push_back(static_cast<QStandardTreeItem*>(tree_item));
    }
  }

  return selected_items;
}

void ScenarioWidget::jumpToSelectedScenario() const
{
  auto selected_items = selectedItems();
  if (selected_items.size() == 1)
  {
    double rel_seconds = selected_items[0]->data(1, (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 1)).toDouble(); //-V1016
    double offset = ui_.jump_to_offset_spinbox->value();
    std::chrono::nanoseconds relative_position = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(rel_seconds - offset));
    QEcalPlay::instance()->jumpTo(measurement_boundaries_.first + relative_position);
  }
}

void ScenarioWidget::autoSizeColumns()
{
  QStandardTreeItem* dummy_item = new QStandardTreeItem();
  dummy_item->setData(0, "extraordinary event _________", Qt::ItemDataRole::DisplayRole);
  dummy_item->setData(1, 999999.999, Qt::ItemDataRole::DisplayRole);

  scenario_tree_model_->insertItem(dummy_item);

  for (int i = 0; i < scenario_tree_model_->columnCount(); i++)
  {
    ui_.scenario_treeview->resizeColumnToContents(i);
  }
  scenario_tree_model_->removeItem(dummy_item);
}


////////////////////////////////////////////////////////////////////////////////
//// Save Layout                                                            ////
////////////////////////////////////////////////////////////////////////////////

void ScenarioWidget::saveLayout()
{
  QSettings settings;
  settings.beginGroup("scenarios");
  settings.setValue("tree_state", ui_.scenario_treeview->saveState(EcalPlayGlobals::Version()));
  settings.setValue("jump_to_offset", ui_.jump_to_offset_spinbox->value());
  settings.endGroup();
}

void ScenarioWidget::restoreLayout()
{
  QSettings settings;
  settings.beginGroup("scenarios");

  QVariant tree_state_variant = settings.value("tree_state");
  if (tree_state_variant.isValid())
  {
    ui_.scenario_treeview->restoreState(tree_state_variant.toByteArray(), EcalPlayGlobals::Version());
  }

  QVariant jump_to_offset_variant = settings.value("jump_to_offset");
  if (jump_to_offset_variant.isValid())
  {
    ui_.jump_to_offset_spinbox->setValue(jump_to_offset_variant.toDouble());
  }

  settings.endGroup();
}

void ScenarioWidget::saveInitialLayout()
{
  initial_tree_state_ = ui_.scenario_treeview->saveState();
}

void ScenarioWidget::resetLayout()
{
  ui_.scenario_treeview->restoreState(initial_tree_state_);
  ui_.jump_to_offset_spinbox->setValue(0.0);
}