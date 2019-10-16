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

#include <QAbstractItemModel>

#include <rec_core/recorder_state.h>

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
    HOST_FILTER,                // Config
    STATE,                      //    State
    BUFFER,                     //    State
    ERROR,                      //    State

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
  QVariant      data       (const QModelIndex &index, int role)                                       const override;
  bool          setData    (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)       override;
  Qt::ItemFlags flags      (const QModelIndex &index)                                                 const override;
  QVariant      headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole)     const override;
  QModelIndex   index      (int row, int column, const QModelIndex &parent = QModelIndex())           const override;
  QModelIndex   index      (const std::string& hostname, int column = 0)                              const;
  QModelIndex   parent     (const QModelIndex &index)                                                 const override;
  int           rowCount   (const QModelIndex &parent = QModelIndex())                                const override;
  int           columnCount(const QModelIndex &parent = QModelIndex())                                const override;

//////////////////////////////////////////////////////
// Setter & Getter
//////////////////////////////////////////////////////
public:
  std::vector<std::pair<std::string, std::set<std::string>>> recorderInstances() const;
  void setRecorderInstances(const std::vector<std::pair<std::string, std::set<std::string>>>& host_hostfilter_pairs);
  void setHostFilter(const std::string& hostname, const std::set<std::string>& host_filter);
  void setAllChecked(bool checked);
  void invertSelection();

  bool isHostRemovable(const QModelIndex& row) const;
  bool isHostRemovable(int row) const;

  bool containsHost(const std::string& hostname) const;
  bool addHost(const std::string& hostname);

  bool removeRow(int row);
  
//////////////////////////////////////////////////////
// External control
//////////////////////////////////////////////////////
public slots:
  void stateUpdate(std::map<std::string, eCAL::rec::RecorderState> recorder_states);
  void monitorUpdated();
  void recordingStateChanged(bool recording);

//////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////
private slots:
  void updateHostFilterWarnings(bool update_view = true);
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
    RecorderEntry()
      : ecal_rec_visible_          (false)
      , connected_to_client_       (false)
      , recording_enabled_         (false)
      , double_host_filter_warning_(false)
      , request_pending_           (false)
      , last_response_             ({true}, "")
    {}

    bool                         ecal_rec_visible_;
    bool                         connected_to_client_;

    bool                         recording_enabled_;
    std::string                  hostname_;
    std::set<std::string>        host_filter_;
    bool                         double_host_filter_warning_;
    bool                         request_pending_;
    eCAL::rec::RecorderState     state_;
    std::pair<bool, std::string> last_response_;
  };

//////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////
private:
  const std::map<Columns, QString> column_labels_
  {
    { Columns::ENABLED,                 "" } ,
    { Columns::HOSTNAME,                "Recorder" } ,
    { Columns::HOST_FILTER,             "Hosts" } ,
    { Columns::ERROR,                   "Error" } ,
    { Columns::STATE,                   "State" } ,
    { Columns::BUFFER,                  "Buffer" } ,
  };

  const QIcon icon_client_connected;
  const QIcon icon_client_disconnected;
  const QIcon icon_waiting;
  const QIcon icon_connected;
  const QIcon icon_disconnected;
  const QIcon icon_recording;
  const QIcon icon_flushing;
  const QIcon icon_error;
  const QIcon icon_warning;

  std::vector<RecorderEntry> recorder_list_;
};
