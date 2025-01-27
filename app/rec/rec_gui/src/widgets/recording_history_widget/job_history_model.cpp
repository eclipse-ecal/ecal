/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "job_history_model.h"

#include "models/tree_item_types.h"

#include <qecalrec.h>

#include <QList>
#include <QSet>

#include <algorithm>

///////////////////////////////////////////
// Constructor & Destructor
///////////////////////////////////////////
JobHistoryModel::JobHistoryModel(QObject *parent)
  : QAbstractTreeModel(parent)
{
}

JobHistoryModel::~JobHistoryModel()
{}

///////////////////////////////////////////
// QAbstractTreeModel overrides
///////////////////////////////////////////

QVariant JobHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((role == Qt::ItemDataRole::DisplayRole)
    && (orientation == Qt::Orientation::Horizontal))
  {
    return column_labels_.at((Columns)section);
  }
  return QVariant(); // Invalid QVariant
}

int JobHistoryModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

int JobHistoryModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  if (tree_item_type == (int)TreeItemType::JobHistoryJobItem)
  {
    switch (model_column)
    {
    case (int)Columns::JOB:
      return (int)JobHistoryJobItem::Columns::JOB;
    case (int)Columns::HOSTNAME:
      return (int)JobHistoryJobItem::Columns::INVOLVED_HOSTS;
    case (int)Columns::ADDON_NAME:
      return -1;
    case (int)Columns::ID:
      return (int)JobHistoryJobItem::Columns::JOB_ID;
    case (int)Columns::LENGTH:
      return (int)JobHistoryJobItem::Columns::LENGTH;
    case (int)Columns::STATUS:
      return (int)JobHistoryJobItem::Columns::STATUS;
    case (int)Columns::LOCAL_PATH:
      return (int)JobHistoryJobItem::Columns::LOCAL_PATH;
    case (int)Columns::MAX_HDF5_FILE_SIZE_MIB:
      return (int)JobHistoryJobItem::Columns::MAX_HDF5_FILE_SIZE_MIB;
    case (int)Columns::UPLOAD:
      return (int)JobHistoryJobItem::Columns::UPLOAD;
    case (int)Columns::INFO:
      return (int)JobHistoryJobItem::Columns::INFO;
    default:
      break;
    }
  }

  else if (tree_item_type == (int)TreeItemType::JobHistoryRecorderItem)
  {
    switch (model_column)
    {
    case (int)Columns::JOB:
      return (int)JobHistoryRecorderItem::Columns::RECORDER_NAME;
    case (int)Columns::HOSTNAME:
      return (int)JobHistoryRecorderItem::Columns::HOSTNAME;
    case (int)Columns::ADDON_NAME:
      return (int)JobHistoryRecorderItem::Columns::ADDON_NAME;
    case (int)Columns::ID:
      return (int)JobHistoryRecorderItem::Columns::ADDON_ID;
    case (int)Columns::PROCESS_ID:
      return (int)JobHistoryRecorderItem::Columns::PROCESS_ID;
    case (int)Columns::STILL_ONLINE:
      return (int)JobHistoryRecorderItem::Columns::STILL_ONLINE;
    case (int)Columns::LENGTH:
      return (int)JobHistoryRecorderItem::Columns::LENGTH;
    case (int)Columns::STATUS:
      return (int)JobHistoryRecorderItem::Columns::STATUS;
    case (int)Columns::LOCAL_PATH:
      return -1;
    case (int)Columns::MAX_HDF5_FILE_SIZE_MIB:
      return -1;
    case (int)Columns::INFO:
      return (int)JobHistoryRecorderItem::Columns::INFO;
    default:
      break;
    }
  }

  return -1;
}

///////////////////////////////////////////
// External API
///////////////////////////////////////////

