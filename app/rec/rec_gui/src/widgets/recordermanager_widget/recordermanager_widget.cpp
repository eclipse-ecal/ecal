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

#include <QString>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QIcon>

#include <sstream>

#include "models/item_data_roles.h"


RecorderManagerWidget::RecorderManagerWidget(QWidget *parent)
  : QWidget(parent)
  , activate_button_state_is_ativate_(true)
{
  ui_.setupUi(this);

  // Recorder model
  recorder_model_ = new RecorderModel(this);
  recorder_proxy_model_ = new QStableSortFilterProxyModel(this);
  recorder_proxy_model_->setSortRole(ItemDataRoles::SortRole);
  recorder_proxy_model_->setFilterRole(ItemDataRoles::FilterRole);
  recorder_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  recorder_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  recorder_proxy_model_->setSourceModel(recorder_model_);
  recorder_proxy_model_->setDynamicSortFilter(false);
  ui_.recorder_list->setModel(recorder_proxy_model_);

  // Tree Header
  tree_header_ = new QCheckboxHeaderView(ui_.recorder_list);
  tree_header_->setStretchLastSection(true);
  tree_header_->setCheckable((int)RecorderModel::Columns::ENABLED);
  ui_.recorder_list->setHeader(tree_header_);

  ui_.recorder_list->resizeColumnToContents(0);

  // Delegate for Host filter
  host_filter_delegate_ = new HostFilterDelegate(ui_.recorder_list);
  ui_.recorder_list->setItemDelegateForColumn((int)RecorderModel::Columns::HOST_FILTER, host_filter_delegate_);

  connect(QEcalRec::instance(), &QEcalRec::monitorUpdatedSignal, this,
      [this]()
      {
        std::set<QString> host_list;
        for (const auto& host : QEcalRec::instance()->hostsRunningEcalRec())
        {
          host_list.emplace(QString::fromStdString(host.first));
        }
        host_filter_delegate_->setMonitorHosts(host_list);
      });
  connect(QEcalRec::instance(), &QEcalRec::recorderInstancesChangedSignal, this, &RecorderManagerWidget::setCurrentlyInUseTopicsToDelegate);
  connect(QEcalRec::instance(), &QEcalRec::hostFilterChangedSignal, this, [this]() {setCurrentlyInUseTopicsToDelegate(QEcalRec::instance()->recorderInstances()); });

  connect(ui_.recorder_list, &QTreeView::clicked, this,
      [this](const QModelIndex& index)
      {
        if (index.flags() & Qt::ItemFlag::ItemIsEditable)
        {
          ui_.recorder_list->edit(index);
        }
      });

  // Tree Header connections
  ui_.recorder_list->sortByColumn((int)RecorderModel::Columns::HOSTNAME, Qt::SortOrder::AscendingOrder);

  connect(tree_header_,    &QCheckboxHeaderView::clicked,             this, &RecorderManagerWidget::headerClicked);
  connect(recorder_model_, &QAbstractItemModel::dataChanged,          this, &RecorderManagerWidget::treeDataChanged);

  connect(recorder_model_, &QAbstractItemModel::rowsInserted,         this, &RecorderManagerWidget::updateHeader);
  connect(recorder_model_, &QAbstractItemModel::rowsRemoved,          this, &RecorderManagerWidget::updateHeader);

  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal, this, &RecorderManagerWidget::updateHeader);


  // Context menu
  ui_.recorder_list->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.recorder_list, &QAbstractItemView::customContextMenuRequested, this, &RecorderManagerWidget::contextMenu);

  // Buttons
  connect(ui_.local_recording_button, &QAbstractButton::clicked, this, &RecorderManagerWidget::localRecordingButtonPressed);
  connect(ui_.add_button,             &QAbstractButton::clicked, this, &RecorderManagerWidget::addNewRecorder);
  connect(ui_.remove_button,          &QAbstractButton::clicked, this, &RecorderManagerWidget::removeSelectedRecorders);

  connect(ui_.recorder_list->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RecorderManagerWidget::updateRemoveButton);
  connect(recorder_model_,                     &QAbstractItemModel::dataChanged,       this, &RecorderManagerWidget::updateRemoveButton); // TODO: Would it increase performance to use an extra signal for this?

  connect(QEcalRec::instance(), &QEcalRec::hostFilterChangedSignal,        this, &RecorderManagerWidget::updateLocalRecordingButton);
  connect(QEcalRec::instance(), &QEcalRec::recorderInstancesChangedSignal, this, &RecorderManagerWidget::updateLocalRecordingButton);
  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,    this, &RecorderManagerWidget::updateLocalRecordingButton);

  // Buffering
  connect(ui_.pre_buffer_checkbox, &QCheckBox::toggled, QEcalRec::instance(), [this](bool checked) {QEcalRec::instance()->setPreBufferingEnabled(checked); });
  connect(ui_.pre_buffer_spinbox, static_cast<void (QDoubleSpinBox:: *)(double)>(&QDoubleSpinBox::valueChanged), this, [](double val) {QEcalRec::instance()->setMaxPreBufferLength(std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(val))); });
  connect(QEcalRec::instance(), &QEcalRec::preBufferingEnabledChangedSignal, this, &RecorderManagerWidget::preBufferingEnabledChanged);
  connect(QEcalRec::instance(), &QEcalRec::maxPreBufferLengthChangedSignal, this, &RecorderManagerWidget::maxPreBufferLengthChanged);

  // Initial state
  updateRemoveButton();
  updateLocalRecordingButton();
  preBufferingEnabledChanged(QEcalRec::instance()->preBufferingEnabled());
  maxPreBufferLengthChanged(QEcalRec::instance()->maxPreBufferLength());
  updateHeader();
  setCurrentlyInUseTopicsToDelegate(QEcalRec::instance()->recorderInstances());
}

