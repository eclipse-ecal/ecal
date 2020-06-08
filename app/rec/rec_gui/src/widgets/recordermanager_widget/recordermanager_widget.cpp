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

#include "recordermanager_widget.h"

#include "qecalrec.h"

#include <rec_client_core/ecal_rec_defs.h>

#include <QString>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QMessageBox>
#include <QPixmap>
#include <QSettings>

#include <QFontMetrics>
#include <QFont>

#include <sstream>
#include <cmath>

#include "models/item_data_roles.h"
#include "recordermanager_add_dialog.h"

#include <ecal/ecal.h>


RecorderManagerWidget::RecorderManagerWidget(QWidget *parent)
  : QWidget(parent)
  , activate_button_state_is_ativate_(true)
  , first_show_event_(true)
{
  ui_.setupUi(this);

  // Warning label
  ui_.pre_buffer_warning_label->setPixmap(QPixmap(":/ecalicons/WARNING").scaled(ui_.pre_buffer_checkbox->sizeHint().height(), ui_.pre_buffer_checkbox->sizeHint().height(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
  connect(QEcalRec::instance(), &QEcalRec::connectedToEcalStateChangedSignal, this, &RecorderManagerWidget::updatePreBufferWarning);
  connect(QEcalRec::instance(), &QEcalRec::preBufferingEnabledChangedSignal,  this, &RecorderManagerWidget::updatePreBufferWarning);

  // Recorder model
  recorder_model_ = new RecorderModel(this);
  recorder_proxy_model_ = new QMulticolumnSortFilterProxyModel(this);
  recorder_proxy_model_->setSortRole             (ItemDataRoles::SortRole);
  recorder_proxy_model_->setFilterRole           (ItemDataRoles::FilterRole);
  recorder_proxy_model_->setFilterKeyColumn      ((int)RecorderModel::Columns::HOSTNAME);
  recorder_proxy_model_->setSortCaseSensitivity  (Qt::CaseSensitivity::CaseInsensitive);
  recorder_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  recorder_proxy_model_->setSourceModel          (recorder_model_);
  recorder_proxy_model_->setDynamicSortFilter    (false);

  if (QEcalRec::instance()->showDisabledElementsAtEnd())
    recorder_proxy_model_->setAlwaysSortedColumn((int)RecorderModel::Columns::ENABLED, Qt::SortOrder::DescendingOrder);
  else
    recorder_proxy_model_->setAlwaysSortedColumn(-1);

  ui_.recorder_list->setModel(recorder_proxy_model_);

  connect(QEcalRec::instance(), &QEcalRec::showDisabledElementsAtEndChanged, this
          , [this](bool show_at_end)
            {
              if (show_at_end)
                recorder_proxy_model_->setAlwaysSortedColumn((int)RecorderModel::Columns::ENABLED, Qt::SortOrder::DescendingOrder);
              else
                recorder_proxy_model_->setAlwaysSortedColumn(-1);
            });

  // Filter lineedit
  connect(ui_.filter_lineedit, &QLineEdit::textChanged, recorder_proxy_model_, &QSortFilterProxyModel::setFilterFixedString);

  // Tree Header
  tree_header_ = new QCheckboxHeaderView(ui_.recorder_list);
  tree_header_->setStretchLastSection(true);
  tree_header_->setCheckable((int)RecorderModel::Columns::ENABLED);
  ui_.recorder_list->setHeader(tree_header_);

  connect(tree_header_, &QHeaderView::sortIndicatorChanged, this
          , [this](int logical_index, Qt::SortOrder /*order*/)
            {
              // Dynamically re-sorting rapidly changing columns (like the buffer size) looks stupid and wastes CPU
              bool dynamic_sorting_allowed = 
                   (logical_index == static_cast<int>(RecorderModel::Columns::ENABLED))
                || (logical_index == static_cast<int>(RecorderModel::Columns::HOSTNAME))
                || (logical_index == static_cast<int>(RecorderModel::Columns::HOST_FILTER));
              recorder_proxy_model_->setDynamicSortFilter(dynamic_sorting_allowed);
            });

  ui_.recorder_list->sortByColumn((int) RecorderModel::Columns::HOSTNAME, Qt::SortOrder::AscendingOrder);

  // Treeview
  ui_.recorder_list->setForcedColumns({
          (int) RecorderModel::Columns::ENABLED
        , (int) RecorderModel::Columns::HOSTNAME
        , (int) RecorderModel::Columns::HOST_FILTER
  });

  ui_.recorder_list->setAlternatingRowColors(QEcalRec::instance()->alternatingRowColorsEnabled());
  connect(QEcalRec::instance(), &QEcalRec::alternatingRowColorsEnabledChanged, ui_.recorder_list, &QTreeView::setAlternatingRowColors);

  // Delegate for Host filter
  host_filter_delegate_ = new HostFilterDelegate(ui_.recorder_list);
  ui_.recorder_list->setItemDelegateForColumn((int)RecorderModel::Columns::HOST_FILTER, host_filter_delegate_);

  connect(QEcalRec::instance(), &QEcalRec::monitorUpdatedSignal, this,
      [this](const eCAL::rec_server::TopicInfoMap_T& /*topic_info_map*/, const eCAL::rec_server::HostsRunningEcalRec_T& hosts_running_ecal_rec)
      {
        std::set<QString> host_list;
        for (const auto& host : hosts_running_ecal_rec)
        {
          host_list.emplace(QString::fromStdString(host.first));
        }
        host_filter_delegate_->setMonitorHosts(host_list);
      });

  connect(QEcalRec::instance(), &QEcalRec::enabledRecClientsChangedSignal, this, &RecorderManagerWidget::setCurrentlyInUseHostsToDelegate);
  connect(QEcalRec::instance(), &QEcalRec::enabledRecClientsChangedSignal, this, [this]() {setCurrentlyInUseHostsToDelegate(QEcalRec::instance()->enabledRecClients()); });

  connect(ui_.recorder_list, &QTreeView::clicked, this
          , [this](const QModelIndex& index)
            {
              if (index.flags() & Qt::ItemFlag::ItemIsEditable)
              {
                ui_.recorder_list->edit(index);
              }
            });

  // Checkboxes (Tree Header & Show elmeents at the bottom)
  ui_.recorder_list->sortByColumn((int)RecorderModel::Columns::HOSTNAME, Qt::SortOrder::AscendingOrder);

  connect(tree_header_,    &QCheckboxHeaderView::clicked,             this, &RecorderManagerWidget::headerClicked);
  connect(recorder_model_, &QAbstractItemModel::dataChanged,          this, &RecorderManagerWidget::treeDataChanged);

  connect(recorder_model_, &QAbstractItemModel::rowsInserted,         this, &RecorderManagerWidget::treeRowsInserted);
  connect(recorder_model_, &QAbstractItemModel::rowsRemoved,          this, &RecorderManagerWidget::updateHeader);

  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal, this, &RecorderManagerWidget::updateHeader);


  // Context menu
  ui_.recorder_list->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.recorder_list, &QAbstractItemView::customContextMenuRequested, this, &RecorderManagerWidget::contextMenu);

  // Buttons
  connect(ui_.local_recording_button, &QAbstractButton::clicked, this, &RecorderManagerWidget::localRecordingButtonPressed);
  connect(ui_.add_button,             &QAbstractButton::clicked, this, &RecorderManagerWidget::addButtonClicked);
  connect(ui_.remove_button,          &QAbstractButton::clicked, this, &RecorderManagerWidget::removeSelectedRows);

  connect(QEcalRec::instance(),                &QEcalRec::recordModeChangedSignal,     this, &RecorderManagerWidget::updateRemoveButton);
  connect(ui_.recorder_list->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RecorderManagerWidget::updateRemoveButton);
  connect(recorder_model_,                     &QAbstractItemModel::dataChanged,       this, &RecorderManagerWidget::updateRemoveButton);

  connect(QEcalRec::instance(),                &QEcalRec::recordModeChangedSignal,     this, &RecorderManagerWidget::updateAddButton);

  connect(QEcalRec::instance(), &QEcalRec::enabledRecClientsChangedSignal, this, &RecorderManagerWidget::updateLocalRecordingButton);
  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,    this, &RecorderManagerWidget::updateLocalRecordingButton);

  // Buffering
  connect(ui_.pre_buffer_checkbox, &QCheckBox::toggled, QEcalRec::instance(), [](bool checked) {QEcalRec::instance()->setPreBufferingEnabled(checked); });
  connect(ui_.pre_buffer_spinbox, static_cast<void (QDoubleSpinBox:: *)(double)>(&QDoubleSpinBox::valueChanged), this, [](double val) {QEcalRec::instance()->setMaxPreBufferLength(std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(val))); });
  connect(QEcalRec::instance(), &QEcalRec::preBufferingEnabledChangedSignal, this, &RecorderManagerWidget::preBufferingEnabledChanged);
  connect(QEcalRec::instance(), &QEcalRec::maxPreBufferLengthChangedSignal, this, &RecorderManagerWidget::maxPreBufferLengthChanged);

  // Initial state
  updateAddButton();
  updateRemoveButton();
  updateLocalRecordingButton();
  preBufferingEnabledChanged(QEcalRec::instance()->preBufferingEnabled());
  maxPreBufferLengthChanged(QEcalRec::instance()->maxPreBufferLength());
  updateHeader();
  setCurrentlyInUseHostsToDelegate(QEcalRec::instance()->enabledRecClients());
  updatePreBufferWarning();
}

