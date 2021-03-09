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

#include "log_widget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QMenu>
#include <QClipboard>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // disable QTextStream warnings
#endif
#include <QTextStream>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <ecal/ecal.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "widgets/models/item_data_roles.h"

#include "ecalmon_globals.h"
#include "util.h"

LogWidget::LogWidget(QWidget *parent)
  : QWidget(parent)
  , log_update_time_milliseconds_(100)
  , parse_time_enabled_(true)
{
  ui_.setupUi(this);

  // Tree Model
  log_model_ = new LogModel(this);
  log_proxy_model_ = new LogSortFilterProxyModel(this);
  log_proxy_model_->setFilterRole(ItemDataRoles::FilterRole);
  log_proxy_model_->setSortRole(ItemDataRoles::SortRole);
  log_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  log_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  updateLogLevelFilter();

  log_proxy_model_->setSourceModel(log_model_);

  ui_.log_tree->setModel(log_proxy_model_);

  QVector<int> default_visible_columns
  {
    LogModel::Columns::TIME,
    LogModel::Columns::LOG_LEVEL,
    LogModel::Columns::PROCESS_NAME,
    LogModel::Columns::MESSAGE,
  };
  for (int i = 0; i < log_model_->columnCount(QModelIndex()); i++)
  {
    ui_.log_tree->setColumnHidden(i, !default_visible_columns.contains(i));
  }

  // Log Level Checkboxes
  connect(ui_.debug_checkbox,   &QCheckBox::stateChanged, this, &LogWidget::updateLogLevelFilter);
  connect(ui_.info_checkbox,    &QCheckBox::stateChanged, this, &LogWidget::updateLogLevelFilter);
  connect(ui_.warning_checkbox, &QCheckBox::stateChanged, this, &LogWidget::updateLogLevelFilter);
  connect(ui_.error_checkbox,   &QCheckBox::stateChanged, this, &LogWidget::updateLogLevelFilter);
  connect(ui_.fatal_checkbox,   &QCheckBox::stateChanged, this, &LogWidget::updateLogLevelFilter);

  // Pause button
  connect(ui_.pause_button, &QPushButton::toggled, this, &LogWidget::setPaused);

  // Clear Button
  connect(ui_.clear_button, &QPushButton::clicked, this, &LogWidget::clearLog);

  // Resize Columns Button
  connect(ui_.resize_columns_button, &QPushButton::clicked,
      [this]()
      {
        for (int i = 0; i < ui_.log_tree->model()->columnCount(); i++)
        {
          if (!ui_.log_tree->isColumnHidden(i))
          {
            ui_.log_tree->resizeColumnToContents(i);
          }
        }
      });

  // Save button
  connect(ui_.save_button, &QPushButton::clicked, this, &LogWidget::saveLogAs);


  // Filter Combobox
  filter_combobox_model_ = new QStandardItemModel(this);
  QStandardItem* filter_all_item = new QStandardItem();
  filter_all_item->setData("*", Qt::ItemDataRole::DisplayRole);
  filter_all_item->setData(-1, Qt::ItemDataRole::UserRole);
  filter_combobox_model_->appendRow(filter_all_item);
  for (int filter_column : filter_columns_)
  {
    QStandardItem* filter_item = new QStandardItem();
    filter_item->setData(log_model_->headerData(filter_column, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole), Qt::ItemDataRole::DisplayRole);
    filter_item->setData(filter_column, Qt::ItemDataRole::UserRole);
    filter_combobox_model_->appendRow(filter_item);
  }
  connect(ui_.filter_combobox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      [this](int index)
      {
        int filter_column = filter_combobox_model_->item(index)->data(Qt::ItemDataRole::UserRole).toInt();
        if (filter_column == -1)
        {
          log_proxy_model_->setFilterKeyColumns(filter_columns_);
        }
        else
        {
          log_proxy_model_->setFilterKeyColumn(filter_column);
        }
      });

  ui_.filter_combobox->setModel(filter_combobox_model_);

  // Filter Lineedit
  connect(ui_.filter_lineedit, &QLineEdit::textChanged, [this](const QString& text) {log_proxy_model_->setFilterFixedString(text); });

  log_update_timer_ = new QTimer(this);
  connect(log_update_timer_, &QTimer::timeout, this, &LogWidget::getEcalLogs);
  log_update_timer_->start(log_update_time_milliseconds_);

  // Set a nicer icon for the filter lineedit
  ui_.filter_lineedit->setClearIcon(QIcon(":/ecalicons/FILTER_CANCEL"));

  // Context menu
  ui_.log_tree->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.log_tree, &QTreeView::customContextMenuRequested, this, &LogWidget::contextMenu);

  connect(ui_.log_tree, &QAdvancedTreeView::keySequenceCopyPressed, this, &LogWidget::copySelectedRows);

  saveInitialState();

  loadGuiSettings();
}

