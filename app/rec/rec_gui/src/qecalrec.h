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

#include <string>
#include <set>
#include <map>
#include <chrono>
#include <functional>
#include <memory>

#include <QObject>
#include <QTimer>

#include <rec_client_core/state.h>
#include <rec_client_core/record_mode.h>

#include <rec_client_core/rec_error.h>

#include <rec_server_core/rec_server_config.h>
#include <rec_server_core/rec_server_types.h>
#include <rec_server_core/status.h>

#include <ecal/msg/protobuf/server.h>

// protobuf includes
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800) // disable proto warnings
#endif
#include <ecal/pb/rec/server_service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


namespace eCAL { namespace rec_server { class RecServer; } }

class QEcalRec : public QObject
{
  Q_OBJECT

////////////////////////////////////
// Constructor, Destructor, Singleton
////////////////////////////////////
public:
  static QEcalRec* instance();
  ~QEcalRec();

private:
  QEcalRec(QObject* parent = nullptr);

  QEcalRec(const QEcalRec&)            = delete;
  QEcalRec& operator=(const QEcalRec&) = delete;

////////////////////////////////////
// Client management
////////////////////////////////////
public slots:
  bool setEnabledRecClients(const std::map<std::string, eCAL::rec_server::ClientConfig>& enabled_recorders, bool omit_dialogs = false);
  std::map<std::string, eCAL::rec_server::ClientConfig> enabledRecClients() const;

  bool setHostFilter(const std::string& hostname, const std::set<std::string>& host_filter, bool omit_dialogs = false);
  std::set<std::string> hostFilter(const std::string& hostname) const;

  bool setConnectionToClientsActive(bool active, bool omit_dialogs = false);
  bool connectionToClientsActive() const;

signals:
  void enabledRecClientsChangedSignal(const std::map<std::string, eCAL::rec_server::ClientConfig>& enabled_recorders);
  void connectionToClientsActiveChangedSignal(bool active);

////////////////////////////////////
// Recorder control
////////////////////////////////////
public slots:
  bool connectToEcal         (bool omit_dialogs = false);
  bool disconnectFromEcal    (bool omit_dialogs = false);
  bool savePreBufferedData   (bool omit_dialogs = false);
  bool startRecording        (bool omit_dialogs = false);
  bool stopRecording         (bool omit_dialogs = false);

  bool connectedToEcal()             const;
  bool recording()                   const;
  int64_t currentlyRecordingMeasId() const;

  bool                  anyRequestPending()        const;
  std::set<std::string> hostsWithPendingRequests() const;
  void                  waitForPendingRequests()   const;

signals:
  void connectedToEcalStateChangedSignal(bool connected_to_ecal);
  void recordingStateChangedSignal      (bool recording);
  void recordJobCreatedSignal           (const eCAL::rec_server::JobHistoryEntry& created_job);

////////////////////////////////////
// Status
////////////////////////////////////
public slots:
  eCAL::rec_server::RecorderStatusMap_T recorderStatuses() const;
  eCAL::rec::RecorderStatus builtInRecorderInstanceStatus() const;

  eCAL::rec_server::TopicInfoMap_T topicInfo() const;
  eCAL::rec_server::HostsRunningEcalRec_T hostsRunningEcalRec() const;

  std::list<eCAL::rec_server::JobHistoryEntry> jobHistory() const;

  eCAL::rec_server::RecServerStatus status() const;

signals:
  void recorderStatusUpdateSignal(const eCAL::rec_server::RecorderStatusMap_T& recorder_status_map, const std::list<eCAL::rec_server::JobHistoryEntry>& job_history);
  void monitorUpdatedSignal(const eCAL::rec_server::TopicInfoMap_T& topic_info_map, const eCAL::rec_server::HostsRunningEcalRec_T& hosts_running_ecal_rec);

////////////////////////////////////
// General Client Settings
////////////////////////////////////
public slots:
  void setMaxPreBufferLength (std::chrono::steady_clock::duration max_pre_buffer_length, bool omit_dialogs = false);
  void setPreBufferingEnabled(bool                                       pre_buffering_enabled, bool omit_dialogs = false);
  bool setRecordMode         (eCAL::rec::RecordMode                      record_mode,           bool omit_dialogs = false);
  bool setTopicBlacklist     (const std::set<std::string>&               topic_blacklist,       bool omit_dialogs = false);
  bool setTopicWhitelist     (const std::set<std::string>&               topic_whitelist,       bool omit_dialogs = false);