void JobHistoryModel::setRecorderStatuses(const eCAL::rec_server::RecorderStatusMap_T& recorder_statuses, const std::list<eCAL::rec_server::JobHistoryEntry>& job_history)
{
  QSet<QAbstractTreeItem*> items_to_update;

  for (const eCAL::rec_server::JobHistoryEntry& job_history_entry : job_history)
  {
    JobHistoryJobItem* job_item = findJobItemById(job_history_entry.local_evaluated_job_config_.GetJobId());
    if (job_item == nullptr) continue;

    if (job_item->updateIsDeleted(job_history_entry.is_deleted_))
      items_to_update.insert(job_item);

    for (const auto& client_job_status : job_history_entry.client_statuses_)
    {
      // Data that is needed by HDF5 Items _and_ addon items
      std::string hostname              = client_job_status.first;
      int         process_id                   = client_job_status.second.client_pid_;
      bool        is_deleted            = client_job_status.second.job_status_.is_deleted_;
      auto        recorder_status_it    = std::find_if(recorder_statuses.begin(), recorder_statuses.end(),
                                                [process_id = client_job_status.second.client_pid_] (const auto& rec_status) -> bool
                                                {
                                                  return (rec_status.second.first.pid_ == process_id);
                                                });
      bool        recorder_still_online = (recorder_status_it != recorder_statuses.end());

      // Update the HDF5 Item
      JobHistoryRecorderItem* hdf5_recorder_item = findRecorderItem(job_item, QString::fromStdString(hostname));

      if (hdf5_recorder_item == nullptr) continue;

      bool update_needed_hdf5_rec = false;
      update_needed_hdf5_rec = hdf5_recorder_item->updateStillOnline            (recorder_still_online) || update_needed_hdf5_rec;
      update_needed_hdf5_rec = hdf5_recorder_item->updatePid                    (process_id) || update_needed_hdf5_rec;
      update_needed_hdf5_rec = hdf5_recorder_item->updateInfoLastCommandResponse(client_job_status.second.info_last_command_response_) || update_needed_hdf5_rec;
      update_needed_hdf5_rec = hdf5_recorder_item->updateLength                 ({ client_job_status.second.job_status_.rec_hdf5_status_.total_length_, client_job_status.second.job_status_.rec_hdf5_status_.total_frame_count_ }) || update_needed_hdf5_rec;
      update_needed_hdf5_rec = hdf5_recorder_item->updateUnflushedFrameCount    (client_job_status.second.job_status_.rec_hdf5_status_.unflushed_frame_count_) || update_needed_hdf5_rec;
      update_needed_hdf5_rec = hdf5_recorder_item->updateState                  (client_job_status.second.job_status_.state_) || update_needed_hdf5_rec;
      update_needed_hdf5_rec = hdf5_recorder_item->updateUploadStatus           (client_job_status.second.job_status_.upload_status_) || update_needed_hdf5_rec;
      update_needed_hdf5_rec = hdf5_recorder_item->updateInfo                   (client_job_status.second.job_status_.rec_hdf5_status_.info_) || update_needed_hdf5_rec;
      update_needed_hdf5_rec = hdf5_recorder_item->updateIsDeleted              (is_deleted) || update_needed_hdf5_rec;

      if (update_needed_hdf5_rec)
      {
        items_to_update.insert(hdf5_recorder_item);
        items_to_update.insert(job_item); // Also update the parent, as it accumulates its information from the children
      }

      // Update the Addon Items
      for (const auto& addon_id_status_pair : client_job_status.second.job_status_.rec_addon_statuses_)
      {
        const std::string& addon_id = addon_id_status_pair.first;

        JobHistoryRecorderItem* addon_item = findAddonItem(job_item, QString::fromStdString(hostname), QString::fromStdString(addon_id));
        if (addon_item == nullptr) continue;

        // convert the Addon Job State to an HDF5 Job state. We can do that, as the HDF5 recorder can do anything an addon can do.
        eCAL::rec::JobState addon_state;
        switch (addon_id_status_pair.second.state_)
        {
        case eCAL::rec::RecAddonJobStatus::State::Recording:
          addon_state = eCAL::rec::JobState::Recording;
          break;
        case eCAL::rec::RecAddonJobStatus::State::Flushing:
          addon_state = eCAL::rec::JobState::Flushing;
          break;
        case eCAL::rec::RecAddonJobStatus::State::FinishedFlushing:
          addon_state = eCAL::rec::JobState::FinishedFlushing;
          break;
        default:
          addon_state = eCAL::rec::JobState::NotStarted;
          break;
        }

        bool addon_still_online = false;
        if (recorder_still_online)
        {
          // Unfortunatelly, the recorder delivers a job status containing all
          // addon states, even those that have crashed or are not available any
          // more for any other reason. The only way to check if the addon is
          // still available is to lookup its status in the "normal" recorder
          // status. If it is not in there, the addon is offline.
          addon_still_online = std::find_if(recorder_status_it->second.first.addon_statuses_.begin(), recorder_status_it->second.first.addon_statuses_.end(),
                                            [&addon_id_status_pair](const eCAL::rec::RecorderAddonStatus& rec_addon_status)
                                            {
                                              return (addon_id_status_pair.first == rec_addon_status.addon_id_);
                                            }) != recorder_status_it->second.first.addon_statuses_.end();
        }

        bool update_needed_addon = false;
        update_needed_addon = addon_item->updateStillOnline        (addon_still_online) || update_needed_addon;
        update_needed_addon = addon_item->updatePid                (process_id) || update_needed_addon;
        update_needed_addon = addon_item->updateLength             ({std::chrono::steady_clock::duration(0), addon_id_status_pair.second.total_frame_count_ }) || update_needed_addon;
        update_needed_addon = addon_item->updateUnflushedFrameCount(addon_id_status_pair.second.unflushed_frame_count_) || update_needed_addon;
        update_needed_addon = addon_item->updateState              (addon_state) || update_needed_addon;
        update_needed_addon = addon_item->updateInfo               (addon_id_status_pair.second.info_) || update_needed_addon;
        update_needed_addon = addon_item->updateIsDeleted          (is_deleted) || update_needed_addon;

        if (update_needed_addon)
        {
          items_to_update.insert(addon_item);
          items_to_update.insert(job_item); // Also update the parent, as it accumulates its information from the children
        }
      }
    }
  }

  for (QAbstractTreeItem* item : items_to_update)
  {
    updateItem(item);
  }
}