LogWidget::~LogWidget()
{
  saveGuiSettings();
}


void LogWidget::getEcalLogs()
{
  eCAL::pb::Logging logging;
  std::string     logging_string;

  if (eCAL::Monitoring::GetLogging(logging_string))
  {
    logging.ParseFromString(logging_string);
  }
  else
  {
    return;
  }

  log_model_->insertLogs(logging);

  if (ui_.autoscroll_checkbox->isChecked())
  {
    ui_.log_tree->scrollToBottom();
  }

  ui_.message_counter_label->setText(QString::number(log_model_->rowCount()) + " entries");
}

void LogWidget::updateLogLevelFilter()
{
  int log_level_filter = eCAL_Logging_eLogLevel::log_level_none;
  
  if (ui_.debug_checkbox->isChecked())
  {
    log_level_filter |= (eCAL_Logging_eLogLevel::log_level_debug1 | eCAL_Logging_eLogLevel::log_level_debug2 | eCAL_Logging_eLogLevel::log_level_debug3 | eCAL_Logging_eLogLevel::log_level_debug4);
  }
  if (ui_.info_checkbox->isChecked())
  {
    log_level_filter |= eCAL_Logging_eLogLevel::log_level_info;
  }
  if (ui_.warning_checkbox->isChecked())
  {
    log_level_filter |= eCAL_Logging_eLogLevel::log_level_warning;
  }
  if (ui_.error_checkbox->isChecked())
  {
    log_level_filter |= eCAL_Logging_eLogLevel::log_level_error;
  }
  if (ui_.fatal_checkbox->isChecked())
  {
    log_level_filter |= eCAL_Logging_eLogLevel::log_level_fatal;
  }

  log_proxy_model_->setLogLevelFilter((eCAL_Logging_eLogLevel)log_level_filter);
}

void LogWidget::setPaused(bool paused)
{
  ui_.pause_button->blockSignals(true);

  if (paused && log_update_timer_->isActive())
  {
    log_update_timer_->stop();
    ui_.pause_button->setText("Resume");
    if (!ui_.pause_button->isChecked())
      ui_.pause_button->setChecked(true);
  }
  else
  {
    log_update_timer_->start(log_update_time_milliseconds_);
    ui_.pause_button->setText("Pause");
    if (ui_.pause_button->isChecked())
      ui_.pause_button->setChecked(false);
  }

  ui_.pause_button->blockSignals(false);

  emit this->paused(paused);
}

bool LogWidget::isPaused() const
{
  return ui_.pause_button->isChecked();
}

void LogWidget::setPollSpeed(int milliseconds)
{
  log_update_time_milliseconds_ = milliseconds;
  if (!isPaused())
  {
    log_update_timer_->start(log_update_time_milliseconds_);
  }
}


void LogWidget::setParseTimeEnabled(bool enabled)
{
  parse_time_enabled_ = enabled;
  log_model_->setParseTimeEnabled(enabled);
}

bool LogWidget::isParseTimeEnabled() const
{
  return parse_time_enabled_;
}

void LogWidget::clearLog()
{
  log_model_->clear();
  ui_.message_counter_label->setText("0 entries");
}

void LogWidget::saveLogAs()
{
  QSettings settings;
  settings.beginGroup("log_widget");
  QVariant last_log_file_dir_variant = settings.value("last_log_file_dir");
  QString start_dir;
  if (last_log_file_dir_variant.isValid())
  {
    start_dir = last_log_file_dir_variant.toString() + "/log.csv";
  }
  else
  {
    start_dir = "log.csv";
  }
  settings.endGroup();

  QString selected_file = QFileDialog::getSaveFileName(this, tr("Select log file"), start_dir, tr("CSV Files (*.csv);;All Files (*)"));
  
  if (!selected_file.isEmpty())
  {
    QFileInfo file_info = QFileInfo(selected_file);
    settings.beginGroup("log_widget");
    settings.setValue("last_log_file_dir", file_info.absoluteDir().path());
    settings.endGroup();

    if (log_model_->dumpToCsv(selected_file))
    {
      int rows = log_model_->rowCount(QModelIndex());
      QMessageBox info_message(
        QMessageBox::Icon::Information
        , tr("Saved")
        , tr("Saved ") + QString::number(rows) + " entries to \"" + selected_file + "\""
        , QMessageBox::Button::Ok
        , this);
      info_message.exec();
    }
    else
    {
      QMessageBox error_message(
        QMessageBox::Icon::Critical
        , tr("Error")
        , tr("Failed to save Log to file \"") + selected_file + "\""
        , QMessageBox::Button::Ok
        , this);
      error_message.exec();
    }
  }
}

