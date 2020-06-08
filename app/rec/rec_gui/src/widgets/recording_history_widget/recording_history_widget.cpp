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

#include "recording_history_widget.h"

#include <QMenu>
#include <QInputDialog>
#include <QSettings>

#include "models/item_data_roles.h"
#include "models/tree_item_types.h"

#include "qecalrec.h"

#include <rec_client_core/ecal_rec_defs.h>

////////////////////////////////////////////////
// Constructor & Destructor
////////////////////////////////////////////////
RecordingHistoryWidget::RecordingHistoryWidget(QWidget *parent)
  : QWidget(parent)
  , first_show_event_(true)
{
  ui_.setupUi(this);

  job_history_model_ = new JobHistoryModel(ui_.job_history_treeview);

  job_history_proxy_model_ = new QStableSortFilterProxyModel(job_history_model_);
  job_history_proxy_model_->setSourceModel              (job_history_model_);
  job_history_proxy_model_->setFilterRole               (ItemDataRoles::FilterRole);
  job_history_proxy_model_->setFilterCaseSensitivity    (Qt::CaseSensitivity::CaseInsensitive);
  job_history_proxy_model_->setRecursiveFilteringEnabled(true);
  job_history_proxy_model_->setSortRole                 (ItemDataRoles::SortRole);
  job_history_proxy_model_->setSortCaseSensitivity      (Qt::CaseSensitivity::CaseInsensitive);
  job_history_proxy_model_->setDynamicSortFilter        (false);

  ui_.job_history_treeview->setModel(job_history_proxy_model_);

  // Alternating row colors
  ui_.job_history_treeview->setAlternatingRowColors(QEcalRec::instance()->alternatingRowColorsEnabled());
  connect(QEcalRec::instance(), &QEcalRec::alternatingRowColorsEnabledChanged, ui_.job_history_treeview, &QTreeView::setAlternatingRowColors);

  // Dynamic sorting
  connect(ui_.job_history_treeview->header(), &QHeaderView::sortIndicatorChanged, this
          , [this](int logical_index, Qt::SortOrder /*order*/)
            {
              // Dynamically re-sorting rapidly changing columns (like the measurement length) looks stupid and wastes CPU
              bool dynamic_sorting_allowed = 
                (logical_index == (int) JobHistoryModel::Columns::JOB)
                || (logical_index == (int) JobHistoryModel::Columns::HOSTNAME)
                || (logical_index == (int) JobHistoryModel::Columns::ADDON_NAME)
                || (logical_index == (int) JobHistoryModel::Columns::ID)
                || (logical_index == (int) JobHistoryModel::Columns::STATUS)
                || (logical_index == (int) JobHistoryModel::Columns::LOCAL_PATH)
                || (logical_index == (int) JobHistoryModel::Columns::MAX_HDF5_FILE_SIZE_MIB);
              job_history_proxy_model_->setDynamicSortFilter(dynamic_sorting_allowed);
            });
  
  // Select new jobs
  connect(job_history_model_, &QAbstractItemModel::rowsInserted, this
          , [this](const QModelIndex &parent, int first, int /*last*/)
            {
              if (!parent.isValid())
              {
                // We only select the first index, as each job will be added separately anyways.
                QModelIndex proxy_index = job_history_proxy_model_->mapFromSource(job_history_model_->index(first, (int)JobHistoryModel::Columns::JOB, parent));
                ui_.job_history_treeview->selectionModel()->select(proxy_index, QItemSelectionModel::SelectionFlag::Rows | QItemSelectionModel::SelectionFlag::ClearAndSelect);
                ui_.job_history_treeview->scrollTo(proxy_index);
              }
            });

  // Context menu
  ui_.job_history_treeview->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.job_history_treeview, &QAbstractItemView::customContextMenuRequested, this, &RecordingHistoryWidget::contextMenu);

  // Delegate for add comment
  add_comment_delegate_ = new PushButtonDelegate(QIcon(":/ecalicons/ADD_FILE")
                                                , "Comment..."
                                                , [](const QModelIndex& index)
                                                    {
                                                      int64_t job_id = index.model()->data(index.model()->index(index.row(), (int)JobHistoryModel::Columns::ID)).toLongLong();
                                                      return QEcalRec::instance()->canAddComment(job_id);
                                                    }
                                                , ui_.job_history_treeview);
  ui_.job_history_treeview->setItemDelegateForColumn((int)JobHistoryModel::Columns::COMMENT, add_comment_delegate_);

  // Delegate for upload button
  upload_button_delegate_ = new PushButtonDelegate(QIcon(":/ecalicons/MERGE")
                                                  , "Upload"
                                                  , [](const QModelIndex& index)
                                                      {
                                                        int64_t job_id = index.model()->data(index.model()->index(index.row(), (int)JobHistoryModel::Columns::ID)).toLongLong();
                                                        return QEcalRec::instance()->canUploadMeasurement(job_id);
                                                      }
                                                  , ui_.job_history_treeview);
  ui_.job_history_treeview->setItemDelegateForColumn((int)JobHistoryModel::Columns::UPLOAD, upload_button_delegate_);

  // Click on buttons in treeview
  connect(ui_.job_history_treeview, &QTreeView::clicked, this, &RecordingHistoryWidget::treeViewClicked);

  // QEcalRec connections
  connect(QEcalRec::instance(), &QEcalRec::recordJobCreatedSignal,     job_history_model_, &JobHistoryModel::recordJobCreated);
  connect(QEcalRec::instance(), &QEcalRec::recorderStatusUpdateSignal, job_history_model_, &JobHistoryModel::setRecorderStatuses);
  connect(QEcalRec::instance(), &QEcalRec::measurementDeletedSignal,   job_history_model_, &JobHistoryModel::setDeleted);
}

