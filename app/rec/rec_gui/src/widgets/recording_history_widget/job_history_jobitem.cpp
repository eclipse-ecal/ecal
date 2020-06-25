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

#include "job_history_jobitem.h"

#include "job_history_recorderitem.h"

#include "models/tree_item_types.h"
#include "models/item_data_roles.h"

#include <widgets/bytes_to_pretty_string_utils.h>

#include <qecalrec.h>

#include <QTime>
#include <QFont>

///////////////////////////////////////////
// Constructor & Destructor
///////////////////////////////////////////
JobHistoryJobItem::JobHistoryJobItem(const eCAL::rec::JobConfig& local_evaluated_job_config, std::chrono::system_clock::time_point local_start_timestamp)
  : local_evaluated_job_config_ (local_evaluated_job_config)
  , local_start_timestamp_      (local_start_timestamp)
  , is_deleted_                 (false)
  , icon_recording_             (":/ecalicons/RECORD")
  , icon_flushing_              (":/ecalicons/SAVE_TO_DISK")
  , icon_uploading_             (":/ecalicons/MERGE")
  , icon_error_                 (":/ecalicons/ERROR")
{}

JobHistoryJobItem::~JobHistoryJobItem()
{}

///////////////////////////////////////////
// QAbstractTreeItem overrides
///////////////////////////////////////////
QVariant JobHistoryJobItem::data(int column, Qt::ItemDataRole role) const
{
  if (column == (int)Columns::JOB_ID)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return static_cast<qint64>(local_evaluated_job_config_.GetJobId());
    }
  }

  else if (column == (int)Columns::JOB)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      time_t time_t_time = std::chrono::system_clock::to_time_t(local_start_timestamp_);
      char time_string[12];
      size_t strftime_output_size = strftime(time_string, 12, "[%H:%M:%S] ", localtime(&time_t_time));
      time_string[strftime_output_size] = '\0';
      return (QString(time_string) + QString::fromStdString(local_evaluated_job_config_.GetMeasName()));
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      auto info = combinedInfo();
      if (!info.first)
        return icon_error_;
    }
    else if (role == ItemDataRoles::SortRole)
    {
      return (qint64)local_start_timestamp_.time_since_epoch().count();
    }
  }

  else if (column == (int)Columns::INVOLVED_HOSTS)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      // Collect involved hosts
      std::set<QString> involved_hosts;
      for (int i = 0; i < childCount(); i++)
      {
        QAbstractTreeItem* tree_item = child(i);

        if (tree_item->type() != (int)TreeItemType::JobHistoryRecorderItem)
          continue;

        const JobHistoryRecorderItem* job_item = static_cast<JobHistoryRecorderItem*>(tree_item);
        involved_hosts.emplace(job_item->hostname());
      }

      // Create comma-separated list
      QString involved_hosts_string;
      for (auto host_it = involved_hosts.begin(); host_it != involved_hosts.end(); host_it++)
      {
        if (host_it != involved_hosts.begin())
          involved_hosts_string += ", ";
        involved_hosts_string += *host_it;
      }
      return involved_hosts_string;
    }
  }

  else if (column == (int)Columns::LENGTH)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      auto combined_length = combinedLength();
      return QString::number(std::chrono::duration_cast<std::chrono::duration<double>>(combined_length.first).count(), 'f', 1) + " s / "
       + QString::number(combined_length.second) + " frames";
    }
  }

  else if (column == (int)Columns::STATUS)
  {
    eCAL::rec::JobState combined_state = combinedJobSate();

    if (role == ItemDataRoles::RawDataRole)
    {
      return static_cast<int>(combined_state);
    }

    if (is_deleted_)
    {
      if (role == Qt::ItemDataRole::DisplayRole)
        return "Deleted";
    }
    else if (combined_state == eCAL::rec::JobState::NotStarted)
    {
      if (role == Qt::ItemDataRole::DisplayRole)
        return "No Started";
    }
    else if (combined_state == eCAL::rec::JobState::Recording)
    {
      if (role == Qt::ItemDataRole::DisplayRole)
        return "Recording";
      else if (role == Qt::ItemDataRole::DecorationRole)
        return icon_recording_;
    }
    else if (combined_state == eCAL::rec::JobState::Flushing)
    {
      if (role == Qt::ItemDataRole::DisplayRole)
        return "Flushing (" + QString::number(combinedUnflushedFrames()) + " frames)";
      else if (role == Qt::ItemDataRole::DecorationRole)
        return icon_flushing_;
    }
    else if (combined_state == eCAL::rec::JobState::FinishedFlushing)
    {
      if (role == Qt::ItemDataRole::DisplayRole)
        return "Finished";
    }
    else if (combined_state == eCAL::rec::JobState::Uploading)
    {
      if (role == Qt::ItemDataRole::DisplayRole)
      {
        eCAL::rec::UploadStatus upload_status = combinedUploadStatus();
        return "Uploading (" + bytesToPrettyString(upload_status.bytes_uploaded_) + " of " + bytesToPrettyString(upload_status.bytes_total_size_) + ")";
      }
      else if (role == Qt::ItemDataRole::DecorationRole)
      {
        return icon_uploading_;
      }
    }
    else if (combined_state == eCAL::rec::JobState::FinishedUploading)
    {
      if (role == Qt::ItemDataRole::DisplayRole)
      {
        eCAL::rec::UploadStatus upload_status = combinedUploadStatus();
        return QString("Finished Uploading (") + bytesToPrettyString(upload_status.bytes_uploaded_) + ")";
      }
    }

    if (role == ItemDataRoles::SortRole)
    {
      return (int)combined_state;
    }
  }

  else if (column == (int)Columns::LOCAL_PATH)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return QString::fromStdString(local_evaluated_job_config_.GetCompleteMeasurementPath());
    }
  }

  else if (column == (int)Columns::MAX_HDF5_FILE_SIZE_MIB)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return QString::number(local_evaluated_job_config_.GetMaxFileSize()) + " MiB";
    }
    else if (role == ItemDataRoles::SortRole)
    {
      return (quint64)local_evaluated_job_config_.GetMaxFileSize();
    }
  }

  else if (column == (int)Columns::UPLOAD)
  {
    if (role == ItemDataRoles::SortRole)
    {
      return (QEcalRec::instance()->canUploadMeasurement(local_evaluated_job_config_.GetJobId()));
    }
  }

  else if (column == (int)Columns::INFO)
  {
    auto info = combinedInfo();

    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return info.second;
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (!info.first)
        return icon_error_;
    }
  }

  // Fallback to the DisplayRole
  if ((role == ItemDataRoles::SortRole)
    || (role == ItemDataRoles::FilterRole)
    || (role == Qt::ItemDataRole::ToolTipRole))
  {
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  // Font settings
  if (role == Qt::ItemDataRole::FontRole)
  {
    if (is_deleted_ && column != (int)Columns::INFO) // Don't strikethrough the info column, so the user can clearly ready error messages
    {
      QFont font;
      font.setStrikeOut(true);
      return font;
    }
  }
  if (role == Qt::ItemDataRole::ForegroundRole)
  {
    if (is_deleted_)
    {
      return QColor(127, 127, 127);
    }
  }

  return QVariant();
}