RecorderManagerWidget::~RecorderManagerWidget()
{}

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

  if (fabs(seconds - ui_.pre_buffer_spinbox->value()) > 0.01)
  {
    ui_.pre_buffer_spinbox->blockSignals(true);
    ui_.pre_buffer_spinbox->setValue(seconds);
    ui_.pre_buffer_spinbox->blockSignals(false);
  }
}

void RecorderManagerWidget::headerClicked(int column, bool checked)
{

  if (column == (int)RecorderModel::Columns::ENABLED)
  {
    recorder_model_->setAllChecked(checked);
    QEcalRec::instance()->setRecorderInstances(recorder_model_->recorderInstances());
  }
}

void RecorderManagerWidget::updateHeader()
{
  int checked_item_count = (int)recorder_model_->recorderInstances().size();

  if (checked_item_count == 0)
  {
    tree_header_->setCheckState((int)RecorderModel::Columns::ENABLED, Qt::CheckState::Unchecked);
  }
  else if (checked_item_count == recorder_model_->rowCount())
  {
    tree_header_->setCheckState((int)RecorderModel::Columns::ENABLED, Qt::CheckState::Checked);
  }
  else
  {
    tree_header_->setCheckState((int)RecorderModel::Columns::ENABLED, Qt::CheckState::PartiallyChecked);
  }

  tree_header_->setCheckboxEnabled(0, !QEcalRec::instance()->recordersRecording());
}

void RecorderManagerWidget::treeDataChanged(const QModelIndex &top_left, const QModelIndex &bottom_right, const QVector<int> &roles)
{
  if ((top_left.column() <= (int)RecorderModel::Columns::ENABLED) && ((int)RecorderModel::Columns::ENABLED <= bottom_right.column()))
  {
    if (roles.empty() || roles.contains(Qt::ItemDataRole::CheckStateRole))
    {
      updateHeader();
    }
  }
}

void RecorderManagerWidget::localRecordingButtonPressed()
{
  auto recorder_instances = QEcalRec::instance()->recorderInstances();
  for (auto& recorder : recorder_instances)
  {
    recorder.second = std::set<std::string>{ recorder.first };
  }
  QEcalRec::instance()->setRecorderInstances(recorder_instances);
}

void RecorderManagerWidget::addNewRecorder()
{
  QString text = QInputDialog::getText(this, tr("Add host"), tr("Hostname:"), QLineEdit::Normal);
  if (!text.isEmpty())
  {
    if (!recorder_model_->containsHost(text.toStdString()))
    {
      if (recorder_model_->addHost(text.toStdString()))
      {
        QModelIndex index = recorder_model_->index(text.toStdString());
        if (index.isValid())
        {
          ui_.recorder_list->scrollTo(recorder_proxy_model_->mapFromSource(index));
          ui_.recorder_list->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
        }
      }
    }
  }
}

void RecorderManagerWidget::removeSelectedRecorders()
{
  auto selected_rows = getSelectedRemovableRows();

  // copying the row numbers into a set will automatically sort them
  std::set<int> rows;
  for (const auto& row_index : selected_rows)
  {
    rows.emplace(row_index.row());
  }

  // Remove back to front
  for (auto row = rows.rbegin(); row != rows.rend(); row++)
  {
    recorder_model_->removeRow(*row);
  }
}

