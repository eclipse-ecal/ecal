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

#include "qecalrec.h"

#include <rec_server_core/rec_server.h>
#include <rec_server_core/status.h>
#include <rec_server_core/ecalstate_helpers.h>

#include <ecal/ecal_process.h>

#include <QMessageBox>
#include <QIcon>
#include <QTimer>
#include <QInputDialog>

#include <service/rec_server_service.h>

#include <algorithm>

////////////////////////////////////
// Constructor, Destructor, Singleton
////////////////////////////////////

QEcalRec::QEcalRec(QObject* parent)
  : QObject(parent)
  , rec_server_(std::make_unique<eCAL::rec_server::RecServer>())
  , show_disabled_elements_at_the_end_(true)
  , alternating_row_colors_(true)
  , config_has_been_modified_(false)
{
  std::map<std::string, eCAL::rec_server::ClientConfig> initial_server_config;

  initial_server_config.emplace(eCAL::Process::GetHostName(), eCAL::rec_server::ClientConfig());

  setEnabledRecClients(initial_server_config);

  setMeasRootDir("$TARGET{OSSELECT WIN \"C:\" LINUX \"$TARGET{ENV HOME}\"}/ecal_meas");
  setMeasName("${TIME}_measurement");
  setDescription(
R"(Date: ${TIME %F %R}
Car: 
Scenario: 
Location: 
Driver: 
)"
  );
  setMaxFileSizeMib(100);

  updateConfigModified(false);

  recorder_status_poll_timer_ = new QTimer(this);
  connect(recorder_status_poll_timer_, &QTimer::timeout, this
        , [this]() 
          {
            emit recorderStatusUpdateSignal(rec_server_->GetRecorderStatuses(), rec_server_->GetJobHistory());
          });

  recorder_status_poll_timer_->start(recorder_status_poll_time_ms_);

  ecal_state_update_timer_ = new QTimer(this);
  connect(ecal_state_update_timer_, &QTimer::timeout, this
        , [this]() 
          {
            auto status = this->status();
            auto config = this->enabledRecClients();
            auto ecalstate = eCAL::rec_server::GetProcessSeverity(status, config);
            eCAL::Process::SetState(ecalstate.first, eCAL_Process_eSeverity_Level::proc_sev_level1, ecalstate.second.c_str());
          });

  ecal_state_update_timer_->start(ecal_state_update_time_ms_);


  qRegisterMetaType<eCAL::rec_server::TopicInfoMap_T>                      ("eCAL::rec_server::TopicInfoMap_T");
  qRegisterMetaType<eCAL::rec_server::HostsRunningEcalRec_T>               ("eCAL::rec_server::HostsRunningEcalRec_T");
  qRegisterMetaType<eCAL::rec_server::RecorderStatusMap_T>                 ("eCAL::rec_server::RecorderStatusMap_T");
  qRegisterMetaType<std::string>                                           ("std::string");
  qRegisterMetaType<std::map<std::string, eCAL::rec_server::ClientConfig>> ("std::map<std::string, eCAL::rec_server::ClientConfig>");
  qRegisterMetaType<eCAL::rec::RecordMode>                                 ("eCAL::rec::RecordMode");
  qRegisterMetaType<std::chrono::steady_clock::duration>                   ("std::chrono::steady_clock::duration");

  rec_server_->SetMonitoringUpdateCallback([this](const eCAL::rec_server::TopicInfoMap_T& topic_info_map, const eCAL::rec_server::HostsRunningEcalRec_T& hosts_running_ecal_rec)
                                            {
                                              emit monitorUpdatedSignal(topic_info_map, hosts_running_ecal_rec);
                                            });

  rec_server_service_ = std::shared_ptr<eCAL::pb::rec_server::EcalRecServerService>(new RecServerService());
  rec_server_service_server_.Create(rec_server_service_);
}

QEcalRec::~QEcalRec()
{}

QEcalRec* QEcalRec::instance()
{
  static QEcalRec* instance = new QEcalRec();
  return instance;
}

////////////////////////////////////
// Client management
////////////////////////////////////

