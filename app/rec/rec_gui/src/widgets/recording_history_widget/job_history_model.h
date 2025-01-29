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

#include <CustomQt/QAbstractTreeModel.h>
#include <map>
#include <QString>

#include <rec_server_core/rec_server_config.h>

#include "job_history_jobitem.h"
#include "job_history_recorderitem.h"

#include <rec_client_core/state.h>
#include <rec_server_core/rec_server_types.h>
#include <rec_server_core/status.h>

#include <map>
#include <string>
#include <memory>

class JobHistoryModel : public QAbstractTreeModel
{
  Q_OBJECT

///////////////////////////////////////////
// Constructor & Destructor
///////////////////////////////////////////
public:
  JobHistoryModel(QObject *parent = nullptr);
  ~JobHistoryModel();

///////////////////////////////////////////
// QAbstractTreeModel overrides
///////////////////////////////////////////
public:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;

///////////////////////////////////////////
// External API
///////////////////////////////////////////
public:

public slots :
  void setRecorderStatuses(const eCAL::rec_server::RecorderStatusMap_T& recorder_statuses, const std::list<eCAL::rec_server::JobHistoryEntry>& job_history);
  void recordJobCreated(const eCAL::rec_server::JobHistoryEntry& job_history_entry);
  void setDeleted(int64_t meas_id);

///////////////////////////////////////////
// Helper methods
///////////////////////////////////////////
private:
  JobHistoryJobItem* findJobItemById(int64_t job_id);

  JobHistoryRecorderItem* findRecorderItem(int64_t job_id, const QString& hostname);
  JobHistoryRecorderItem* findRecorderItem(JobHistoryJobItem* job_item, const QString& hostname);

  JobHistoryRecorderItem* findAddonItem(int64_t job_id, const QString& hostname, const QString& addon_id);
  JobHistoryRecorderItem* findAddonItem(JobHistoryJobItem* job_item, const QString& hostname, const QString& addon_id);


///////////////////////////////////////////
// Column Management
///////////////////////////////////////////
public:
  enum class Columns : int
  {
    JOB,
    ID,
    PROCESS_ID,
    STILL_ONLINE,
    STATUS,
    HOSTNAME,
    ADDON_NAME,
    LOCAL_PATH,
    MAX_HDF5_FILE_SIZE_MIB,
    LENGTH,
    COMMENT,
    UPLOAD,
    INFO,

    COLUMN_COUNT
  };

private:
  const std::map<Columns, QString> column_labels_
  {
    { Columns::JOB,                    "Job" } ,
    { Columns::ID,                     "ID" } ,
    { Columns::PROCESS_ID,             "Process Id" } ,
    { Columns::STILL_ONLINE,           "Still online" } ,
    { Columns::STATUS,                 "Status" } ,
    { Columns::HOSTNAME,               "Host" } ,
    { Columns::ADDON_NAME,             "Addon name" } ,
    { Columns::LOCAL_PATH,             "Path (this PC)" } ,
    { Columns::MAX_HDF5_FILE_SIZE_MIB, "HDF5 file size" } ,
    { Columns::LENGTH,                 "Length" } ,
    { Columns::COMMENT,                "Comment" } ,
    { Columns::UPLOAD,                 "Upload" } ,
    { Columns::INFO,                   "Info" } ,
  };

};