RecorderManagerWidget::~RecorderManagerWidget()
{
  saveLayout();
}

///////////////////////////////////
// Save / Restore layout
///////////////////////////////////

void RecorderManagerWidget::showEvent(QShowEvent * /*event*/)
{
  if (first_show_event_)
  {
    // Auto-size the columns to some dummy data
    static std::map<RecorderModel::Columns, std::pair<bool, QString>> dummy_data
    {
      { RecorderModel::Columns::ENABLED,      { true,  "" }},
      { RecorderModel::Columns::HOSTNAME,     { true,  "CARPC01 (CAN rec)__" }},
      { RecorderModel::Columns::HOST_FILTER,  { true,  "CARPC01__" }},
      { RecorderModel::Columns::TIMESTAMP,    { true,  "2020-02-22 22:22:22.222_" }},
      { RecorderModel::Columns::TIME_ERROR,   { true,  "999999.999" }},
      { RecorderModel::Columns::BUFFER,       { false, "999.9 s / 99999 frames" }},
      { RecorderModel::Columns::STATE,        { true,  "Subscribing (999 Topics)" }},
      { RecorderModel::Columns::INFO,         { true,  "No information available (This is a very long error message)" }},
    };

    QFontMetrics tree_font_metrics   = ui_.recorder_list->fontMetrics();
    QFontMetrics header_font_metrics = ui_.recorder_list->header()->fontMetrics();

    int minimum_column_width = ui_.recorder_list->header()->minimumSectionSize();
    
    for (int column = 0; column < recorder_proxy_model_->columnCount(); column++)
    {
      auto dummy_it = dummy_data.find((RecorderModel::Columns)column);
      if (dummy_it != dummy_data.end())
      {
        // This calculation is far from exact, at some points is just relies on some magic numbers (for icon and frame sizes)
        int dummy_text_width = tree_font_metrics.boundingRect(dummy_it->second.second).width() + 4;
        int icon_width       = (dummy_it->second.first ? 25 : 0);
        int header_width     = header_font_metrics.boundingRect(ui_.recorder_list->model()->headerData(column, Qt::Orientation::Horizontal).toString()).width() + 4;
        
        ui_.recorder_list->header()->resizeSection(column, std::max(std::max((dummy_text_width + icon_width), header_width), minimum_column_width));
      }
    }

    // Hide Columns that the user may not need that often
    static std::vector<RecorderModel::Columns> intial_colum_selection =
    {
      RecorderModel::Columns::ENABLED,
      RecorderModel::Columns::HOSTNAME,
      RecorderModel::Columns::HOST_FILTER,
      RecorderModel::Columns::STATE,
      RecorderModel::Columns::BUFFER,
      RecorderModel::Columns::TIMESTAMP,
      RecorderModel::Columns::INFO,
    };
    for (int col = 0; col < (int)RecorderModel::Columns::COLUMN_COUNT; col++)
    {
      if (std::find(intial_colum_selection.begin(), intial_colum_selection.end(), (RecorderModel::Columns)col) == intial_colum_selection.end())
      {
        ui_.recorder_list->hideColumn(col);
      }
    }

    saveInitialLayout();
    restoreLayout();
  }
  first_show_event_ = false;
}