void JobHistoryModel::recordJobCreated(const eCAL::rec_server::JobHistoryEntry& job_history_entry)
{
  if (findJobItemById(job_history_entry.local_evaluated_job_config_.GetJobId()))
  {
    // The item already exists
    return;
  }
  else
  {
    // Get the rec status map needed for AddonID -> AddonID resolving
    eCAL::rec_server::RecorderStatusMap_T recorder_status_map = QEcalRec::instance()->recorderStatuses();

    // Create a new Job Tree Item
    JobHistoryJobItem* new_job_item = new JobHistoryJobItem(job_history_entry.local_evaluated_job_config_, job_history_entry.local_start_time_);
    
    for (const auto& client_status : job_history_entry.client_statuses_)
    {
      // HDF5 Item
      JobHistoryRecorderItem* hdf5_item = new JobHistoryRecorderItem(QString::fromStdString(client_status.first));
      hdf5_item->setPid                    (client_status.second.client_pid_);
      hdf5_item->setStillOnline            (client_status.second.client_pid_ > 0);
      hdf5_item->setInfoLastCommandResponse(client_status.second.info_last_command_response_);
      hdf5_item->setLength                 ({client_status.second.job_status_.rec_hdf5_status_.total_length_, client_status.second.job_status_.rec_hdf5_status_.total_frame_count_ });
      hdf5_item->setUnflushedFrameCount    (client_status.second.job_status_.rec_hdf5_status_.unflushed_frame_count_);
      hdf5_item->setState                  (client_status.second.job_status_.state_);
      hdf5_item->setUploadStatus           (client_status.second.job_status_.upload_status_);
      hdf5_item->setInfo                   (client_status.second.job_status_.rec_hdf5_status_.info_);

      new_job_item->insertChild(hdf5_item);

      // Addon Items
      for (const auto& rec_addon_job_status : client_status.second.job_status_.rec_addon_statuses_)
      {
        // Resolve the addon ID to an addon name.
        std::string addon_name = "";
        auto rec_status_it = recorder_status_map.find(client_status.first);
        if (rec_status_it != recorder_status_map.end())
        {
          auto addon_status_it = std::find_if(rec_status_it->second.first.addon_statuses_.begin(), rec_status_it->second.first.addon_statuses_.end(),
                                              [&rec_addon_job_status](const eCAL::rec::RecorderAddonStatus& addon_status) -> bool
                                              {
                                                return (addon_status.addon_id_ == rec_addon_job_status.first);
                                              });

          if (addon_status_it != rec_status_it->second.first.addon_statuses_.end())
          {
            addon_name = addon_status_it->name_;
          }
        }

        // convert the Addon Job State to an HDF5 Job state. We can do that, as the HDF5 recorder can do anything an addon can do.
        eCAL::rec::JobState addon_state;
        switch (rec_addon_job_status.second.state_)
        {
        case eCAL::rec::RecAddonJobStatus::State::Recording:
          addon_state = eCAL::rec::JobState::Recording;
          break;
        case eCAL::rec::RecAddonJobStatus::State::Flushing:
          addon_state = eCAL::rec::JobState::Flushing;
          break;
        case eCAL::rec::RecAddonJobStatus::State::FinishedFlushing:
          addon_state = eCAL::rec::JobState::FinishedFlushing;
          break;
        default:
          addon_state = eCAL::rec::JobState::NotStarted;
          break;
        }

        // Create a new addon item
        JobHistoryRecorderItem* addon_item = new JobHistoryRecorderItem(QString::fromStdString(client_status.first), QString::fromStdString(rec_addon_job_status.first), QString::fromStdString(addon_name));

        hdf5_item->setPid                (client_status.second.client_pid_);
        hdf5_item->setStillOnline(client_status.second.client_pid_ > 0);
        hdf5_item->setLength             ({ std::chrono::steady_clock::duration(0), rec_addon_job_status.second.total_frame_count_ });
        hdf5_item->setUnflushedFrameCount(rec_addon_job_status.second.unflushed_frame_count_);
        hdf5_item->setState              (addon_state);
        hdf5_item->setInfo               (rec_addon_job_status.second.info_);

        new_job_item->insertChild(addon_item);
      }
    }

    insertItem(new_job_item);
  }
}