  std::chrono::steady_clock::duration maxPreBufferLength()  const;
  bool                                preBufferingEnabled() const;
  eCAL::rec::RecordMode               recordMode()          const;
  std::set<std::string>               topicBlacklist()      const;
  std::set<std::string>               topicWhitelist()      const;

signals:
  void maxPreBufferLengthChangedSignal (std::chrono::steady_clock::duration max_pre_buffer_length);
  void preBufferingEnabledChangedSignal(bool                                pre_buffering_enabled);
  void recordModeChangedSignal         (eCAL::rec::RecordMode               record_mode);
  void topicBlacklistChangedSignal     (std::set<std::string>               topic_blacklist);
  void topicWhitelistChangedSignal     (std::set<std::string>               topic_whitelist);

////////////////////////////////////
// Job Settings
////////////////////////////////////
public slots:
  void setMeasRootDir        (const std::string& meas_root_dir);
  void setMeasName           (const std::string& meas_name);
  void setMaxFileSizeMib     (unsigned int max_file_size_mib);
  void setDescription        (const std::string& description);

  std::string  measRootDir()    const;
  std::string  measName()       const;
  unsigned int maxFileSizeMib() const;
  std::string  description()    const;

signals:
  void measRootDirChangedSignal   (std::string  meas_root_dir);
  void measNameChangedSignal      (std::string  meas_name);
  void maxFileSizeMibChangedSignal(unsigned int max_file_size_mib);
  void descriptionChangedSignal   (std::string  description);

////////////////////////////////////
// Server Settings
////////////////////////////////////
public slots:
  bool setUsingBuiltInRecorderEnabled(bool enabled, bool omit_dialogs = false);
  bool usingBuiltInRecorderEnabled() const;

signals:
  void usingBuiltInRecorderEnabledChangedSignal(bool enabled);

////////////////////////////////////
// Measurement Upload
////////////////////////////////////
public slots:
  void setUploadConfig(const eCAL::rec_server::UploadConfig& upload_config);
  eCAL::rec_server::UploadConfig uploadConfig() const;
  int internalFtpServerOpenConnectionsCount() const;
  uint16_t internalFtpServerPort() const;

  eCAL::rec::Error uploadMeasurement(int64_t meas_id);
  bool canUploadMeasurement(int64_t meas_id) const;
  eCAL::rec::Error simulateUploadMeasurement(int64_t meas_id) const;
  int uploadNonUploadedMeasurements(bool omit_dialogs = false);

signals:
  void uploadConfigChanged(const eCAL::rec_server::UploadConfig& upload_config);

////////////////////////////////////
// Comments
////////////////////////////////////
public slots:
  eCAL::rec::Error addCommentWithDialog(int64_t job_id);
  eCAL::rec::Error addComment(int64_t meas_id, const std::string& comment, bool omit_dialogs = false);
  bool canAddComment(int64_t meas_id) const;
  eCAL::rec::Error simulateAddComment(int64_t meas_id) const;

////////////////////////////////////
// Delete measurement
////////////////////////////////////
public slots:
  eCAL::rec::Error deleteMeasurement(int64_t meas_id, bool omit_dialogs = false);
  eCAL::rec::Error deleteMeasurement(std::set<int64_t> meas_ids, bool omit_dialogs = false);

  bool canDeleteMeasurement(int64_t meas_id) const;
  eCAL::rec::Error simulateDeleteMeasurement(int64_t meas_id) const;


signals:
  void measurementDeletedSignal(int64_t measid);

////////////////////////////////////
// Config Save / Load
////////////////////////////////////
public slots:
  bool clearConfig();
  bool saveConfigToFile  (const std::string& path);
  bool loadConfigFromFile(const std::string& path, bool omit_dialogs = false);

  std::string loadedConfigPath() const;
  int loadedConfigVersion() const;
  int nativeConfigVersion() const;

  bool configHasBeenModified() const;

private:
  void updateConfigModified(bool modified);

signals:
  void loadedConfigChangedSignal(const std::string& path, int version);
  void configHasBeenModifiedChangedSignal(bool has_been_modified);

////////////////////////////////////
// GUI Settings
////////////////////////////////////
public slots:
  void setShowDisabledElementsAtEnd(bool show_at_end);
  bool showDisabledElementsAtEnd() const;

  void setAlternatingRowColorsEnabled(bool enabled);
  bool alternatingRowColorsEnabled() const;

signals:
  void showDisabledElementsAtEndChanged(bool show_at_end);
  void alternatingRowColorsEnabledChanged(bool enabled);


////////////////////////////////////
// Auxiliary methods
////////////////////////////////////
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


////////////////////////////////////
// Member Variables
////////////////////////////////////
private:
  std::unique_ptr<eCAL::rec_server::RecServer> rec_server_;

  // Service provider
  std::shared_ptr<eCAL::pb::rec_server::EcalRecServerService> rec_server_service_;
  eCAL::protobuf::CServiceServer<eCAL::pb::rec_server::EcalRecServerService> rec_server_service_server_;

  QTimer* recorder_status_poll_timer_;
  const int recorder_status_poll_time_ms_ = 100;

  QTimer* ecal_state_update_timer_;
  const int ecal_state_update_time_ms_ = 500;

  std::set<std::string> topic_blacklist_;
  std::set<std::string> topic_whitelist_;

  bool show_disabled_elements_at_the_end_;
  bool alternating_row_colors_;

  bool config_has_been_modified_;
};