RecordingHistoryWidget::~RecordingHistoryWidget()
{
  saveLayout();
}

void RecordingHistoryWidget::contextMenu(const QPoint &pos)
{
  QMenu context_menu(this);

  auto selected_job_item_indexes = selectedJobItemIndexes();

  // FTP Parameters
  QString ftp_target_string;
  if (QEcalRec::instance()->uploadConfig().type_ == eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP)
    ftp_target_string = "this PC";
  else
    ftp_target_string = QString::fromStdString(QEcalRec::instance()->uploadConfig().host_);

  QAction* add_comment_action = new QAction(QIcon(":/ecalicons/ADD_FILE"), tr("Add comment..."),                                &context_menu);
  QAction* upload_action      = new QAction(QIcon(":/ecalicons/MERGE"),    tr("Merge / Upload (to ") + ftp_target_string + ")", &context_menu);
  QAction* delete_action      = new QAction(QIcon(":/ecalicons/DELETE"),   tr("Delete from disk"),                                &context_menu);

  context_menu.addAction(add_comment_action);
  context_menu.addAction(upload_action);
  context_menu.addSeparator();
  context_menu.addAction(delete_action);

  // Check whether we can add a comment
  if (selected_job_item_indexes.size() == 1)
  {
    const JobHistoryJobItem* job_item = static_cast<JobHistoryJobItem*>(job_history_model_->item(selected_job_item_indexes.front()));
    int64_t job_id = job_item->jobId();

    if (QEcalRec::instance()->canAddComment(job_id))
    {
      connect(add_comment_action, &QAction::triggered, this, [job_id]() { QEcalRec::instance()->addCommentWithDialog(job_id); });
    }
    else
    {
      add_comment_action->setEnabled(false);
    }
  }
  else
  {
    add_comment_action->setEnabled(false);
  }

  // Check wether we can upload any selected job
  bool can_upload_any = false;
  for (const auto& job_item_index : selected_job_item_indexes)
  {
    const JobHistoryJobItem* job_item = static_cast<JobHistoryJobItem*>(job_history_model_->item(job_item_index));
    int64_t job_id = job_item->jobId();
    if (QEcalRec::instance()->canUploadMeasurement(job_id))
    {
      can_upload_any = true;
      break;
    }   
  }

  // Check wether we can delete any selected job
  bool can_delete_any = false;
  for (const auto& job_item_index : selected_job_item_indexes)
  {
    const JobHistoryJobItem* job_item = static_cast<JobHistoryJobItem*>(job_history_model_->item(job_item_index));
    int64_t job_id = job_item->jobId();
    if (QEcalRec::instance()->canDeleteMeasurement(job_id))
    {
      can_delete_any = true;
      break;
    }   
  }

  connect(upload_action,      &QAction::triggered, this, &RecordingHistoryWidget::uploadSelectedMeasurements);
  upload_action->setEnabled(can_upload_any);

  connect(delete_action,      &QAction::triggered, this, &RecordingHistoryWidget::deleteSelectedMeasurements);
  delete_action->setEnabled(can_delete_any);

  context_menu.exec(ui_.job_history_treeview->viewport()->mapToGlobal(pos));
}

