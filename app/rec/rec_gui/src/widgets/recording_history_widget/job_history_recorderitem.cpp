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

#include "job_history_recorderitem.h"

#include <CustomQt/QBytesToPrettyStringUtils.h>

#include "models/tree_item_types.h"
#include "models/item_data_roles.h"

#include <QFont>

///////////////////////////////////////////
// Constructor & Destructor
///////////////////////////////////////////

JobHistoryRecorderItem::JobHistoryRecorderItem(const QString& hostname)
  : JobHistoryRecorderItem(hostname, "", "")
{}

JobHistoryRecorderItem::JobHistoryRecorderItem(const QString& hostname, const QString& addon_id, const QString& addon_name)
  : icon_addon_                (":/ecalicons/ADDON")
  , icon_addon_offline_        (":/ecalicons/ADDON_DISCONNECTED")
  , icon_addon_error_          (":/ecalicons/ADDON_ERROR")
  , icon_host_                 (":/ecalicons/HOST")
  , icon_host_offline_         (":/ecalicons/HOST_DISCONNECTED")
  , icon_host_error_           (":/ecalicons/HOST_ERROR")
  , icon_error_                (":/ecalicons/ERROR")
  , icon_recording_            (":/ecalicons/RECORD")
  , icon_flushing_             (":/ecalicons/SAVE_TO_DISK")
  , icon_uploading_            (":/ecalicons/UPLOAD")
  , hostname_                  (hostname)
  , addon_id_                  (addon_id)
  , addon_name_                (addon_name)
  , pid_                       (-1)
  , still_online_              (false)
  , info_last_command_response_{true, ""}
  , length_                    { std::chrono::steady_clock::duration(0), 0 }
  , unflushed_frame_count_     (0)
  , state_                     (eCAL::rec::JobState::NotStarted)
  , info_                      {true, ""}
  , is_deleted_                (false)
{}

JobHistoryRecorderItem::~JobHistoryRecorderItem()
{}

///////////////////////////////////////////
// QAbstractTreeItem overrides
///////////////////////////////////////////
QVariant JobHistoryRecorderItem::data(int column, Qt::ItemDataRole role) const
{
  if (column == (int)Columns::RECORDER_NAME)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return hostname_ + (addon_id_.isEmpty() ? "" : " (" + (addon_name_.isEmpty() ? addon_id_ : addon_name_) + ")");
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (isAddonItem())
      {
        if (still_online_)
        {
          auto displayed_info = displayedInfo();

          if (displayed_info.first)
            return icon_addon_;
          else
            return icon_addon_error_;
        }
        else
        {
          return icon_addon_offline_;
        }
      }
      else
      {
        if (still_online_)
        {
          auto displayed_info = displayedInfo();

          if (displayed_info.first)
            return icon_host_;
          else
            return icon_host_error_;
        }
        else
        {
          return icon_host_offline_;
        }
      }
    }
  }

  else if (column == (int)Columns::HOSTNAME)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return hostname_;
    }
  }

  else if (column == (int)Columns::ADDON_ID)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return addon_id_;
    }
  }

  else if (column == (int)Columns::ADDON_NAME)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return addon_name_;
    }
  }

  else if (column == (int)Columns::PROCESS_ID)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return pid_;
    }
  }

  else if (column == (int)Columns::STILL_ONLINE)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return still_online_ ? "Yes" : "No";
    }
  }

  else if (column == (int)Columns::LENGTH)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      if (isAddonItem())
      {
        return QString::number(length_.second) + " frames";
      }
      else
      {
        return QString::number(std::chrono::duration_cast<std::chrono::duration<double>>(length_.first).count(), 'f', 1) + " s / "
          + QString::number(length_.second) + " frames";
      }
    }
    else if (role == ItemDataRoles::SortRole)
    {
      return static_cast<qint64>(length_.second);
    }
  }

  else if (column == (int)Columns::STATUS)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      switch (state_)
      {
      case eCAL::rec::JobState::NotStarted:
        return QString("Not started");
        break;
      case eCAL::rec::JobState::Recording:
        return QString("Recording");
        break;
      case eCAL::rec::JobState::Flushing:
        return QString("Flushing (") + QString::number(unflushed_frame_count_) + QString(" frames)");
        break;
      case eCAL::rec::JobState::FinishedFlushing:
        return QString("Finished");
        break;
      case eCAL::rec::JobState::Uploading:
        return "Uploading (" + bytesToPrettyString(upload_status_.bytes_uploaded_) + " of " + bytesToPrettyString(upload_status_.bytes_total_size_) + ")";
        break;
      case eCAL::rec::JobState::FinishedUploading:
        return QString("Finished Uploading (") + bytesToPrettyString(upload_status_.bytes_uploaded_) + ")";
        break;
      default:
        break;
      }
    }
    if (role == Qt::ItemDataRole::DecorationRole)
    {
      switch (state_)
      {
      case eCAL::rec::JobState::NotStarted:
        break;
      case eCAL::rec::JobState::Recording:
        return icon_recording_;
        break;
      case eCAL::rec::JobState::Flushing:
        return icon_flushing_;
        break;
      case eCAL::rec::JobState::FinishedFlushing:
        break;
      case eCAL::rec::JobState::Uploading:
        return icon_uploading_;
        break;
      case eCAL::rec::JobState::FinishedUploading:
        break;
      default:
        break;
      }
    }
  }

  else if (column == (int)Columns::INFO)
  {
    const std::pair<bool, std::string>& info = displayedInfo();

    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return QString::fromStdString(info.second);
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (!info.first)
        return icon_error_;
    }
  }

  if (role == Qt::ItemDataRole::ForegroundRole)
  {
    if (is_deleted_)
      return QColor(127, 127, 127);
  }
  else if (role == Qt::ItemDataRole::FontRole)
  {
    if (is_deleted_ && (column != (int)Columns::INFO))
    {
      QFont font;
      font.setStrikeOut(true);
      return font;
    }
  }

  // Fallback to the DisplayRole
  if ((role == ItemDataRoles::SortRole)
    || (role == ItemDataRoles::FilterRole)
    || (role == Qt::ItemDataRole::ToolTipRole))
  {
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  return QVariant();
}