void JobHistoryModel::setDeleted(int64_t meas_id)
{
  JobHistoryJobItem* job_item = findJobItemById(meas_id);
  if (job_item == nullptr) return;

  if (job_item->updateIsDeleted(true))
    updateItem(job_item);
}

///////////////////////////////////////////
// Helper methods
///////////////////////////////////////////
JobHistoryJobItem* JobHistoryModel::findJobItemById(int64_t job_id)
{
  for (int i = 0; i < rowCount(); i++)
  {
    QAbstractTreeItem* tree_item = root()->child(i);

    if ((tree_item->type() == (int)TreeItemType::JobHistoryJobItem)
      && (static_cast<JobHistoryJobItem*>(tree_item)->jobId() == job_id))
    {
      return static_cast<JobHistoryJobItem*>(tree_item);
    }
  }

  return nullptr;
}

JobHistoryRecorderItem* JobHistoryModel::findRecorderItem(int64_t job_id, const QString& hostname)
{
  JobHistoryJobItem* job_item = findJobItemById(job_id);
  if (job_item != nullptr)
  {
    return findRecorderItem(job_item, hostname);
  }
  else
  {
    return nullptr;
  }
}

JobHistoryRecorderItem* JobHistoryModel::findRecorderItem(JobHistoryJobItem* job_item, const QString& hostname)
{
  for (int i = 0; i < job_item->childCount(); i++)
  {
    QAbstractTreeItem* tree_item = job_item->child(i);

    if ((tree_item->type() == (int)TreeItemType::JobHistoryRecorderItem)
      && (!static_cast<JobHistoryRecorderItem*>(tree_item)->isAddonItem())
      && (static_cast<JobHistoryRecorderItem*>(tree_item)->hostname() == hostname))
    {
      return static_cast<JobHistoryRecorderItem*>(tree_item);
    }
  }

  return nullptr;
}

JobHistoryRecorderItem* JobHistoryModel::findAddonItem(int64_t job_id, const QString& hostname, const QString& addon_id)
{
  JobHistoryJobItem* job_item = findJobItemById(job_id);
  if (job_item != nullptr)
  {
    return findAddonItem(job_item, hostname, addon_id);
  }
  else
  {
    return nullptr;
  }
}

JobHistoryRecorderItem* JobHistoryModel::findAddonItem(JobHistoryJobItem* job_item, const QString& hostname, const QString& addon_id)
{
  for (int i = 0; i < job_item->childCount(); i++)
  {
    QAbstractTreeItem* tree_item = job_item->child(i);

    if ((tree_item->type() == (int)TreeItemType::JobHistoryRecorderItem)
      && (static_cast<JobHistoryRecorderItem*>(tree_item)->isAddonItem())
      && (static_cast<JobHistoryRecorderItem*>(tree_item)->hostname() == hostname)
      && (static_cast<JobHistoryRecorderItem*>(tree_item)->addonId() == addon_id))
    {
      return static_cast<JobHistoryRecorderItem*>(tree_item);
    }
  }

  return nullptr;
}