////////////////////////////////////////////////
// Save / Restore layout
////////////////////////////////////////////////

void RecordingHistoryWidget::showEvent(QShowEvent* /*event*/)
{
  if (first_show_event_)
  {
    // Auto-size the columns to some dummy data
    static std::map<JobHistoryModel::Columns, std::pair<bool, QString>> dummy_data
    {
      { JobHistoryModel::Columns::JOB,                    { false,  "[22:22:22] Measurement" }},
      { JobHistoryModel::Columns::ID,                     { false,  "68719476736" }},
      { JobHistoryModel::Columns::STATUS,                 { true,   "Uploading (99 MiB of 99 GiB)" }},
      { JobHistoryModel::Columns::HOSTNAME,               { false,  "CARPC01, CARPC02," }},
      { JobHistoryModel::Columns::ADDON_NAME,             { false,  "eCAL CAN rec" }},
      { JobHistoryModel::Columns::LOCAL_PATH,             { false,  "C:\\ecal_meas\\long_path" }},
      { JobHistoryModel::Columns::MAX_HDF5_FILE_SIZE_MIB, { false,  "9999" }},
      { JobHistoryModel::Columns::LENGTH,                 { false,  "999.9 s / 99999 frames" }},
      { JobHistoryModel::Columns::INFO,                   { true,   "No information available (This is a very long error message)" }},
    };

    QFontMetrics tree_font_metrics   = ui_.job_history_treeview->fontMetrics();
    QFontMetrics header_font_metrics = ui_.job_history_treeview->header()->fontMetrics();

    int minimum_column_width = ui_.job_history_treeview->header()->minimumSectionSize();
    
    for (int column = 0; column < ui_.job_history_treeview->model()->columnCount(); column++)
    {
      auto dummy_it = dummy_data.find((JobHistoryModel::Columns)column);
      if (dummy_it != dummy_data.end())
      {
        // This calculation is far from exact, at some points is just relies on some magic numbers (for icon and frame sizes)
        int dummy_text_width = tree_font_metrics.boundingRect(dummy_it->second.second).width() + 4;
        int icon_width       = (dummy_it->second.first ? 25 : 0);
        int header_width     = header_font_metrics.boundingRect(ui_.job_history_treeview->model()->headerData(column, Qt::Orientation::Horizontal).toString()).width() + 4;
        
        ui_.job_history_treeview->header()->resizeSection(column, std::max(std::max((dummy_text_width + icon_width), header_width), minimum_column_width));
      }
    }

    // Hide Columns that the user may not need that often
    static std::vector<JobHistoryModel::Columns> intial_colum_selection =
    {
      JobHistoryModel::Columns::JOB,
      JobHistoryModel::Columns::LENGTH,
      JobHistoryModel::Columns::STATUS,
      JobHistoryModel::Columns::COMMENT,
      JobHistoryModel::Columns::UPLOAD,
      JobHistoryModel::Columns::INFO,
    };
    for (int col = 0; col < (int)JobHistoryModel::Columns::COLUMN_COUNT; col++)
    {
      if (std::find(intial_colum_selection.begin(), intial_colum_selection.end(), (JobHistoryModel::Columns)col) == intial_colum_selection.end())
      {
        ui_.job_history_treeview->hideColumn(col);
      }
    }

    saveInitialLayout();
    restoreLayout();
  }
  first_show_event_ = false;
}

void RecordingHistoryWidget::saveLayout()
{
  QSettings settings;
  settings.setValue("recording_history_tree_state", ui_.job_history_treeview->saveState(eCAL::rec::Version()));
}

void RecordingHistoryWidget::restoreLayout()
{
  QSettings settings;
  QVariant recording_history_tree_state_variant = settings.value("recording_history_tree_state");
  if (recording_history_tree_state_variant.isValid())
  {
    QByteArray recording_history_tree_state = recording_history_tree_state_variant.toByteArray();
    ui_.job_history_treeview->restoreState(recording_history_tree_state, eCAL::rec::Version());
  }
}

