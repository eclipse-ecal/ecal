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

#include "scenario_widget.h"

#include <QItemSelectionModel>
#include <QSettings>
#include <QHeaderView>
#include <QMenu>

#include <widgets/models/item_data_roles.h>

ScenarioWidget::ScenarioWidget(QWidget *parent)
  : QWidget(parent)
  , first_show_event_(true)
{
  ui_.setupUi(this);

  // Tree Model
  scenario_tree_model_ = new ScenarioModel(this);

  scenario_tree_proxy_model_ = new QStableSortFilterProxyModel(this);
  scenario_tree_proxy_model_->setSortRole             (ItemDataRoles::SortRole);
  scenario_tree_proxy_model_->setFilterRole           (ItemDataRoles::FilterRole);
  scenario_tree_proxy_model_->setSortCaseSensitivity  (Qt::CaseSensitivity::CaseInsensitive);
  scenario_tree_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  scenario_tree_proxy_model_->setFilterKeyColumn      (0);
  scenario_tree_proxy_model_->setSourceModel          (scenario_tree_model_);

  ui_.scenario_treeview->setModel(scenario_tree_proxy_model_);
  ui_.scenario_treeview->sortByColumn((int)ScenarioModel::Column::SCENARIO_TIMESTAMP, Qt::SortOrder::AscendingOrder);

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  ui_.scenario_treeview->header()->setFirstSectionMovable(true);
#endif

  edit_button_delegate_ = new EditButtonDelegate(ui_.scenario_treeview);
  ui_.scenario_treeview->setItemDelegate(edit_button_delegate_);
  connect(edit_button_delegate_, &EditButtonDelegate::buttonStateChanged, scenario_tree_model_,
          [this](const QModelIndex& index)
          {
            scenario_tree_model_->triggerRepaint(scenario_tree_proxy_model_->mapToSource(index));
          });

  connect(edit_button_delegate_, &EditButtonDelegate::buttonClicked, ui_.scenario_treeview,
          [this](const QModelIndex& index)
          {
            ui_.scenario_treeview->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
            ui_.scenario_treeview->edit(index);
          });

  ui_.scenario_treeview->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.scenario_treeview, &QAbstractItemView::customContextMenuRequested, this, &ScenarioWidget::openScenarioContextMenu);

  connect(scenario_tree_model_, &QAbstractItemModel::rowsInserted, this, &ScenarioWidget::promoteScenariosToQEcalPlay);
  connect(scenario_tree_model_, &QAbstractItemModel::rowsRemoved,  this, &ScenarioWidget::promoteScenariosToQEcalPlay);
  connect(scenario_tree_model_, &QAbstractItemModel::dataChanged,  this, &ScenarioWidget::promoteScenariosToQEcalPlay);

  // Connect QEcalPlay -> this
  connect(QEcalPlay::instance(), &QEcalPlay::measurementLoadedSignal, this, &ScenarioWidget::measurementLoaded);
  connect(QEcalPlay::instance(), &QEcalPlay::measurementClosedSignal, this, &ScenarioWidget::measurementClosed);

  // Button
  connect(ui_.scenario_treeview->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() { this->updateButtonRow(); });
  connect(QEcalPlay::instance(), &QEcalPlay::measurementClosedSignal, this, &ScenarioWidget::updateButtonRow);
  connect(QEcalPlay::instance(), &QEcalPlay::measurementLoadedSignal, this, &ScenarioWidget::updateButtonRow);
  connect(QEcalPlay::instance(), &QEcalPlay::scenariosChangedSignal,  this, &ScenarioWidget::updateButtonRow);
  connect(QEcalPlay::instance(), &QEcalPlay::scenariosSavedSignal,    this, &ScenarioWidget::updateButtonRow);

  connect(ui_.add_button,    &QAbstractButton::clicked, this, &ScenarioWidget::addScenario);
  connect(ui_.remove_button, &QAbstractButton::clicked, this, &ScenarioWidget::removeScenarios);
  connect(ui_.save_button, &QAbstractButton::clicked, QEcalPlay::instance(), []() { QEcalPlay::instance()->saveScenariosToDisk(); });

  // Jump to
  connect(ui_.jump_to_button,    &QPushButton::clicked,     this, &ScenarioWidget::jumpToSelectedScenario);
  connect(ui_.scenario_treeview, &QTreeView::doubleClicked, this, &ScenarioWidget::jumpToSelectedScenario);

  if (QEcalPlay::instance()->isMeasurementLoaded())
  {
    measurementLoaded(QEcalPlay::instance()->measurementPath());
  }

  updateButtonRow();
}