int JobHistoryRecorderItem::type() const
{
  return (int)TreeItemType::JobHistoryRecorderItem;
}

///////////////////////////////////////////
// Lazy Getters & Setters
///////////////////////////////////////////

const QString&                                          JobHistoryRecorderItem::hostname()                const { return hostname_; }
const QString&                                          JobHistoryRecorderItem::addonId()                 const { return addon_id_; }
const QString&                                          JobHistoryRecorderItem::addonName()               const { return addon_name_; }
bool                                                    JobHistoryRecorderItem::isAddonItem()             const { return !addon_id_.isEmpty(); }

int                                                     JobHistoryRecorderItem::process_id()                     const { return pid_; }
bool                                                    JobHistoryRecorderItem::stillOnline()             const { return still_online_; }
std::pair<bool, std::string>                            JobHistoryRecorderItem::infoLastCommandResponse() const { return info_last_command_response_; }
std::pair<std::chrono::steady_clock::duration, int64_t> JobHistoryRecorderItem::length()                  const { return length_; }
int64_t                                                 JobHistoryRecorderItem::unflushedFrameCount()     const { return unflushed_frame_count_; }
eCAL::rec::JobState                                     JobHistoryRecorderItem::state()                   const { return state_; }
eCAL::rec::UploadStatus                                 JobHistoryRecorderItem::uploadStatus()            const { return upload_status_; }
std::pair<bool, std::string>                            JobHistoryRecorderItem::info()                    const { return info_; }
bool                                                    JobHistoryRecorderItem::isDeleted()               const { return is_deleted_; }
const std::pair<bool, std::string>&                     JobHistoryRecorderItem::displayedInfo()           const
{
  // Error case
  if (!info_.first)
    return info_;
  else if (!info_last_command_response_.first)
    return info_last_command_response_;
  else if (!upload_status_.info_.first)
    return upload_status_.info_;

  // No-error case
  else if (!info_last_command_response_.second.empty())
    return info_last_command_response_;
  else if (!upload_status_.info_.second.empty())
    return upload_status_.info_;
  else
    return info_;
}

void JobHistoryRecorderItem::setPid                    (int process_id)                                                               { pid_ = process_id; }
void JobHistoryRecorderItem::setStillOnline            (bool still_online)                                                     { still_online_ = still_online; }
void JobHistoryRecorderItem::setInfoLastCommandResponse(const std::pair<bool, std::string>& info_last_command_response)        { info_last_command_response_ = info_last_command_response; }
void JobHistoryRecorderItem::setLength                 (const std::pair<std::chrono::steady_clock::duration, int64_t>& length) { length_ = length; }
void JobHistoryRecorderItem::setUnflushedFrameCount    (int64_t unflushed_frame_count)                                         { unflushed_frame_count_ = unflushed_frame_count; }
void JobHistoryRecorderItem::setState                  (eCAL::rec::JobState state)                                             { state_ = state; }
void JobHistoryRecorderItem::setUploadStatus           (const eCAL::rec::UploadStatus& upload_status)                          { upload_status_ = upload_status; }
void JobHistoryRecorderItem::setInfo                   (const std::pair<bool, std::string>& info)                              { info_ = info; }
void JobHistoryRecorderItem::setIsDeleted              (bool is_deleted)                                                       { is_deleted_ = is_deleted; }

bool JobHistoryRecorderItem::updatePid(int process_id)
{
  if (pid_ != process_id)
  {
    pid_ = process_id;
    return true;
  }
  return false;
}

bool JobHistoryRecorderItem::updateStillOnline(bool still_online)
{
  if (still_online_ != still_online)
  {
    still_online_ = still_online;
    return true;
  }
  return false;
}

bool JobHistoryRecorderItem::updateInfoLastCommandResponse(const std::pair<bool, std::string>& info_last_command_response)
{
  if (info_last_command_response_ != info_last_command_response)
  {
    info_last_command_response_ = info_last_command_response;
    return true;
  }
  return false;
}

bool JobHistoryRecorderItem::updateLength(const std::pair<std::chrono::steady_clock::duration, int64_t>& length)
{
  if (length_ != length)
  {
    length_ = length;
    return true;
  }
  return false;
}

bool JobHistoryRecorderItem::updateUnflushedFrameCount(int64_t unflushed_frame_count)
{
  if (unflushed_frame_count_ != unflushed_frame_count)
  {
    unflushed_frame_count_ = unflushed_frame_count;
    return true;
  }
  return false;
}

bool JobHistoryRecorderItem::updateState(eCAL::rec::JobState state)
{
  if (state_ != state)
  {
    state_ = state;
    return true;
  }
  return false;
}

bool JobHistoryRecorderItem::updateUploadStatus(const eCAL::rec::UploadStatus& upload_status)
{
  if (upload_status_ != upload_status)
  {
    upload_status_ = upload_status;
    return true;
  }
  return false;
}

bool JobHistoryRecorderItem::updateInfo(const std::pair<bool, std::string>& info)
{
  if (info_ != info)
  {
    info_ = info;
    return true;
  }
  return false;
}

bool JobHistoryRecorderItem::updateIsDeleted(bool is_deleted)
{
  if (is_deleted_ != is_deleted)
  {
    is_deleted_ = is_deleted;
    return true;
  }
  return false;
}
