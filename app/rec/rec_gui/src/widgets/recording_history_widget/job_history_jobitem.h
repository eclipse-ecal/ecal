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

#pragma once

#include <CustomQt/QAbstractTreeItem.h>

#include <QString>
#include <QIcon>

#include <chrono>

#include <rec_client_core/state.h>
#include <rec_client_core/job_config.h>

class JobHistoryJobItem : public QAbstractTreeItem
{
///////////////////////////////////////////
// Constructor & Destructor
///////////////////////////////////////////
public:
  JobHistoryJobItem(const eCAL::rec::JobConfig& local_evaluated_job_config, std::chrono::system_clock::time_point local_start_timestamp);
  ~JobHistoryJobItem() override;

///////////////////////////////////////////
// QAbstractTreeItem overrides
///////////////////////////////////////////
public:
  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const override;

  int type() const override;

///////////////////////////////////////////
// Lazy Getters, Setters & Update methods
///////////////////////////////////////////
public:
  int64_t jobId() const;
  std::chrono::system_clock::time_point timestamp() const;
  bool isDeleted() const;

  void setIsDeleted(bool is_deleted);

  bool updateIsDeleted(bool is_deleted);

  eCAL::rec::JobState                                     combinedJobSate()         const;
  std::pair<std::chrono::steady_clock::duration, int64_t> combinedLength()          const;
  int64_t                                                 combinedUnflushedFrames() const;
  eCAL::rec::UploadStatus                                 combinedUploadStatus()    const;
  std::pair<bool, QString>                                combinedInfo()            const;

///////////////////////////////////////////
// Column Management
///////////////////////////////////////////
public:
  enum class Columns : int
  {
    JOB_ID,
    JOB,
    INVOLVED_HOSTS,
    LENGTH,
    STATUS,
    LOCAL_PATH,
    MAX_HDF5_FILE_SIZE_MIB,
    UPLOAD,
    INFO,
  };

///////////////////////////////////////////
// Member Variables
///////////////////////////////////////////
private:
  const eCAL::rec::JobConfig                  local_evaluated_job_config_;
  const std::chrono::system_clock::time_point local_start_timestamp_;
  bool                                        is_deleted_;

///////////////////////////////////////////
// Icons
///////////////////////////////////////////
private:
  const QIcon icon_recording_;
  const QIcon icon_flushing_;
  const QIcon icon_uploading_;
  const QIcon icon_error_;
};