ScenarioWidget::~ScenarioWidget()
{
  saveLayout();
}

void ScenarioWidget::measurementLoaded(const QString& /*path*/)
{
  const auto measurement_boundaries_ = QEcalPlay::instance()->measurementBoundaries();
  const auto scenarios               = QEcalPlay::instance()->scenarios();

  scenario_tree_model_->setScenarios(scenarios, measurement_boundaries_);
  ui_.scenario_treeview->sortByColumn((int)ScenarioModel::Column::SCENARIO_TIMESTAMP, Qt::SortOrder::AscendingOrder);
}

void ScenarioWidget::measurementClosed()
{
  scenario_tree_model_->setScenarios({}, {});
  ui_.scenario_treeview->sortByColumn((int)ScenarioModel::Column::SCENARIO_TIMESTAMP, Qt::SortOrder::AscendingOrder);
}

std::vector<int> ScenarioWidget::selectedSourceRows() const
{
  std::vector<int> selected_source_rows;

  auto selected_proxy_rows = ui_.scenario_treeview->selectionModel()->selectedRows();
  selected_source_rows.reserve(selected_proxy_rows.size());

  for (const auto& proxy_index : selected_proxy_rows)
  {
    auto source_index = scenario_tree_proxy_model_->mapToSource(proxy_index);
    selected_source_rows.push_back(source_index.row());
  }

  return selected_source_rows;
}

void ScenarioWidget::jumpToSelectedScenario() const
{
  auto selected_source_rows = selectedSourceRows();
  if (selected_source_rows.size() == 1)
  {
    QEcalPlay::instance()->jumpTo(scenario_tree_model_->scenario(selected_source_rows[0]).time_);
  }
}

void ScenarioWidget::updateButtonRow()
{
  bool   measurement_loaded = QEcalPlay::instance()->isMeasurementLoaded();
  size_t selected_row_count = selectedSourceRows().size();

//  QModelIndex current_index = ui_.scenario_treeview->selectionModel()->currentIndex();

  if (!measurement_loaded)
  {
    ui_.add_button    ->setEnabled(false);
    ui_.remove_button ->setEnabled(false);
    ui_.save_button   ->setEnabled(false);
    ui_.jump_to_button->setEnabled(false);
  }
  else
  {
    ui_.add_button    ->setEnabled(true);
    ui_.remove_button ->setEnabled(selected_row_count >= 1);
    ui_.save_button   ->setEnabled(true);
    ui_.jump_to_button->setEnabled(selected_row_count == 1);
  }

  ui_.save_button->setEnabled(QEcalPlay::instance()->scenariosModified());
}

void ScenarioWidget::openEditor()
{
  QModelIndex current_index = ui_.scenario_treeview->selectionModel()->currentIndex();
  ui_.scenario_treeview->edit(current_index);
}