void RecorderManagerWidget::saveLayout()
{
  QSettings settings;
  settings.setValue("recordermanager_list_state", ui_.recorder_list->saveState());
}

void RecorderManagerWidget::restoreLayout()
{
  QSettings settings;
  
  QVariant recorder_list_state_variant = settings.value("recordermanager_list_state");

  if (recorder_list_state_variant.isValid())
  {
    QByteArray recorder_list_state = recorder_list_state_variant.toByteArray();
    ui_.recorder_list->restoreState(recorder_list_state);
  }
}

void RecorderManagerWidget::saveInitialLayout()
{
  initial_recorder_list_state_ = ui_.recorder_list->saveState(eCAL::rec::Version());
}

void RecorderManagerWidget::resetLayout()
{
  ui_.recorder_list  ->restoreState(initial_recorder_list_state_, eCAL::rec::Version());
  ui_.filter_lineedit->clear();
}

////////////////////////////////////
/// Internal slots
////////////////////////////////////

void RecorderManagerWidget::preBufferingEnabledChanged(bool enabled)
{
  if (enabled != ui_.pre_buffer_checkbox->isChecked())
  {
    ui_.pre_buffer_checkbox->blockSignals(true);
    ui_.pre_buffer_checkbox->setChecked(enabled);
    ui_.pre_buffer_checkbox->blockSignals(false);
  }
  ui_.pre_buffer_spinbox->setEnabled(enabled);
}