bool QEcalRec::setEnabledRecClients(const std::map<std::string, eCAL::rec_server::ClientConfig>& enabled_recorders, bool omit_dialogs)
{
  bool success = executeBlockingMethod<bool>([this, &enabled_recorders]() { return rec_server_->SetEnabledRecClients(enabled_recorders); }, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    updateConfigModified(true);
    emit enabledRecClientsChangedSignal(enabled_recorders);
  }
  else
  {
    emit enabledRecClientsChangedSignal(rec_server_->GetEnabledRecClients());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon (QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon       (QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText       ("Error setting recorder instances");
      error_message.exec();
    }
  }
  return success;
}

std::map<std::string, eCAL::rec_server::ClientConfig> QEcalRec::enabledRecClients() const { return rec_server_->GetEnabledRecClients(); }

bool QEcalRec::setHostFilter(const std::string& hostname, const std::set<std::string>& host_filter, bool omit_dialogs)
{
  bool success = rec_server_->SetHostFilter(hostname, host_filter);
  success = executeBlockingMethod<bool>([this, &hostname, &host_filter]() { return rec_server_->SetHostFilter(hostname, host_filter); }, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    updateConfigModified(true);
    emit enabledRecClientsChangedSignal(rec_server_->GetEnabledRecClients());
  }
  else
  {
    emit enabledRecClientsChangedSignal(rec_server_->GetEnabledRecClients());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Error setting host filter list");
      error_message.exec();
    }
  }
  return success;
}

std::set<std::string> QEcalRec::hostFilter(const std::string& hostname) const { return rec_server_->GetHostFilter(hostname); };

bool QEcalRec::setConnectionToClientsActive(bool active, bool omit_dialogs)
{
  bool success = rec_server_->SetConnectionToClientsActive(active);
  
  if (success)
  {
    if (!active)
    {
      emit connectedToEcalStateChangedSignal(false);
    }
    emit connectionToClientsActiveChangedSignal(active);
  }
  else
  {
    emit connectionToClientsActiveChangedSignal(rec_server_->IsConnectionToClientsActive());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText(tr("Unable to ") + (active ? "connect to clients" : "disconnect from clients") + ". Stop the recording and try again.");
      error_message.exec();
    }
  }

  return success;
}

bool QEcalRec::connectionToClientsActive() const { return rec_server_->IsConnectionToClientsActive(); };

////////////////////////////////////
// Recorder control
////////////////////////////////////

bool QEcalRec::connectToEcal(bool omit_dialogs)
{
  bool success = executeBlockingMethod<bool>([this]() -> bool { return rec_server_->ConnectToEcal(); }, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit connectionToClientsActiveChangedSignal(true);
    emit connectedToEcalStateChangedSignal     (true);
  }
  else
  {
    emit connectedToEcalStateChangedSignal(rec_server_->IsConnectedToEcal());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Error connecting to eCAL");
      error_message.exec();
    }
  }

  return success;
}

bool QEcalRec::disconnectFromEcal(bool omit_dialogs)
{
  bool success = executeBlockingMethod<bool>([this]()->bool { return rec_server_->DisconnectFromEcal(); }, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit connectedToEcalStateChangedSignal(false);
  }
  else
  {
    emit connectedToEcalStateChangedSignal(rec_server_->IsConnectedToEcal());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Error disconnecting from eCAL");
      error_message.exec();
    }
  }

  return success;
}

bool QEcalRec::savePreBufferedData(bool omit_dialogs)
{
  if (measRootDir().empty() && measName().empty())
  {
    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Unable to save buffer. Please enter a measurement root directory and name and try again.");
      error_message.exec();
    }
    return false;
  }

  bool success = executeBlockingMethod<bool>([this]()->bool { return rec_server_->SavePreBufferedData(); }, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit recordJobCreatedSignal(rec_server_->GetJobHistory().back());
  }
  else
  {
    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Error saving pre-buffered data");
      error_message.exec();
    }
  }
  return success;
}

bool QEcalRec::startRecording(bool omit_dialogs)
{
  if (measRootDir().empty() && measName().empty())
  {
    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Unable to start recording. Please enter a measurement root directory and name and try again.");
      error_message.exec();
    }
    emit recordingStateChangedSignal(rec_server_->IsRecording());
    return false;
  }

  bool success = executeBlockingMethod<bool>([this]()->bool { return rec_server_->StartRecording(); } , widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit connectionToClientsActiveChangedSignal(true);
    emit connectedToEcalStateChangedSignal(true);
    emit recordingStateChangedSignal(true);
    emit recordJobCreatedSignal(rec_server_->GetJobHistory().back());
  }
  else
  {
    emit recordingStateChangedSignal(rec_server_->IsRecording());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Error starting recording");
      error_message.exec();
    }
  }

  return success;
}