void RecorderManagerWidget::enableSelectedRecorders()
{
  auto selected_rows = getSelectedRows();
  for (const QModelIndex& row : selected_rows)
  {
    recorder_model_->setData(recorder_model_->index(row.row(), (int)RecorderModel::Columns::ENABLED), Qt::CheckState::Checked, Qt::ItemDataRole::CheckStateRole);
  }
}

void RecorderManagerWidget::disableSelectedRecorders()
{
  auto selected_rows = getSelectedRows();
  for (const QModelIndex& row : selected_rows)
  {
    recorder_model_->setData(recorder_model_->index(row.row(), (int)RecorderModel::Columns::ENABLED), Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole);
  }
}

void RecorderManagerWidget::invertRecorderSelection()
{
  recorder_model_->invertSelection();
  QEcalRec::instance()->setRecorderInstances(recorder_model_->recorderInstances());
}

void RecorderManagerWidget::updateRemoveButton()
{
  ui_.remove_button->setEnabled(getSelectedRemovableRows().size() > 0);
}

void RecorderManagerWidget::updateLocalRecordingButton()
{
  auto recorder_instances = QEcalRec::instance()->recorderInstances();

  ui_.local_recording_button->blockSignals(true);

  ui_.local_recording_button->setEnabled((recorder_instances.size() > 0) && !QEcalRec::instance()->recordersRecording());

  bool all_recorders_record_locally = true;
  for (const auto& instance : recorder_instances)
  {
    if (!((instance.second.size() == 1) && (*instance.second.begin() == instance.first)))
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
  QAction* remove_action           = new QAction(QIcon(":/ecalicons/REMOVE"), tr("Remove"),           &context_menu);

  QAction* select_action           = new QAction(                             tr("Select"),           &context_menu);
  QAction* deselect_action         = new QAction(                             tr("Deselect"),         &context_menu);
  QAction* invert_selection_action = new QAction(                             tr("Invert selection"), &context_menu);

  context_menu.addAction(add_action);
  context_menu.addAction(remove_action);
  context_menu.addSeparator();
  context_menu.addAction(select_action);
  context_menu.addAction(deselect_action);
  context_menu.addAction(invert_selection_action);

  connect(add_action,                &QAction::triggered, this, &RecorderManagerWidget::addNewRecorder);
  connect(remove_action,             &QAction::triggered, this, &RecorderManagerWidget::removeSelectedRecorders);
  connect(select_action,             &QAction::triggered, this, &RecorderManagerWidget::enableSelectedRecorders);
  connect(deselect_action,           &QAction::triggered, this, &RecorderManagerWidget::disableSelectedRecorders);
  connect(invert_selection_action,   &QAction::triggered, this, &RecorderManagerWidget::invertRecorderSelection);

  remove_action->setEnabled(getSelectedRemovableRows().size() > 0);

  int selected_item_count = getSelectedRows().size();
  select_action          ->setEnabled(!QEcalRec::instance()->recordersRecording() && (selected_item_count > 0));
  deselect_action        ->setEnabled(!QEcalRec::instance()->recordersRecording() && (selected_item_count > 0));
  invert_selection_action->setEnabled(!QEcalRec::instance()->recordersRecording() && (recorder_model_->rowCount() > 0));

  context_menu.exec(ui_.recorder_list->viewport()->mapToGlobal(pos));
}

void RecorderManagerWidget::setCurrentlyInUseTopicsToDelegate(const std::vector<std::pair<std::string, std::set<std::string>>>& recorder_instances)
{
  std::set<QString> hosts;
  for (const auto& instance : recorder_instances)
  {
    hosts.emplace(QString::fromStdString(instance.first));

    for (const std::string& filter : instance.second)
    {
      hosts.emplace(QString::fromStdString(filter));
    }
  }
  host_filter_delegate_->setHostsCurrentlyInUse(hosts);
}


////////////////////////////////////
/// Helper methods
////////////////////////////////////

QModelIndexList RecorderManagerWidget::getSelectedRows() const
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

QModelIndexList RecorderManagerWidget::getSelectedRemovableRows() const
{
  auto selected_rows = getSelectedRows();
  QModelIndexList selected_non_visible_rows;
  for (const auto& row_index : selected_rows)
  {
    if (recorder_model_->isHostRemovable(row_index))
      selected_non_visible_rows.push_back(row_index);
  }
  return selected_non_visible_rows;
}