void RecorderManagerWidget::maxPreBufferLengthChanged(std::chrono::steady_clock::duration max_buffer_length)
{
  double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(max_buffer_length).count();

  if (std::fabs(seconds - ui_.pre_buffer_spinbox->value()) > 0.01)
  {
    ui_.pre_buffer_spinbox->blockSignals(true);
    ui_.pre_buffer_spinbox->setValue(seconds);
    ui_.pre_buffer_spinbox->blockSignals(false);
  }
}

void RecorderManagerWidget::updatePreBufferWarning()
{
  ui_.pre_buffer_warning_label->setHidden(!QEcalRec::instance()->preBufferingEnabled() || QEcalRec::instance()->connectedToEcal());
}

void RecorderManagerWidget::headerClicked(int column, bool checked)
{

  if (column == (int)RecorderModel::Columns::ENABLED)
  {
    recorder_model_->setAllChecked(checked);
    QEcalRec::instance()->setEnabledRecClients(recorder_model_->enabledRecClients());

    if (!recorder_proxy_model_->dynamicSortFilter())
    {
      recorder_proxy_model_->invalidate();
    }
  }
}

void RecorderManagerWidget::updateHeader()
{
  bool some_checked(false);
  bool some_unchecked(false);

  for (int i = 0; i < recorder_model_->rowCount(); i++)
  {
    QModelIndex enabled_index = recorder_model_->index(i, (int)RecorderModel::Columns::ENABLED);
    bool is_checked = (recorder_model_->data(enabled_index, Qt::ItemDataRole::CheckStateRole) == Qt::CheckState::Checked);

    if (is_checked)
      some_checked = true;
    else
      some_unchecked = true;
  }

  if (some_checked && some_unchecked)
  {
    tree_header_->setCheckState((int)RecorderModel::Columns::ENABLED, Qt::CheckState::PartiallyChecked);
  }
  else if (some_checked && !some_unchecked)
  {
    tree_header_->setCheckState((int)RecorderModel::Columns::ENABLED, Qt::CheckState::Checked);
  }
  else
  {
    tree_header_->setCheckState((int)RecorderModel::Columns::ENABLED, Qt::CheckState::Unchecked);
  }

  tree_header_->setCheckboxEnabled(0, !QEcalRec::instance()->recording());
}