int JobHistoryJobItem::type() const
{
  return (int)TreeItemType::JobHistoryJobItem;
}

///////////////////////////////////////////
// Lazy Getters, Setters & Update methods
///////////////////////////////////////////

int64_t                               JobHistoryJobItem::jobId()     const { return local_evaluated_job_config_.GetJobId(); }
std::chrono::system_clock::time_point JobHistoryJobItem::timestamp() const { return local_start_timestamp_; };
bool                                  JobHistoryJobItem::isDeleted() const { return is_deleted_; }

void                                  JobHistoryJobItem::setIsDeleted(bool is_deleted) { is_deleted_ = is_deleted; }

bool JobHistoryJobItem::updateIsDeleted(bool is_deleted)
{
  if (is_deleted_ != is_deleted)
  {
    is_deleted_ = is_deleted;
    return true;
  }
  return false;
}

eCAL::rec::JobState                   JobHistoryJobItem::combinedJobSate() const
{
  // Create a map to count how many children have which state
  std::map<eCAL::rec::JobState, int> job_state_count
  {
    { eCAL::rec::JobState::NotStarted,        0 },
    { eCAL::rec::JobState::Recording,         0 },
    { eCAL::rec::JobState::Flushing,          0 },
    { eCAL::rec::JobState::FinishedFlushing,  0 },
    { eCAL::rec::JobState::Uploading,         0 },
    { eCAL::rec::JobState::FinishedUploading, 0 },
  };

  // Count Job States
  for (int i = 0; i < childCount(); i++)
  {
    QAbstractTreeItem* tree_item = child(i);

    if (tree_item->type() != (int)TreeItemType::JobHistoryRecorderItem)
      continue;

    job_state_count[static_cast<JobHistoryRecorderItem*>(tree_item)->state()]++;
  }

  // Evaluate counted Job states to calculate a combined state
  if (job_state_count[eCAL::rec::JobState::Recording] > 0)
  {
    return eCAL::rec::JobState::Recording;
  }
  else if (job_state_count[eCAL::rec::JobState::Flushing] > 0)
  {
    return eCAL::rec::JobState::Flushing;
  }
  else if ((job_state_count[eCAL::rec::JobState::FinishedFlushing] > 0)
    && (job_state_count[eCAL::rec::JobState::Uploading]         == 0)
    && (job_state_count[eCAL::rec::JobState::FinishedUploading] == 0))
  {
    return eCAL::rec::JobState::FinishedFlushing;
  }
  else if (job_state_count[eCAL::rec::JobState::Uploading] > 0)
  {
    return eCAL::rec::JobState::Uploading;
  }
  else if (job_state_count[eCAL::rec::JobState::FinishedUploading] > 0)
  {
    return eCAL::rec::JobState::FinishedUploading;
  }
  else
  {
    return eCAL::rec::JobState::NotStarted;
  }
}