bool QEcalRec::stopRecording(bool omit_dialogs)
{
  bool success = executeBlockingMethod<bool>([this]()->bool { return rec_server_->StopRecording(); }, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit recordingStateChangedSignal(false);
  }
  else
  {
    emit recordingStateChangedSignal(rec_server_->IsRecording());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Error stopping recording");
      error_message.exec();
    }
  }

  return success;
}

bool QEcalRec::connectedToEcal() const { return rec_server_->IsConnectedToEcal(); };

bool QEcalRec::recording() const { return rec_server_->IsRecording(); };

int64_t QEcalRec::currentlyRecordingMeasId() const { return rec_server_->GetCurrentlyRecordingMeasId(); }

bool QEcalRec::anyRequestPending() const { return rec_server_->IsAnyRequestPending(); }

std::set<std::string> QEcalRec::hostsWithPendingRequests() const { return rec_server_->GetHostsWithPendingRequests(); }

void QEcalRec::waitForPendingRequests() const { rec_server_->WaitForPendingRequests(); }

////////////////////////////////////
// Status
////////////////////////////////////

eCAL::rec_server::RecorderStatusMap_T                   QEcalRec::recorderStatuses()              const { return rec_server_->GetRecorderStatuses(); };

eCAL::rec::RecorderStatus                               QEcalRec::builtInRecorderInstanceStatus() const { return rec_server_->GetBuiltInRecorderInstanceStatus(); };

eCAL::rec_server::TopicInfoMap_T                        QEcalRec::topicInfo()                     const { return rec_server_->GetTopicInfo(); }

eCAL::rec_server::HostsRunningEcalRec_T                 QEcalRec::hostsRunningEcalRec()           const { return rec_server_->GetHostsRunningEcalRec(); }

std::list<eCAL::rec_server::JobHistoryEntry>            QEcalRec::jobHistory()                    const { return rec_server_->GetJobHistory(); }

eCAL::rec_server::RecServerStatus                       QEcalRec::status()                        const { return rec_server_->GetStatus(); }

////////////////////////////////////
// General Client Settings
////////////////////////////////////

void QEcalRec::setMaxPreBufferLength(std::chrono::steady_clock::duration max_pre_buffer_length, bool omit_dialogs)
{
  executeBlockingMethod<void>([this, max_pre_buffer_length]()->void { rec_server_->SetMaxPreBufferLength(max_pre_buffer_length); }, widgetOf(sender()), omit_dialogs);

  updateConfigModified(true);
  emit maxPreBufferLengthChangedSignal(max_pre_buffer_length);
}

void QEcalRec::setPreBufferingEnabled(bool pre_buffering_enabled, bool omit_dialogs)
{
  executeBlockingMethod<void>([this, pre_buffering_enabled]()->void { rec_server_->SetPreBufferingEnabled(pre_buffering_enabled); }, widgetOf(sender()), omit_dialogs);

  updateConfigModified(true);
  emit preBufferingEnabledChangedSignal(pre_buffering_enabled);
}

bool QEcalRec::setRecordMode(eCAL::rec::RecordMode record_mode, bool omit_dialogs)
{
  bool success;
  switch (record_mode)
  {
  case eCAL::rec::RecordMode::Blacklist:
    success = executeBlockingMethod<bool>([this, record_mode]() { return rec_server_->SetRecordMode(record_mode, topic_blacklist_); }, widgetOf(sender()), omit_dialogs);
    break;
  case eCAL::rec::RecordMode::Whitelist:
    success = executeBlockingMethod<bool>([this, record_mode]() { return rec_server_->SetRecordMode(record_mode, topic_whitelist_); }, widgetOf(sender()), omit_dialogs);
    break;
  default:
    success = executeBlockingMethod<bool>([this, record_mode]() { return rec_server_->SetRecordMode(record_mode); }, widgetOf(sender()), omit_dialogs);
    break;
  }

  if (success)
  {
    updateConfigModified(true);
    emit recordModeChangedSignal(record_mode);
  }
  else
  {
    emit recordModeChangedSignal(rec_server_->GetRecordMode());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Error setting record mode");
      error_message.exec();
    }
  }

  return success;
}

