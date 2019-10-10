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

#include <QObject>
#include <QTimer>

#include <QProgressDialog>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QApplication>
#include <QIcon>

#include <map>
#include <set>
#include <string>
#include <memory>
#include <chrono>
#include <thread>

#include <rec_core/record_mode.h>
#include <rec_core/recorder_state.h>

#include <rec_core/topic_info.h>
#include <functional>

namespace eCAL { namespace rec { class RecServer; } }

class MonitoringThread;

class QEcalRec : public QObject
{
  Q_OBJECT

public:
  static QEcalRec* instance();
  ~QEcalRec();

////////////////////////////////////
// Instance management            //
////////////////////////////////////
public:
  bool addRecorderInstance(const std::string& hostname, const std::set<std::string>& host_filter = {}, bool omit_dialogs = false);

  bool removeRecorderInstance(const std::string& hostname, bool omit_dialogs = false);

  bool setRecorderInstances(const std::vector<std::pair<std::string, std::set<std::string>>>& host_hostfilter_list, bool omit_dialogs = false);
  std::vector<std::pair<std::string, std::set<std::string>>> recorderInstances() const;

  bool setClientConnectionsEnabled(bool enabled, bool omit_dialogs = false);
  void initiateConnectionShutdown();

  bool clientConnectionsEnabled() const;

signals:
  void recorderInstancesChangedSignal(const std::vector<std::pair<std::string, std::set<std::string>>>& host_hostfilter_list);
  void clientConnectionsEnabledSignal(bool enabled);

////////////////////////////////////
// Requests                       //
////////////////////////////////////
public:

  std::map<std::string, std::pair<bool, std::string>> lastResponses();

  void waitForPendingRequests() const;

  std::map<std::string, bool> areRequestsPending() const;

  bool anyRequestPending() const;

////////////////////////////////////
// Settings                       //
////////////////////////////////////
public:
  bool setMaxPreBufferLength (std::chrono::steady_clock::duration max_pre_buffer_length,             bool omit_dialogs = false);
  bool setPreBufferingEnabled(bool                                pre_buffering_enabled,             bool omit_dialogs = false);
  bool setRecordMode         (eCAL::rec::RecordMode               record_mode,                       bool omit_dialogs = false);
  bool setTopicBlacklist     (const std::set<std::string>&        topic_blacklist,                   bool omit_dialogs = false);
  bool setTopicWhitelist     (const std::set<std::string>&        topic_whitelist,                   bool omit_dialogs = false);
  bool setHostFilter         (const std::string& hostname, const std::set<std::string>& host_filter, bool omit_dialogs = false);

  void setMeasRootDir        (std::string                         meas_root_dir);
  void setMeasName           (std::string                         meas_name);
  void setMaxFileSizeMib     (int                                 max_file_size_mib);
  void setDescription        (std::string                         description);

  std::chrono::steady_clock::duration maxPreBufferLength()                      const;
  bool                                preBufferingEnabled()                     const;
  eCAL::rec::RecordMode               recordMode()                              const;
  std::set<std::string>               topicBlacklist()                          const;
  std::set<std::string>               topicWhitelist()                          const;
  std::set<std::string>               hostFilter(const std::string& hostname)   const;

  std::string                         measRootDir()                             const;
  std::string                         measName()                                const;
  size_t                              maxFileSizeMib()                          const;
  std::string                         description()                             const;

signals:
  void maxPreBufferLengthChangedSignal (std::chrono::steady_clock::duration max_pre_buffer_length);
  void preBufferingEnabledChangedSignal(bool                                pre_buffering_enabled);
  void recordModeChangedSignal         (eCAL::rec::RecordMode               record_mode);
  void topicBlacklistChangedSignal     (std::set<std::string>               topic_blacklist);
  void topicWhitelistChangedSignal     (std::set<std::string>               topic_whitelist);
  void hostFilterChangedSignal         (const std::string& hostname, const std::set<std::string>& host_filter);