void RecorderManagerWidget::treeDataChanged(const QModelIndex &top_left, const QModelIndex &bottom_right, const QVector<int> &roles)
{
  if ((top_left.column() <= (int)RecorderModel::Columns::ENABLED) && ((int)RecorderModel::Columns::ENABLED <= bottom_right.column()))
  {
    if (roles.empty() || roles.contains(Qt::ItemDataRole::CheckStateRole))
    {
      updateHeader();

      if (!recorder_proxy_model_->dynamicSortFilter())
      {
        recorder_proxy_model_->invalidate();
      }
    }
  }
}

void RecorderManagerWidget::treeRowsInserted(const QModelIndex &/*parent*/, int /*first*/, int /*last*/)
{
  updateHeader();
  if (!recorder_proxy_model_->dynamicSortFilter())
  {
    recorder_proxy_model_->invalidate();
  }
}

void RecorderManagerWidget::localRecordingButtonPressed()
{
  auto enabled_recorders = QEcalRec::instance()->enabledRecClients();
  for (auto& recorder : enabled_recorders)
  {
    recorder.second.host_filter_ = std::set<std::string>{ recorder.first };
  }
  QEcalRec::instance()->setEnabledRecClients(enabled_recorders);
}

void RecorderManagerWidget::addButtonClicked()
{
  QString initially_selected_host;
  auto selected_rows = getSelectedRows();
  if (!selected_rows.empty())
  {
    initially_selected_host = QString::fromStdString(recorder_model_->getHostname(*selected_rows.begin()));
  }

  RecorderManagerAddDialog add_dialog(recorder_model_->getAllHosts(), true,  initially_selected_host, this);
  int button_id = add_dialog.exec();
  
  if (button_id == QDialog::Accepted)
  {
    if (add_dialog.addHostSelected())
    { 
      const QString& new_hostname = add_dialog.newHostName();

      if (new_hostname.isEmpty())
      {
        QMessageBox error_message(this);
        error_message.setIcon       (QMessageBox::Icon::Critical);
        error_message.setWindowTitle("Error");
        error_message.setText       ("Hostname is empty.");
        error_message.exec();
        return;
      }
      else if (recorder_model_->containsHost(new_hostname.toStdString()))
      {
        QMessageBox error_message(this);
        error_message.setIcon       (QMessageBox::Icon::Critical);
        error_message.setWindowTitle("Error");
        error_message.setText       ("The host already exists in the list.");
        error_message.exec();
      }
      else
      {
        if (recorder_model_->addHost(new_hostname.toStdString()))
        {
          QModelIndex index = recorder_model_->index(new_hostname.toStdString(), "", (int)RecorderModel::Columns::ENABLED);

          if (index.isValid())
          {
            ui_.recorder_list->scrollTo(recorder_proxy_model_->mapFromSource(index));
            ui_.recorder_list->selectionModel()->select(recorder_proxy_model_->mapFromSource(index), QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);

            if (!QEcalRec::instance()->recording())
            {
              recorder_model_->setData(index, Qt::CheckState::Checked, Qt::ItemDataRole::CheckStateRole);
            }
          }
        }
        else
        {
          QMessageBox error_message(this);
          error_message.setIcon       (QMessageBox::Icon::Critical);
          error_message.setWindowTitle("Error");
          error_message.setText       ("Failed adding host.");
          error_message.exec();
        }
      }
    }
    else if (add_dialog.addAddonSelected())
    { 
      const QString& hostname = add_dialog.newAddon().first;
      const QString& new_addon_id = add_dialog.newAddon().second;

      if (hostname.isEmpty())
      {
        QMessageBox error_message(this);
        error_message.setIcon       (QMessageBox::Icon::Critical);
        error_message.setWindowTitle("Error");
        error_message.setText       ("Hostname is empty.");
        error_message.exec();
        return;
      }
      else if (new_addon_id.isEmpty())
      {
        QMessageBox error_message(this);
        error_message.setIcon       (QMessageBox::Icon::Critical);
        error_message.setWindowTitle("Error");
        error_message.setText       ("Addon ID is empty.");
        error_message.exec();
        return;
      }
      else if (!recorder_model_->containsHost(hostname.toStdString()))
      {
        QMessageBox error_message(this);
        error_message.setIcon       (QMessageBox::Icon::Critical);
        error_message.setWindowTitle("Error");
        error_message.setText       ("The Selected hostname does not exist");
        error_message.exec();
      }
      else if (recorder_model_->containsAddon(hostname.toStdString(), new_addon_id.toStdString()))
      {
        QMessageBox error_message(this);
        error_message.setIcon       (QMessageBox::Icon::Critical);
        error_message.setWindowTitle("Error");
        error_message.setText       ("The host "  + hostname + " already has an addon " + new_addon_id);
        error_message.exec();
      }
      else
      {
        if (recorder_model_->addAddon(hostname.toStdString(), new_addon_id.toStdString()))
        {
          QModelIndex index = recorder_model_->index(hostname.toStdString(), new_addon_id.toStdString(), (int)RecorderModel::Columns::ENABLED);

          if (index.isValid())
          {
            ui_.recorder_list->scrollTo(recorder_proxy_model_->mapFromSource(index));
            ui_.recorder_list->selectionModel()->select(recorder_proxy_model_->mapFromSource(index), QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);

            if (!QEcalRec::instance()->recording())
            {
              recorder_model_->setData(index, Qt::CheckState::Checked, Qt::ItemDataRole::CheckStateRole);
            }
          }
        }
        else
        {
          QMessageBox error_message(this);
          error_message.setIcon       (QMessageBox::Icon::Critical);
          error_message.setWindowTitle("Error");
          error_message.setText       ("Failed adding addon.");
          error_message.exec();
        }
      }
    }
  }
}