bool QEcalRec::setTopicBlacklist(const std::set<std::string>& topic_blacklist, bool omit_dialogs)
{
  if (rec_server_->GetRecordMode() == eCAL::rec::RecordMode::Blacklist)
  {
    bool success = false;
    success = executeBlockingMethod<bool>([this, &topic_blacklist]() { return rec_server_->SetRecordMode(rec_server_->GetRecordMode(), topic_blacklist); }, widgetOf(sender()), omit_dialogs);

    if (success)
    {
      topic_blacklist_ = topic_blacklist;
      updateConfigModified(true);
      emit topicBlacklistChangedSignal(topic_blacklist_);
    }
    else
    {
      emit topicBlacklistChangedSignal(topic_blacklist_);

      if (!omit_dialogs)
      {
        QMessageBox error_message;
        error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
        error_message.setIcon(QMessageBox::Icon::Critical);
        error_message.setWindowTitle("Error");
        error_message.setText("Error setting topic blacklist");
        error_message.exec();
      }
    }

    return success;
  }
  else
  {
    topic_blacklist_ = topic_blacklist;
    emit topicBlacklistChangedSignal(topic_blacklist_);
    return true;
  }
}

bool QEcalRec::setTopicWhitelist(const std::set<std::string>& topic_whitelist, bool omit_dialogs)
{
  if (rec_server_->GetRecordMode() == eCAL::rec::RecordMode::Whitelist)
  {
    bool success = false;
    success = executeBlockingMethod<bool>([this, &topic_whitelist]() { return rec_server_->SetRecordMode(rec_server_->GetRecordMode(), topic_whitelist); }, widgetOf(sender()), omit_dialogs);

    if (success)
    {
      topic_whitelist_ = topic_whitelist;
      updateConfigModified(true);
      emit topicWhitelistChangedSignal(topic_whitelist_);
    }
    else
    {
      emit topicWhitelistChangedSignal(topic_whitelist_);

      if (!omit_dialogs)
      {
        QMessageBox error_message;
        error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
        error_message.setIcon(QMessageBox::Icon::Critical);
        error_message.setWindowTitle("Error");
        error_message.setText("Error setting topic whitelist");
        error_message.exec();
      }
    }

    return success;
  }
  else
  {
    topic_whitelist_ = topic_whitelist;
    emit topicWhitelistChangedSignal(topic_whitelist_);
    return true;
  }
}

std::chrono::steady_clock::duration QEcalRec::maxPreBufferLength () const { return rec_server_->GetMaxPreBufferLength();  };
bool                                QEcalRec::preBufferingEnabled() const { return rec_server_->GetPreBufferingEnabled(); };
eCAL::rec::RecordMode               QEcalRec::recordMode         () const { return rec_server_->GetRecordMode();          };
std::set<std::string>               QEcalRec::topicBlacklist     () const { return topic_blacklist_;                      };
std::set<std::string>               QEcalRec::topicWhitelist     () const { return topic_whitelist_;                      };

////////////////////////////////////
// Job Settings
////////////////////////////////////

void QEcalRec::setMeasRootDir(const std::string& meas_root_dir)
{
  updateConfigModified(true);
  rec_server_->SetMeasRootDir(meas_root_dir);
  emit measRootDirChangedSignal(meas_root_dir);
}

void QEcalRec::setMeasName(const std::string& meas_name)
{
  updateConfigModified(true);
  rec_server_->SetMeasName(meas_name);
  emit measNameChangedSignal(meas_name);
}

void QEcalRec::setMaxFileSizeMib(unsigned int max_file_size_mib)
{
  updateConfigModified(true);
  rec_server_->SetMaxFileSizeMib(max_file_size_mib);
  emit maxFileSizeMibChangedSignal(max_file_size_mib);
}

void QEcalRec::setDescription(const std::string& description)
{
  updateConfigModified(true);
  rec_server_->SetDescription(description);
  emit descriptionChangedSignal(description);
}

std::string  QEcalRec::measRootDir   () const { return rec_server_->GetMeasRootDir();    };
std::string  QEcalRec::measName      () const { return rec_server_->GetMeasName();       };
unsigned int QEcalRec::maxFileSizeMib() const { return rec_server_->GetMaxFileSizeMib(); };
std::string  QEcalRec::description   () const { return rec_server_->GetDescription();    };

