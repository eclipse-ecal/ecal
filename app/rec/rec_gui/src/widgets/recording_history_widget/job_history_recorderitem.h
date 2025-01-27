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

#pragma once

#include <CustomQt/QAbstractTreeItem.h>

#include <QString>
#include <QIcon>

#include <rec_client_core/state.h>

class JobHistoryRecorderItem : public QAbstractTreeItem
{
///////////////////////////////////////////
// Constructor & Destructor
///////////////////////////////////////////
public:
  JobHistoryRecorderItem(const QString& hostname);
  JobHistoryRecorderItem(const QString& hostname, const QString& addon_id, const QString& addon_name);
  ~JobHistoryRecorderItem() override;

///////////////////////////////////////////
// QAbstractTreeItem overrides
///////////////////////////////////////////
public:
  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const override;
  int type() const override;

///////////////////////////////////////////
// Lazy Getters & Setters
///////////////////////////////////////////
public:
  const QString& hostname () const;
  const QString& addonId  () const;
  const QString& addonName() const;
  bool isAddonItem() const;

  int                                                     process_id()                     const;
  bool                                                    stillOnline()             const;
  std::pair<bool, std::string>                            infoLastCommandResponse() const;
  std::pair<std::chrono::steady_clock::duration, int64_t> length()                  const;
  int64_t                                                 unflushedFrameCount()     const;
  eCAL::rec::JobState                                     state()                   const;
  eCAL::rec::UploadStatus                                 uploadStatus()            const;
  std::pair<bool, std::string>                            info()                    const;
  bool                                                    isDeleted()               const;
  const std::pair<bool, std::string>&                     displayedInfo()           const;

  void setPid                    (int                                   process_id);
  void setStillOnline            (bool                                  still_online);
  void setInfoLastCommandResponse(const std::pair<bool, std::string>&   info_last_command_response);
  void setLength                 (const std::pair<std::chrono::steady_clock::duration, int64_t>& length);
  void setUnflushedFrameCount    (int64_t                               unflushed_frame_count);
  void setState                  (eCAL::rec::JobState                   state);
  void setUploadStatus           (const eCAL::rec::UploadStatus&        upload_status);
  void setInfo                   (const std::pair<bool, std::string>&   info);
  void setIsDeleted              (bool is_deleted);

  bool updatePid                    (int                                 process_id);
  bool updateStillOnline            (bool                                still_online);
  bool updateInfoLastCommandResponse(const std::pair<bool, std::string>& info_last_command_response);
  bool updateLength                 (const std::pair<std::chrono::steady_clock::duration, int64_t>& length);
  bool updateUnflushedFrameCount    (int64_t                             unflushed_frame_count);
  bool updateState                  (eCAL::rec::JobState                 state);
  bool updateUploadStatus           (const eCAL::rec::UploadStatus&      upload_status);
  bool updateInfo                   (const std::pair<bool, std::string>& info);
  bool updateIsDeleted              (bool is_deleted);


///////////////////////////////////////////
// Column Management
///////////////////////////////////////////
public:
  enum class Columns : int
  {
    RECORDER_NAME,
    HOSTNAME,
    ADDON_ID,
    ADDON_NAME,
    PROCESS_ID,
    STILL_ONLINE,
    LENGTH,
    STATUS,
    INFO,
  };

///////////////////////////////////////////
// Member Variables
///////////////////////////////////////////
private:
  // Icons
  const QIcon icon_addon_;
  const QIcon icon_addon_offline_;
  const QIcon icon_addon_error_;
  const QIcon icon_host_;
  const QIcon icon_host_offline_;
  const QIcon icon_host_error_;
  const QIcon icon_error_;
  const QIcon icon_recording_;
  const QIcon icon_flushing_;
  const QIcon icon_uploading_;

  // Element Status
  const QString                                           hostname_;
  const QString                                           addon_id_;
  const QString                                           addon_name_;
  
  int                                                     pid_;
  bool                                                    still_online_;
  std::pair<bool, std::string>                            info_last_command_response_;

  std::pair<std::chrono::steady_clock::duration, int64_t> length_;
  int64_t                                                 unflushed_frame_count_;
  eCAL::rec::JobState                                     state_;
  eCAL::rec::UploadStatus                                 upload_status_;
  std::pair<bool, std::string>                            info_;
  bool                                                    is_deleted_;
};