void RecorderManagerWidget::removeSelectedRows()
{
  auto selected_rows = getSelectedRows();

  auto rows_that_will_actually_be_removed = recorder_model_->getRowsThatWillBeRemoved(selected_rows);
  
  // Create a list of all rows that will be removed although the user didn't select them
  std::set<int> unselected_rows_that_will_be_removed;
  for (int i : rows_that_will_actually_be_removed)
  {
    if (selected_rows.find(i) == selected_rows.end())
      unselected_rows_that_will_be_removed.emplace(i);
  }

  if (unselected_rows_that_will_be_removed.size() > 0)
  {
    QString detailed_text = "The following elements will also be removed:\n\n";
    for (auto row_it = unselected_rows_that_will_be_removed.begin(); row_it != unselected_rows_that_will_be_removed.end(); row_it++)
    {
      if (row_it != unselected_rows_that_will_be_removed.begin())
        detailed_text += "\n";
      
      detailed_text += recorder_model_->data(recorder_model_->index(*row_it, (int)RecorderModel::Columns::HOSTNAME), Qt::ItemDataRole::DisplayRole).toString();
    }

    QMessageBox error_message(this);
    error_message.setWindowIcon (QIcon(":/ecalrec/APP_ICON"));
    error_message.setIcon       (QMessageBox::Icon::Warning);
    error_message.setWindowTitle("Warning");
    error_message.setText       ("Deleting a host will also delete all of its addons (see details). Continue?");
    error_message.setDetailedText(detailed_text);
    error_message.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Abort);

    int user_selection = error_message.exec();

    if (user_selection != QMessageBox::StandardButton::Yes)
    {
      return;
    }
  }

  recorder_model_->removeRecorderRows(getSelectedRows());
}

void RecorderManagerWidget::enableSelectedRecorders()
{
  auto selected_rows = getSelectedRowIndexes();
  for (const QModelIndex& row : selected_rows)
  {
    recorder_model_->setData(recorder_model_->index(row.row(), (int)RecorderModel::Columns::ENABLED), Qt::CheckState::Checked, Qt::ItemDataRole::CheckStateRole);
  }
}

void RecorderManagerWidget::disableSelectedRecorders()
{
  auto selected_rows = getSelectedRowIndexes();
  for (const QModelIndex& row : selected_rows)
  {
    recorder_model_->setData(recorder_model_->index(row.row(), (int)RecorderModel::Columns::ENABLED), Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole);
  }
}

void RecorderManagerWidget::updateAddButton()
{
  ui_.add_button->setEnabled(!QEcalRec::instance()->recording());
}