////////////////////////////////////
// Server Settings
////////////////////////////////////

bool QEcalRec::setUsingBuiltInRecorderEnabled(bool enabled, bool omit_dialogs)
{
  bool success = rec_server_->SetUsingBuiltInRecorderEnabled(enabled);
  
  if (success)
  {
    emit usingBuiltInRecorderEnabledChangedSignal(enabled);
    updateConfigModified(true);
  }
  else
  {
    emit usingBuiltInRecorderEnabledChangedSignal(rec_server_->IsUsingBuiltInRecorderEnabled());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText(tr("Error ") + (enabled ? "enabling" : "disabling") + " built in recorder. Stop the recording and try again.");
      error_message.exec();
    }
  }

  return success;
}

bool QEcalRec::usingBuiltInRecorderEnabled() const
{
  return rec_server_->IsUsingBuiltInRecorderEnabled();
}

////////////////////////////////////
// Measurement Upload
////////////////////////////////////

void QEcalRec::setUploadConfig                       (const eCAL::rec_server::UploadConfig& upload_config)
{
  rec_server_->SetUploadConfig(upload_config);
  updateConfigModified(true);
  emit uploadConfigChanged(upload_config);
}

eCAL::rec_server::UploadConfig QEcalRec::uploadConfig() const                { return rec_server_->GetUploadConfig(); }
int QEcalRec::internalFtpServerOpenConnectionsCount  () const                { return rec_server_->GetInternalFtpServerOpenConnectionCount(); }
uint16_t QEcalRec::internalFtpServerPort             () const                { return rec_server_->GetInternalFtpServerPort(); }

eCAL::rec::Error QEcalRec::uploadMeasurement         (int64_t meas_id)       { return rec_server_->UploadMeasurement(meas_id); }
bool QEcalRec::canUploadMeasurement                  (int64_t meas_id) const { return rec_server_->CanUploadMeasurement(meas_id); }
eCAL::rec::Error QEcalRec::simulateUploadMeasurement (int64_t meas_id) const { return rec_server_->SimulateUploadMeasurement(meas_id); }
int QEcalRec::uploadNonUploadedMeasurements          (bool omit_dialogs)
{
  int num_uploads_triggered = rec_server_->UploadNonUploadedMeasurements();

  if (!omit_dialogs)
  {
    if (num_uploads_triggered > 0)
    {
      QMessageBox info_message;
      info_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      info_message.setIcon(QMessageBox::Icon::Information);
      info_message.setWindowTitle("Upload");
      info_message.setText(tr("Tiggered upload of ") + QString::number(num_uploads_triggered) + " measurements.");
      info_message.exec();
    }
    else
    {
      QMessageBox warning_message;
      warning_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      warning_message.setIcon(QMessageBox::Icon::Warning);
      warning_message.setWindowTitle("Warning");
      warning_message.setText(tr("No measurements to upload!"));
      warning_message.exec();
    }
  }

  return num_uploads_triggered;
}

////////////////////////////////////
// Comments
////////////////////////////////////
eCAL::rec::Error QEcalRec::addCommentWithDialog(int64_t job_id)
{
  QInputDialog add_comment_dialog;
  add_comment_dialog.setWindowTitle("Add comment");
  add_comment_dialog.setLabelText("Enter comment:");
  add_comment_dialog.setWindowIcon(QIcon(":/ecalicons/ADD_FILE"));

  add_comment_dialog.setInputMode(QInputDialog::InputMode::TextInput);
  add_comment_dialog.setOption(QInputDialog::InputDialogOption::UsePlainTextEditForTextInput, true);

  int ret = add_comment_dialog.exec();

  if ((ret == QDialog::DialogCode::Accepted)
    && !add_comment_dialog.textValue().isEmpty())
  {
    return addComment(job_id, add_comment_dialog.textValue().toStdString());
  }
  else
  {
    return eCAL::rec::Error::ErrorCode::ABORTED_BY_USER;
  }
}