void ScenarioWidget::addScenario()
{
  auto current_pos = QEcalPlay::instance()->currentPlayState().current_frame_timestamp;
  int new_row = scenario_tree_model_->addScenario(EcalPlayScenario("Label", current_pos));
  QModelIndex proxy_index = scenario_tree_proxy_model_->mapFromSource(scenario_tree_model_->index(new_row, 0));

  ui_.scenario_treeview->selectionModel()->setCurrentIndex(proxy_index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
  ui_.scenario_treeview->edit(proxy_index);
}

void ScenarioWidget::removeScenarios()
{
  std::vector<int> selected_rows = selectedSourceRows();
  std::sort(selected_rows.begin(), selected_rows.end());
  
  for (auto selected_row_it = selected_rows.rbegin(); selected_row_it != selected_rows.rend(); ++selected_row_it)
  {
    scenario_tree_model_->removeScenario(*selected_row_it);
  }
}

void ScenarioWidget::promoteScenariosToQEcalPlay() const
{
  // TODO: Also react to the scenarioschanged signal!
  QEcalPlay::instance()->setScenarios(scenario_tree_model_->getScenarios());
}

void ScenarioWidget::openScenarioContextMenu(const QPoint& pos)
{
  QMenu context_menu;

  bool   measurement_loaded = QEcalPlay::instance()->isMeasurementLoaded();
  size_t selected_row_count = selectedSourceRows().size();

  QModelIndex current_index = ui_.scenario_treeview->selectionModel()->currentIndex();

  QAction* jump_to_action          = new QAction(                             "Jump to",       &context_menu);
  QAction* add_scenario_action     = new QAction(QIcon(":/ecalicons/ADD"),    "Add label",     &context_menu);
  QAction* remove_scenario_action  = new QAction(QIcon(":/ecalicons/REMOVE"), "Remove labels", &context_menu);
  QAction* edit_action             = new QAction(QIcon(":/ecalicons/EDIT"),   "Edit",          &context_menu);

  connect(jump_to_action,         &QAction::triggered, this, &ScenarioWidget::jumpToSelectedScenario);
  connect(add_scenario_action,    &QAction::triggered, this, &ScenarioWidget::addScenario);
  connect(remove_scenario_action, &QAction::triggered, this, &ScenarioWidget::removeScenarios);
  connect(edit_action,            &QAction::triggered, this, &ScenarioWidget::openEditor);

  context_menu.addAction(jump_to_action);
  context_menu.addSeparator();
  context_menu.addAction(edit_action);
  context_menu.addAction(add_scenario_action);
  context_menu.addAction(remove_scenario_action);

  jump_to_action        ->setEnabled(selected_row_count == 1);
  add_scenario_action   ->setEnabled(measurement_loaded);
  remove_scenario_action->setEnabled(selected_row_count > 0);
  edit_action           ->setEnabled(current_index.isValid());

  context_menu.exec(ui_.scenario_treeview->viewport()->mapToGlobal(pos));
}

void ScenarioWidget::showEvent(QShowEvent* /*event*/)
{
  if (first_show_event_)
  {
    // Auto-size the columns to some dummy data
    static const std::map<ScenarioModel::Column, QString> dummy_data
    {
      { ScenarioModel::Column::SCENARIO_NAME,      "extraordinary event ____" },
      { ScenarioModel::Column::SCENARIO_TIMESTAMP, "99999999.999" },
    };

    QFontMetrics tree_font_metrics   = ui_.scenario_treeview->fontMetrics();
    QFontMetrics header_font_metrics = ui_.scenario_treeview->header()->fontMetrics();

    int minimum_column_width = ui_.scenario_treeview->header()->minimumSectionSize();

    for (int column = 0; column < ui_.scenario_treeview->model()->columnCount(); column++)
    {
      auto dummy_it = dummy_data.find((ScenarioModel::Column)column);
      if (dummy_it != dummy_data.end())
      {
        // This calculation is far from exact, at some points is just relies on some magic numbers (for icon and frame sizes)
        int dummy_text_width = tree_font_metrics.boundingRect(dummy_it->second).width() + 4;
        int header_width     = header_font_metrics.boundingRect(ui_.scenario_treeview->model()->headerData(column, Qt::Orientation::Horizontal).toString()).width() + 4;

        ui_.scenario_treeview->header()->resizeSection(column, std::max(std::max(dummy_text_width, header_width), minimum_column_width));
      }
    }

    saveInitialLayout();

    restoreLayout();
  }
  first_show_event_ = false;
}

////////////////////////////////////////////////////////////////////////////////
//// Save Layout                                                            ////
////////////////////////////////////////////////////////////////////////////////

void ScenarioWidget::saveLayout()
{
  QSettings settings;
  settings.beginGroup("scenarios");
  settings.setValue("tree_state", ui_.scenario_treeview->saveState(EcalPlayGlobals::Version()));
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

  settings.endGroup();
}

void ScenarioWidget::saveInitialLayout()
{
  initial_tree_state_ = ui_.scenario_treeview->saveState();
}

void ScenarioWidget::resetLayout()
{
  ui_.scenario_treeview->restoreState(initial_tree_state_);
}