std::pair<std::chrono::steady_clock::duration, int64_t> JobHistoryJobItem::combinedLength() const
{
  std::chrono::steady_clock::duration duration(0);
  int64_t frame_count(0);

  for (int i = 0; i < childCount(); i++)
  {
    QAbstractTreeItem* tree_item = child(i);

    if (tree_item->type() != (int)TreeItemType::JobHistoryRecorderItem)
      continue;

    auto length = static_cast<JobHistoryRecorderItem*>(tree_item)->length();

    duration    = std::max(duration, length.first);
    frame_count += length.second;
  }

  return { duration, frame_count };
}

int64_t JobHistoryJobItem::combinedUnflushedFrames() const
{
  int64_t frame_count(0);

  for (int i = 0; i < childCount(); i++)
  {
    QAbstractTreeItem* tree_item = child(i);

    if (tree_item->type() != (int)TreeItemType::JobHistoryRecorderItem)
      continue;

    frame_count += static_cast<JobHistoryRecorderItem*>(tree_item)->unflushedFrameCount();
  }

  return frame_count;
}

eCAL::rec::UploadStatus JobHistoryJobItem::combinedUploadStatus() const
{
  eCAL::rec::UploadStatus status;

  for (int i = 0; i < childCount(); i++)
  {
    QAbstractTreeItem* tree_item = child(i);

    if (tree_item->type() != (int)TreeItemType::JobHistoryRecorderItem)
      continue;

    const eCAL::rec::UploadStatus this_item_upload_status = static_cast<JobHistoryRecorderItem*>(tree_item)->uploadStatus();
    status.bytes_total_size_ += this_item_upload_status.bytes_total_size_;
    status.bytes_uploaded_   += this_item_upload_status.bytes_uploaded_;
  }

  return status;
}

std::pair<bool, QString> JobHistoryJobItem::combinedInfo() const
{
  std::map<QString, std::pair<bool, std::string>> error_infos;

  for (int i = 0; i < childCount(); i++)
  {
    QAbstractTreeItem* tree_item = child(i);

    if (tree_item->type() != (int)TreeItemType::JobHistoryRecorderItem)
      continue;

    JobHistoryRecorderItem* recorder_item = static_cast<JobHistoryRecorderItem*>(tree_item);

    // Collect the errors of all children (ignore the infos that are OK)
    auto displayed_info = recorder_item->displayedInfo();
    if (!displayed_info.first)
    {
      QString name;
      if(recorder_item->isAddonItem())
      {
        name = (!recorder_item->addonName().isEmpty() ? recorder_item->addonName() : recorder_item->addonId()) + "(" + recorder_item->hostname() + ")";
      }
      else
      {
        name = recorder_item->hostname();
      }
      error_infos.emplace(name, displayed_info);
    }
  }

  if (error_infos.empty())
  {
    // No Error: Just return no error
    return { true, "" };
  }
  else if (error_infos.size() == 1)
  {
    // Only 1 Error: Return the error description of that one error
    return { false, error_infos.begin()->first + ": " + QString::fromStdString(error_infos.begin()->second.second) };
  }
  else
  {
    // 2 or more errors: Return a list of all items that have an error
    QString error_items_string = QString::number(error_infos.size()) + " Errors: ";
    for (auto error_info_it = error_infos.begin(); error_info_it != error_infos.end(); error_info_it++)
    {
      if (error_info_it != error_infos.begin())
        error_items_string += QString(", ");

      error_items_string += error_info_it->first;
    }

    return { false, error_items_string };
  }
}