eCAL::rec::Error QEcalRec::addComment(int64_t meas_id, const std::string& comment, bool omit_dialogs)
{
  eCAL::rec::Error error = rec_server_->AddComment(meas_id, comment);

  if (error && !omit_dialogs)
  {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText(tr("Failed to add comment"));
      error_message.setInformativeText(tr("Failed to add comment to measurement ") + QString::number(meas_id) + ": " + QString::fromStdString(error.ToString()));
      error_message.exec();
  }
  
  return error;
}
bool QEcalRec::canAddComment                 (int64_t meas_id) const { return rec_server_->CanAddComment(meas_id); }
eCAL::rec::Error QEcalRec::simulateAddComment(int64_t meas_id) const { return rec_server_->SimulateAddComment(meas_id); }

////////////////////////////////////
// Delete measurement
////////////////////////////////////
eCAL::rec::Error QEcalRec::deleteMeasurement(int64_t meas_id, bool omit_dialogs)
{
  return deleteMeasurement(std::set<int64_t> {meas_id}, omit_dialogs);
}

eCAL::rec::Error QEcalRec::deleteMeasurement(std::set<int64_t> meas_ids, bool omit_dialogs)
{
  if (meas_ids.empty())
    return eCAL::rec::Error::ErrorCode::PARAMETER_ERROR;

  if (!omit_dialogs)
  {
    // Ask to delete
    QMessageBox ask_to_delete_messagebox;
    ask_to_delete_messagebox.setIcon(QMessageBox::Icon::Question);
    ask_to_delete_messagebox.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
    ask_to_delete_messagebox.setWindowTitle("Delete from disk");
    ask_to_delete_messagebox.setText("Delete from disk");

    QString informative_text;
    if (meas_ids.size() == 1)
    {
      // find job history element to get the pretty name
      auto job_history = rec_server_->GetJobHistory();
      auto job_history_it = std::find_if(job_history.begin(), job_history.end(), [meas_id = *(meas_ids.begin())] (const eCAL::rec_server::JobHistoryEntry& entry) { return meas_id == entry.local_evaluated_job_config_.GetJobId(); });
      if (job_history_it != job_history.end())
      {
        informative_text = "Do you want to delete the following measurement?\n\n"
          + (job_history_it != job_history.end()
                      ? QString::fromStdString(job_history_it->local_evaluated_job_config_.GetMeasName())
                      : QString::number(*meas_ids.begin()))
          + "\n\nThis will delete the measurement directory on all clients.";
      }
      ask_to_delete_messagebox.setStandardButtons(QMessageBox::Button::Yes | QMessageBox::Button::Cancel);
    }
    else
    {
      informative_text = "Do you want to delete " + QString::number(meas_ids.size()) + " measurements?\nThis will delete the measurement directories on all clients.";
      ask_to_delete_messagebox.setStandardButtons(QMessageBox::Button::YesAll | QMessageBox::Button::Cancel);
    }
    ask_to_delete_messagebox.setInformativeText(informative_text);
    ask_to_delete_messagebox.setDefaultButton(QMessageBox::Button::Cancel);

    int user_choice = ask_to_delete_messagebox.exec();

    if (user_choice == QMessageBox::Button::Cancel)
      return eCAL::rec::Error::ErrorCode::ABORTED_BY_USER;
  }

  // Delete measurements and collect errors
  std::map<int64_t, eCAL::rec::Error> errors;
  for (int64_t meas_id : meas_ids)
  {
    auto error = rec_server_->DeleteMeasurement(meas_id);
    if (error)
      errors.emplace(meas_id, error);
    else
      emit measurementDeletedSignal(meas_id);
  }

  if (!omit_dialogs && (errors.size() > 0))
  {
    // Show error message if any error occured
    QMessageBox error_message;
    error_message.setWindowTitle("Error");
    error_message.setText("Error deleting measurements");
    error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
    error_message.setIcon(QMessageBox::Icon::Critical);


    if (meas_ids.size() > 1)
    {
      auto job_history = rec_server_->GetJobHistory();

      error_message.setInformativeText("Failed to delete " + QString::number(errors.size()) + "measurement" + (errors.size() > 1 ? "s" : "") + ".");

      QString details;
      for(auto error_it = errors.begin(); error_it != errors.end(); error_it++)
      {
        if (error_it != errors.begin())
          details += "\n";

        // find job history element to get the pretty name
        auto job_history_it = std::find_if(job_history.begin(), job_history.end(), [meas_id = error_it->first] (const eCAL::rec_server::JobHistoryEntry& entry) { return meas_id == entry.local_evaluated_job_config_.GetJobId(); });
        if (job_history_it != job_history.end())
        {
          details += QString::fromStdString(job_history_it->local_evaluated_job_config_.GetMeasName()) + " (" + QString::number(error_it->first) + "): ";
        }
        else
        {
          details +=  QString::number(error_it->first) + ": ";
        }

        // Add the error message to the detailed text
        details += QString::fromStdString(error_it->second.ToString());
      }
      error_message.setDetailedText(details);
    }
    else
    {
      error_message.setInformativeText("Failed to delete measurement: \n" + QString::fromStdString(errors.begin()->second.ToString()));
    }

    error_message.exec();
  }

  if (meas_ids.size() == 1)
    return (errors.size() > 0 ? errors.begin()->second : eCAL::rec::Error::ErrorCode::OK);
  else if (errors.size() > 0)
    return eCAL::rec::Error::ErrorCode::GENERIC_ERROR;
  else
    return eCAL::rec::Error::ErrorCode::OK;
}