void RecordingHistoryWidget::saveInitialLayout()
{
  initial_job_history_tree_state_ = ui_.job_history_treeview->saveState(eCAL::rec::Version());
}

void RecordingHistoryWidget::resetLayout()
{
  ui_.job_history_treeview->restoreState(initial_job_history_tree_state_, eCAL::rec::Version());
}


////////////////////////////////////////////////
// Upload / Merge
////////////////////////////////////////////////
void RecordingHistoryWidget::uploadSelectedMeasurements()
{
  auto selected_jobs = selectedJobItems();
  for (JobHistoryJobItem* job_item : selected_jobs)
  {
    auto job_id = job_item->jobId();
    if (QEcalRec::instance()->canUploadMeasurement(job_id))
    {
      QEcalRec::instance()->uploadMeasurement(job_id);
    }
  }
}

void RecordingHistoryWidget::treeViewClicked(const QModelIndex& proxy_index)
{
  QModelIndex source_index = job_history_proxy_model_->mapToSource(proxy_index);

  if ((source_index.column() == (int)JobHistoryModel::Columns::COMMENT)
    && (job_history_model_->item(source_index)->type() == (int)TreeItemType::JobHistoryJobItem))
  {
    const JobHistoryJobItem* job_item = static_cast<JobHistoryJobItem*>(job_history_model_->item(source_index));
    if (QEcalRec::instance()->canAddComment(job_item->jobId()))
    {
      QEcalRec::instance()->addCommentWithDialog(job_item->jobId());
    }
  }
  else if ((source_index.column() == (int)JobHistoryModel::Columns::UPLOAD)
    && (job_history_model_->item(source_index)->type() == (int)TreeItemType::JobHistoryJobItem))
  {
    const JobHistoryJobItem* job_item = static_cast<JobHistoryJobItem*>(job_history_model_->item(source_index));

    if (QEcalRec::instance()->canUploadMeasurement(job_item->jobId()))
    {
      QEcalRec::instance()->uploadMeasurement(job_item->jobId());
    }
  }
}

////////////////////////////////////////////////
// Delete
////////////////////////////////////////////////

void RecordingHistoryWidget::deleteSelectedMeasurements()
{
  auto selected_jobs = selectedJobItems();

  std::set<int64_t> jobs_to_delete;
  for (JobHistoryJobItem* job_item : selected_jobs)
  {

    auto job_id = job_item->jobId();
    if (QEcalRec::instance()->canDeleteMeasurement(job_id))
    {
      jobs_to_delete.emplace(job_id);
    }
  }

  QEcalRec::instance()->deleteMeasurement(jobs_to_delete);
}

////////////////////////////////////////////////
// Auxiliary Methods
////////////////////////////////////////////////
QModelIndexList RecordingHistoryWidget::selectedSourceIndexes(int column) const
{
  QModelIndexList selected_source_indexes;

  auto selected_proxy_indexes = ui_.job_history_treeview->selectionModel()->selectedRows(column);
  for (const auto& proxy_index : selected_proxy_indexes)
  {
    selected_source_indexes.push_back(job_history_proxy_model_->mapToSource(proxy_index));
  }

  return selected_source_indexes;
}

QModelIndexList RecordingHistoryWidget::selectedJobItemIndexes(int column) const
{
  QModelIndexList selected_job_item_indexes;

  for (const QModelIndex& index :  selectedSourceIndexes(column))
  {
    if (!index.parent().isValid())
    {
      selected_job_item_indexes.push_back(index);
    }
  }
  
  return selected_job_item_indexes;
}

QList<JobHistoryJobItem*> RecordingHistoryWidget::selectedJobItems() const
{
  QList<JobHistoryJobItem*> selected_job_items;

  for (const QModelIndex& index :  selectedJobItemIndexes())
  {
    QAbstractTreeItem* tree_item = job_history_model_->item(index);
    if (tree_item->type() == (int)TreeItemType::JobHistoryJobItem)
    {
      selected_job_items.push_back(static_cast<JobHistoryJobItem*>(tree_item));
    }
  }
  return selected_job_items;
}
