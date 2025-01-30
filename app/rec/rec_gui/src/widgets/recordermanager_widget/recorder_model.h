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

#include <QAbstractItemModel>

#include <rec_client_core/state.h>
#include <rec_server_core/rec_server_config.h>
#include <rec_server_core/rec_server_types.h>

#include <vector>
#include <QIcon>

class RecorderModel : public QAbstractItemModel
{
 Q_OBJECT

public:
  enum class Columns : int
  {
    ////////////////////////////////////////////////////////////////////////////
    // ATTENTION:
    // When changing this enum, the emitDataChanged* methods have to be adapted!
    ////////////////////////////////////////////////////////////////////////////
    ENABLED,                    // Config
    HOSTNAME,                   //    State
    ADDON_ID,                   //    State
    ADDON_EXECUTABLE_PATH,      //    State
    PROCESS_ID,                 //    State
    HOST_FILTER,                // Config
    TIMESTAMP,                  //    State, Time (updateTimeWarning depends on it!)
    TIME_ERROR,                 //    State, Time (updateTimeWarning depends on it!)
    BUFFER,                     //    State
    STATE,                      //    State
    INFO,                       //    State
     
    COLUMN_COUNT
  };

//////////////////////////////////////////////////////
// Constructor & destructor
//////////////////////////////////////////////////////
public:
  explicit RecorderModel(QObject *parent = 0);
  ~RecorderModel();

//////////////////////////////////////////////////////
// Reimplemented from QAbstractItemModel
//////////////////////////////////////////////////////
public:
  QVariant      data       (const QModelIndex &index, int role)                                         const override;
  bool          setData    (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)         override;
  Qt::ItemFlags flags      (const QModelIndex &index)                                                   const override;
  QVariant      headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole)       const override;
  QModelIndex   index      (int row, int column, const QModelIndex &parent = QModelIndex())             const override;
  QModelIndex   index      (const std::string& hostname, const std::string& addon_name, int column = 0) const;
  QModelIndex   parent     (const QModelIndex &index)                                                   const override;
  int           rowCount   (const QModelIndex &parent = QModelIndex())                                  const override;
  int           columnCount(const QModelIndex &parent = QModelIndex())                                  const override;

//////////////////////////////////////////////////////
// Setter & Getter
//////////////////////////////////////////////////////
public:
  std::map<std::string, eCAL::rec_server::ClientConfig> enabledRecClients() const;
  void setEnabledRecClients(const std::map<std::string, eCAL::rec_server::ClientConfig>& enabled_recorders);
  void setHostFilter(const std::string& hostname, const std::set<std::string>& host_filter);
  void setAllChecked(bool checked);

  QStringList getAllHosts() const;

  std::string getHostname(int row) const;

//////////////////////////////////////////////////////
// Delete and add rows
//////////////////////////////////////////////////////
public:
  bool isRowRemovable(int row) const;
  std::set<int> getRowsThatWillBeRemoved(const std::set<int>& rows_to_remove);
  bool removeRecorderRows(const std::set<int>& rows);

  bool containsHost(const std::string& hostname) const;
  bool addHost(const std::string& hostname);
  int rowOfHost(const std::string& hostname) const;

  bool containsAddon(const std::string& hostname, const std::string& addon_id) const;
  bool addAddon(const std::string& hostname, const std::string& addon_id);
  int rowOfAddon(const std::string& hostname, const std::string& addon_id) const;
 
//////////////////////////////////////////////////////
// External control
//////////////////////////////////////////////////////
public slots:
  void recorderStatusUpdate(const eCAL::rec_server::RecorderStatusMap_T& recorder_statuses);
  void monitorUpdated(const eCAL::rec_server::TopicInfoMap_T& topic_info_map, const eCAL::rec_server::HostsRunningEcalRec_T& hosts_running_ecal_rec);
  void recordingStateChanged(bool recording);

//////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////
private slots:
  void updateHostFilterWarnings(bool update_view = true);
  void updateTimeErrorWarnings(bool update_view = true);

  void emitDataChangedAll(int row);
  void emitDataChangedColumn(int row, Columns column);
  void emitDataChangedConfig(int row);
  void emitDataChangedState(int row);

//////////////////////////////////////////////////////
// Nested types
//////////////////////////////////////////////////////
private:
  struct RecorderEntry
  {
    RecorderEntry                        ()
      : ecal_rec_visible_in_monitor_     (false)
      , connection_established_          (false)
      , recording_enabled_               (false)
      , pid_                             (0)
      , double_host_filter_warning_      (false)
      , request_pending_                 (false)
      , initialized_                     (false)
      , timestamp_                       (eCAL::Time::ecal_clock::duration(0))
      , time_error_                      (0)
      , time_error_warning_              (false)
      , pre_buffer_length_               {0, std::chrono::steady_clock::duration(0)}
      , recording_                       (false)
      , info_                            ({true}, "")
    {}

    bool                         ecal_rec_visible_in_monitor_;
    bool                         connection_established_;

    bool                         recording_enabled_;
    std::string                  hostname_;
    std::string                  addon_id_;
    std::string                  addon_name_;
    std::string                  addon_execuable_path_;
    int32_t                      pid_;
    std::set<std::string>        host_filter_;
    bool                         double_host_filter_warning_;
    bool                         request_pending_;

    // Status
    bool                                                    initialized_;
    eCAL::Time::ecal_clock::time_point                      timestamp_;
    eCAL::Time::ecal_clock::duration                        time_error_;
    bool                                                    time_error_warning_;
    std::pair<int64_t, std::chrono::steady_clock::duration> pre_buffer_length_;
    std::set<std::string>                                   subscribed_topics_;
    bool                                                    recording_;
    std::pair<bool, std::string>                            info_;
  };

//////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////
private:
  const std::map<Columns, QString> column_labels_
  {
    { Columns::ENABLED,                 "" } ,
    { Columns::HOSTNAME,                "Recorder" } ,
    { Columns::ADDON_ID,                "Addon ID" } ,
    { Columns::ADDON_EXECUTABLE_PATH,   "Addon Executable" } ,
    { Columns::PROCESS_ID,              "Process Id" } ,
    { Columns::HOST_FILTER,             "Hosts" } ,
    { Columns::INFO,                    "Info" } ,
    { Columns::STATE,                   "State" } ,
    { Columns::TIMESTAMP,               "Time" } ,
    { Columns::TIME_ERROR,              "Time error" } ,
    { Columns::BUFFER,                  "Buffer" } ,
  };

  const QIcon icon_client_connected;
  const QIcon icon_client_connected_warning;
  const QIcon icon_client_connected_error;
  const QIcon icon_client_disconnected;
  const QIcon icon_addon_connected;
  const QIcon icon_addon_connected_warning;
  const QIcon icon_addon_connected_error;
  const QIcon icon_addon_disconnected;
  const QIcon icon_waiting;
  const QIcon icon_connected;
  const QIcon icon_disconnected;
  const QIcon icon_recording;
  const QIcon icon_flushing;
  const QIcon icon_error;
  const QIcon icon_warning;

  std::vector<RecorderEntry> recorder_list_;
};