bool QEcalRec::canDeleteMeasurement                 (int64_t meas_id) const { return rec_server_->CanDeleteMeasurement(meas_id); }
eCAL::rec::Error QEcalRec::simulateDeleteMeasurement(int64_t meas_id) const { return rec_server_->SimulateDeleteMeasurement(meas_id); }


////////////////////////////////////
// Config Save / Load
////////////////////////////////////

bool QEcalRec::clearConfig()
{
  bool success = rec_server_->ClearConfig();

  if (success)
  {
    emit enabledRecClientsChangedSignal  (rec_server_->GetEnabledRecClients());
    emit maxPreBufferLengthChangedSignal (rec_server_->GetMaxPreBufferLength());
    emit preBufferingEnabledChangedSignal(rec_server_->GetPreBufferingEnabled());
    emit recordModeChangedSignal         (rec_server_->GetRecordMode());
    emit measRootDirChangedSignal        (rec_server_->GetMeasRootDir());
    emit measNameChangedSignal           (rec_server_->GetMeasName());
    emit maxFileSizeMibChangedSignal     (rec_server_->GetMaxFileSizeMib());
    emit descriptionChangedSignal        (rec_server_->GetDescription());

    emit loadedConfigChangedSignal(rec_server_->GetLoadedConfigPath(), rec_server_->GetLoadedConfigVersion());

    updateConfigModified(false);
  }
  else
  {
    QMessageBox error_message;
    error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
    error_message.setIcon(QMessageBox::Icon::Critical);
    error_message.setWindowTitle("Error");
    error_message.setText(tr("Error creating empty config. Stop the recording and try again."));
    error_message.exec();
  }
  
  return success;
}

bool QEcalRec::saveConfigToFile(const std::string& path)
{
  bool success = rec_server_->SaveConfigToFile(path);

  if (success)
  {
    emit loadedConfigChangedSignal(rec_server_->GetLoadedConfigPath(), rec_server_->GetLoadedConfigVersion());
    updateConfigModified(false);
  }
  // TODO: Decide if the GUI or QEcalRec should be responsible for showing error messages. At the moment, the GUI displays an error.  

  return success;
}