  void measRootDirChangedSignal        (std::string                         meas_root_dir);
  void measNameChangedSignal           (std::string                         meas_name);
  void maxFileSizeMibChangedSignal     (int                                 max_file_size_mib);
  void descriptionChangedSignal        (std::string                         description);

////////////////////////////////////
// Commands                       //
////////////////////////////////////
public:
  bool sendRequestConnectToEcal         (bool omit_dialogs = false);
  bool sendRequestDisconnectFromEcal    (bool omit_dialogs = false);
  bool sendRequestSavePreBufferedData   (bool omit_dialogs = false);
  bool sendRequestStartRecording        (bool omit_dialogs = false);
  bool sendRequestStopRecording         (bool omit_dialogs = false);

signals:
  void connectedToEcalStateChangedSignal(bool connected_to_ecal);
  void recordingStateChangedSignal(bool recording);

////////////////////////////////////
// State                          //
////////////////////////////////////
public:
  bool recordersConnectedToEcal() const;
  bool recordersRecording() const;
  std::map<std::string, eCAL::rec::RecorderState> recorderStates() const;

  bool localRecorderInstanceBusy() const;
  eCAL::rec::RecorderState localRecorderInstanceState() const;

signals:
  void stateUpdateSignal(std::map<std::string, eCAL::rec::RecorderState>);

////////////////////////////////////
// Options                        //
////////////////////////////////////
public:
  void setUsingBuiltInRecorderEnabled(bool enabled);
  bool usingBuiltInRecorderEnabled() const;

signals:
  void usingBuiltInRecorderEnabledChangedSignal(bool enabled);

////////////////////////////////////
// Additional methods             //
////////////////////////////////////

public:
  std::map<std::string, eCAL::rec::TopicInfo> monitorTopicInfo() const;
  std::map<std::string, bool> hostsRunningEcalRec() const;

private slots:
  void sendStateUpdate();

private:
  static QWidget* widgetOf(QObject* q_object);

  // TODO: Executing methods in a thread here causes race conditions, as there are still timers (and other stuff) running in the main thread
  // TODO There is also another thread, the monitoring thread, that executes slots in the main thread.
  template <typename ret_t>
  ret_t executeBlockingMethod(std::function<ret_t()> method, QWidget* /*parent*/, bool /*omit_dialog*/)
  {
    //if (omit_dialog)
    //{
      // Directly call the method
      return method();
    //}
    //else
    //{
    //  // Create a progress dialog
    //  QProgressDialog dlg("Please wait...", "cancel", 0, 0, parent);

    //  dlg.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));

    //  dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowCloseButtonHint);
    //  dlg.setCancelButton(nullptr);
    //  dlg.setWindowModality(Qt::WindowModality::ApplicationModal);
    //  dlg.setMinimumDuration(500);
    //  dlg.setValue(0);
    //  dlg.setValue(1);

    //  // Stop other threads that may cause race conditions
    //  state_update_timer_->blockSignals(true);
    //  monitoring_thread_->blockSignals(true);

    //  // Execute method in a thread
    //  QFuture<ret_t> success_future = QtConcurrent::run(method);

    //  // Wait for the method to finish while updating the progress dialog
    //  while (!success_future.isFinished())
    //  {
    //    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    //    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //  }
    //  dlg.close();

    //  // Start other threads again
    //  state_update_timer_->blockSignals(false);
    //  monitoring_thread_->blockSignals(false);

    //  return success_future.result();
    //}
  }

signals:
  void exitSignal();
  void monitorUpdatedSignal(const std::map<std::string, eCAL::rec::TopicInfo>& topic_info_map);

private:
  QEcalRec(QObject* parent = nullptr);
  QEcalRec(const QEcalRec&) = delete;
  QEcalRec& operator=(const QEcalRec&) = delete;

  std::unique_ptr<eCAL::rec::RecServer> rec_server_;

  QTimer* state_update_timer_;
  const int state_update_timer_ms_ = 100;

  std::set<std::string> topic_blacklist_;
  std::set<std::string> topic_whitelist_;

  std::unique_ptr<MonitoringThread> monitoring_thread_;
};