void LogWidget::saveGuiSettings()
{
  QSettings settings;
  settings.beginGroup("log_widget");

  settings.setValue("log_tree", ui_.log_tree->saveState(EcalmonGlobals::Version()));
  settings.setValue("log_level_filter", (int)log_proxy_model_->logLevelFilter());

  settings.endGroup();
}

void LogWidget::loadGuiSettings()
{
  QSettings settings;

  settings.beginGroup("log_widget");

  ui_.log_tree->restoreState(settings.value("log_tree").toByteArray() , EcalmonGlobals::Version());
  
  QVariant log_level_filter_variant = settings.value("log_level_filter");
  if (log_level_filter_variant.isValid())
  {
    int log_level_filter = log_level_filter_variant.toInt();
    setLogLevelFilter(log_level_filter);
  }

  settings.endGroup();
}


void LogWidget::saveInitialState()
{
  initial_tree_state_ = ui_.log_tree->saveState();
  initial_log_level_filter_ = log_proxy_model_->logLevelFilter();
  initial_auto_scroll_ = ui_.autoscroll_checkbox->isChecked();
}

void LogWidget::resetLayout()
{
  ui_.log_tree->restoreState(initial_tree_state_);
  setLogLevelFilter(initial_log_level_filter_);
  ui_.autoscroll_checkbox->setChecked(initial_auto_scroll_);
  ui_.filter_lineedit->clear();
  ui_.filter_combobox->setCurrentIndex(0);
}

void LogWidget::setLogLevelFilter(int log_level)
{
  ui_.debug_checkbox  ->blockSignals(true);
  ui_.info_checkbox   ->blockSignals(true);
  ui_.warning_checkbox->blockSignals(true);
  ui_.error_checkbox  ->blockSignals(true);
  ui_.fatal_checkbox  ->blockSignals(true);

  ui_.debug_checkbox  ->setChecked(log_level & (eCAL_Logging_eLogLevel::log_level_debug1 | eCAL_Logging_eLogLevel::log_level_debug2 | eCAL_Logging_eLogLevel::log_level_debug3 | eCAL_Logging_eLogLevel::log_level_debug4));
  ui_.info_checkbox   ->setChecked(log_level & eCAL_Logging_eLogLevel::log_level_info);
  ui_.warning_checkbox->setChecked(log_level & eCAL_Logging_eLogLevel::log_level_warning);
  ui_.error_checkbox  ->setChecked(log_level & eCAL_Logging_eLogLevel::log_level_error);
  ui_.fatal_checkbox  ->setChecked(log_level & eCAL_Logging_eLogLevel::log_level_fatal);

  updateLogLevelFilter();

  ui_.debug_checkbox  ->blockSignals(false);
  ui_.info_checkbox   ->blockSignals(false);
  ui_.warning_checkbox->blockSignals(false);
  ui_.error_checkbox  ->blockSignals(false);
  ui_.fatal_checkbox  ->blockSignals(false);
}

void LogWidget::contextMenu(const QPoint &pos)
{
  auto selected_proxy_rows = ui_.log_tree->selectionModel()->selectedRows(LogModel::Columns::TIME);
  if (selected_proxy_rows.size() > 0)
  {
    QMenu context_menu(this);

    QAction* copy_rows_action = new QAction(tr("Copy"), &context_menu);

    connect(copy_rows_action, &QAction::triggered, this, &LogWidget::copySelectedRows);

    context_menu.addAction(copy_rows_action);

    context_menu.exec(ui_.log_tree->viewport()->mapToGlobal(pos));
  }
}

void LogWidget::copySelectedRows()
{
  auto selected_proxy_rows = ui_.log_tree->selectionModel()->selectedRows(LogModel::Columns::TIME);
  
  QString output_string;

  for (auto& proxy_row : selected_proxy_rows)
  {
    bool first_element = true;
    auto ui_log_tree_model = ui_.log_tree->model();
    for (int column = 0; column < ui_log_tree_model->columnCount(); column++)
    {
      if (!ui_.log_tree->isColumnHidden(column))
      {
        QModelIndex current_index = ui_log_tree_model->index(proxy_row.row(), column, proxy_row.parent());
        QVariant data_variant = ui_log_tree_model->data(current_index, Qt::ItemDataRole::DisplayRole);

        if (!first_element)
        {
          output_string += '\t';
        }
        first_element = false;

        output_string += QtUtil::variantToString(data_variant);
      }
    }
    output_string += "\r\n";
  }

  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(output_string);
}