bool QEcalRec::loadConfigFromFile(const std::string& path, bool omit_dialogs)
{
  bool success = rec_server_->LoadConfigFromFile(path);

  if (success)
  {
    emit connectedToEcalStateChangedSignal     (rec_server_->IsConnectedToEcal());
    emit connectionToClientsActiveChangedSignal(rec_server_->IsConnectionToClientsActive());
    emit enabledRecClientsChangedSignal        (rec_server_->GetEnabledRecClients());
    emit maxPreBufferLengthChangedSignal       (rec_server_->GetMaxPreBufferLength());
    emit preBufferingEnabledChangedSignal      (rec_server_->GetPreBufferingEnabled());
    emit recordModeChangedSignal               (rec_server_->GetRecordMode());

    if (rec_server_->GetRecordMode() == eCAL::rec::RecordMode::Blacklist)
      emit topicBlacklistChangedSignal(rec_server_->GetListedTopics());
    else if (rec_server_->GetRecordMode() == eCAL::rec::RecordMode::Whitelist)
      emit topicWhitelistChangedSignal(rec_server_->GetListedTopics());

    emit measRootDirChangedSignal   (rec_server_->GetMeasRootDir());
    emit measNameChangedSignal      (rec_server_->GetMeasName());
    emit maxFileSizeMibChangedSignal(rec_server_->GetMaxFileSizeMib());
    emit descriptionChangedSignal   (rec_server_->GetDescription());

    emit usingBuiltInRecorderEnabledChangedSignal(rec_server_->IsUsingBuiltInRecorderEnabled());

    emit loadedConfigChangedSignal(rec_server_->GetLoadedConfigPath(), rec_server_->GetLoadedConfigVersion());

    updateConfigModified(false);

    if (!omit_dialogs)
    {
      if (rec_server_->GetLoadedConfigVersion() < rec_server_->GetNativeConfigVersion())
      {
        QMessageBox non_native_config_warning;
        non_native_config_warning.setWindowIcon (QIcon(":/ecalrec/APP_ICON"));
        non_native_config_warning.setIcon       (QMessageBox::Icon::Warning);
        non_native_config_warning.setWindowTitle("Old config");
        non_native_config_warning.setText       ("This config file has been created with an old version of eCAL rec. When saving, the config will be converted to the new format. Old eCAL Rec versions may not be able to open that file.");
        non_native_config_warning.exec();
      }
      else if (rec_server_->GetLoadedConfigVersion() > rec_server_->GetNativeConfigVersion())
      {
        QMessageBox non_native_config_warning;
        non_native_config_warning.setWindowIcon (QIcon(":/ecalrec/APP_ICON"));
        non_native_config_warning.setIcon       (QMessageBox::Icon::Warning);
        non_native_config_warning.setWindowTitle("New config");
        non_native_config_warning.setText       ("This config file has been created with a newer version of eCAL rec. When saving the configuration, all unrecognized settings will be lost.");
        non_native_config_warning.exec();
      }
    }
  }
  //else
  //{
  //  QMessageBox error_message;
  //  error_message.setWindowIcon (QIcon(":/ecalrec/APP_ICON"));
  //  error_message.setIcon       (QMessageBox::Icon::Critical);
  //  error_message.setWindowTitle("Error");
  //  error_message.setText       ("Failed to load configuration file.");
  //  error_message.exec();
  //}
  // TODO: Decide if the GUI or QEcalRec should be responsible for showing error messages. At the moment, the GUI displays an error.

  return success;
}

std::string QEcalRec::loadedConfigPath()    const { return rec_server_->GetLoadedConfigPath(); }
int QEcalRec::loadedConfigVersion()         const { return rec_server_->GetLoadedConfigVersion(); }
int QEcalRec::nativeConfigVersion()         const { return rec_server_->GetNativeConfigVersion(); }


bool QEcalRec::configHasBeenModified() const { return config_has_been_modified_; }

void QEcalRec::updateConfigModified(bool modified)
{
  if (config_has_been_modified_ != modified)
  {
    config_has_been_modified_ = modified;
    emit configHasBeenModifiedChangedSignal(config_has_been_modified_);
  }
}

////////////////////////////////////
// GUI Settings
////////////////////////////////////
void QEcalRec::setShowDisabledElementsAtEnd(bool show_at_end)
{
  if (show_disabled_elements_at_the_end_ != show_at_end)
  {
    show_disabled_elements_at_the_end_ = show_at_end;
    emit showDisabledElementsAtEndChanged(show_at_end);
  }
}

bool QEcalRec::showDisabledElementsAtEnd() const
{
  return show_disabled_elements_at_the_end_;
}


void QEcalRec::setAlternatingRowColorsEnabled(bool enabled)
{
  if (alternating_row_colors_ != enabled)
  {
    alternating_row_colors_ = enabled;
    emit alternatingRowColorsEnabledChanged(enabled);
  }
}

bool QEcalRec::alternatingRowColorsEnabled() const
{
  return alternating_row_colors_;
}

////////////////////////////////////
// Auxiliary methods
////////////////////////////////////

QWidget* QEcalRec::widgetOf(QObject* q_object)
{
  QWidget* widget = nullptr;
  QObject* object = q_object;

  while (object != nullptr)
  {
    widget = qobject_cast<QWidget*>(object);
    if (widget != nullptr)
    {
      break;
    }
    else
    {
      object = object->parent();
    }

    if (object == q_object)
    {
      break;
    }
  }

  return widget;
}