void RecorderManagerWidget::updateRemoveButton()
{
  ui_.remove_button->setEnabled(!QEcalRec::instance()->recording()
    && (recorder_model_->getRowsThatWillBeRemoved(getSelectedRows()).size() > 0));
}

void RecorderManagerWidget::updateLocalRecordingButton()
{
  auto enabled_recorders = QEcalRec::instance()->enabledRecClients();

  ui_.local_recording_button->blockSignals(true);

  ui_.local_recording_button->setEnabled((enabled_recorders.size() > 0) && !QEcalRec::instance()->recording());

  bool all_recorders_record_locally = true;
  for (const auto& instance : enabled_recorders)
  {
    if (!((instance.second.host_filter_.size() == 1) && (*instance.second.host_filter_.begin() == instance.first)))
    {
      all_recorders_record_locally = false;
      break;
    }
  }
  ui_.local_recording_button->setChecked(all_recorders_record_locally);
  ui_.local_recording_button->blockSignals(false);
}

void RecorderManagerWidget::contextMenu(const QPoint &pos)
{
  QMenu context_menu(this);

  QAction* add_action              = new QAction(QIcon(":/ecalicons/ADD"),    tr("Add..."),           &context_menu);
  QAction* remove_action           = new QAction(QIcon(":/ecalicons/DELETE"), tr("Delete"),           &context_menu);

  QAction* select_action           = new QAction(                             tr("Select"),           &context_menu);
  QAction* deselect_action         = new QAction(                             tr("Deselect"),         &context_menu);

  context_menu.addAction(add_action);
  context_menu.addAction(remove_action);
  context_menu.addSeparator();
  context_menu.addAction(select_action);
  context_menu.addAction(deselect_action);

  connect(add_action,                &QAction::triggered, this, &RecorderManagerWidget::addButtonClicked);
  connect(remove_action,             &QAction::triggered, this, &RecorderManagerWidget::removeSelectedRows);
  connect(select_action,             &QAction::triggered, this, &RecorderManagerWidget::enableSelectedRecorders);
  connect(deselect_action,           &QAction::triggered, this, &RecorderManagerWidget::disableSelectedRecorders);

  remove_action->setEnabled(ui_.remove_button->isEnabled());
  add_action->setEnabled   (ui_.add_button->isEnabled());

  size_t selected_item_count = getSelectedRows().size();
  select_action          ->setEnabled(!QEcalRec::instance()->recording() && (selected_item_count > 0));
  deselect_action        ->setEnabled(!QEcalRec::instance()->recording() && (selected_item_count > 0));

  context_menu.exec(ui_.recorder_list->viewport()->mapToGlobal(pos));
}

void RecorderManagerWidget::setCurrentlyInUseHostsToDelegate(const std::map<std::string, eCAL::rec_server::ClientConfig>& enabled_rec_clients)
{
  std::set<QString> hosts;
  for (const auto& instance : enabled_rec_clients)
  {
    hosts.emplace(QString::fromStdString(instance.first));

    for (const std::string& filter : instance.second.host_filter_)
    {
      hosts.emplace(QString::fromStdString(filter));
    }
  }
  host_filter_delegate_->setHostsCurrentlyInUse(hosts);
}


////////////////////////////////////
/// Helper methods
////////////////////////////////////

QModelIndexList RecorderManagerWidget::getSelectedRowIndexes() const
{
  auto selected_proxy_indexes = ui_.recorder_list->selectionModel()->selectedRows();
  QModelIndexList selected_source_indexes;
  for (const auto& proxy_index : selected_proxy_indexes)
  {
    if (proxy_index.isValid())
    {
      QModelIndex source_index = recorder_proxy_model_->mapToSource(proxy_index);
      if (source_index.isValid())
        selected_source_indexes.push_back(source_index);
    }
  }
  return selected_source_indexes;
}

std::set<int> RecorderManagerWidget::getSelectedRows() const
{
  std::set<int> selected_rows;

  for (const QModelIndex& index : getSelectedRowIndexes())
  {
    selected_rows.emplace(index.row());
  }

  return selected_rows;
}
