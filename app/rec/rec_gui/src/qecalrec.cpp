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

#include "monitoring_thread.h"

#include <functional>

#include <QMessageBox>

#include <ecal/ecal.h>

QEcalRec::QEcalRec(QObject* parent)
  : QObject(parent)
  , rec_server_(std::make_unique<eCAL::rec::RecServer>())
{
  addRecorderInstance(eCAL::Process::GetHostName(), {}, true);

  setMeasRootDir("$TARGET{OSSELECT WIN \"C:\" LINUX \"$TARGET{ENV HOME}\"}/ecal_meas");
  setMeasName("${TIME}_measurement");
  setDescription(
R"(---------------------- description -------------------------
Car: 
Scenario: 
Location: 
Driver: 
Date: ${TIME %F %R}
---------------------- comment -----------------------------
)"
  );
  setMaxFileSizeMib(100);

  state_update_timer_ = new QTimer(this);
  connect(state_update_timer_, &QTimer::timeout, this, &QEcalRec::sendStateUpdate);
  state_update_timer_->start(state_update_timer_ms_);

  qRegisterMetaType<std::map<std::string, eCAL::rec::TopicInfo>>("std::map<std::string,eCAL::rec::TopicInfo>");
  monitoring_thread_ = std::make_unique<MonitoringThread>();
  connect(monitoring_thread_.get(), &MonitoringThread::monitorUpdatedSignal, this, &QEcalRec::monitorUpdatedSignal, Qt::ConnectionType::QueuedConnection);
  monitoring_thread_->Start();
}

QEcalRec::~QEcalRec()
{
  monitoring_thread_->Interrupt();
  monitoring_thread_->Join();
}

QEcalRec* QEcalRec::instance()
{
  static QEcalRec* instance = new QEcalRec();
  return instance;
}

////////////////////////////////////
// Instance management            //
////////////////////////////////////

bool QEcalRec::addRecorderInstance(const std::string& hostname, const std::set<std::string>& host_filter, bool omit_dialogs)
{
  auto recorder_instances = recorderInstances();
  recorder_instances.push_back(std::make_pair(hostname, host_filter));
  return setRecorderInstances(recorder_instances, omit_dialogs);
}

bool QEcalRec::removeRecorderInstance(const std::string& hostname, bool omit_dialogs)
{
  auto recorder_instances = recorderInstances();
  auto recorder_to_remove_it = std::find_if(recorder_instances.begin(), recorder_instances.end(), [&hostname](const auto& host_hostfilter_pair) { return host_hostfilter_pair.first == hostname; });
  
  if (recorder_to_remove_it == recorder_instances.end())
  {
    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Cannot remove recorder \"" + QString::fromStdString(hostname) + "\"");
      error_message.exec();
    }
    return false;
  }
  else
  {
    recorder_instances.erase(recorder_to_remove_it);
    return setRecorderInstances(recorder_instances, omit_dialogs);
  }
}


bool QEcalRec::setRecorderInstances(const std::vector<std::pair<std::string, std::set<std::string>>>& host_hostfilter_list, bool omit_dialogs)
{
  bool success = executeBlockingMethod<bool>([this, &host_hostfilter_list]() { return rec_server_->SetRecorderInstances(host_hostfilter_list); }, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit recorderInstancesChangedSignal(host_hostfilter_list);
  }
  else
  {
    emit recorderInstancesChangedSignal(rec_server_->GetRecorderInstances());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Error setting recorder instances");
      error_message.exec();
    }
  }
  return success;
}

std::vector<std::pair<std::string, std::set<std::string>>> QEcalRec::recorderInstances() const { return rec_server_->GetRecorderInstances(); }

bool QEcalRec::setClientConnectionsEnabled(bool enabled, bool omit_dialogs)
{
  bool success = rec_server_->SetClientConnectionsEnabled(enabled);

  if (!success && !omit_dialogs)
  {
    QMessageBox error_message;
    error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
    error_message.setIcon(QMessageBox::Icon::Critical);
    error_message.setWindowTitle("Error");
    error_message.setText(tr("Unable to ") + (enabled ? "connect to clients" : "disconnect from clients" ) + ". Stop the recording and try again.");
    error_message.exec();
  }

  emit clientConnectionsEnabledSignal(clientConnectionsEnabled());

  return success;
}

void QEcalRec::initiateConnectionShutdown()
{
  rec_server_->InitiateConnectionShutdown();

  emit clientConnectionsEnabledSignal   (false);
  emit recordingStateChangedSignal      (false);
  emit connectedToEcalStateChangedSignal(false);
}


bool QEcalRec::clientConnectionsEnabled() const
{
  return rec_server_->AreClientConnectionsEnabled();
}

////////////////////////////////////
// Requests                       //
////////////////////////////////////

std::map<std::string, std::pair<bool, std::string>> QEcalRec::lastResponses() { return rec_server_->GetLastResponses(); }

void QEcalRec::waitForPendingRequests() const                                 { rec_server_->WaitForPendingRequests(); }

std::map<std::string, bool> QEcalRec::areRequestsPending() const              { return rec_server_->AreRequestsPending(); }

bool QEcalRec::anyRequestPending() const                                      { return rec_server_->IsAnyRequestPending(); }

////////////////////////////////////
// Settings                       //
////////////////////////////////////

bool QEcalRec::setMaxPreBufferLength(std::chrono::steady_clock::duration max_pre_buffer_length, bool omit_dialogs)
{
  std::function<bool()> method = std::bind(&eCAL::rec::RecServer::SetMaxPreBufferLength, rec_server_.get(), max_pre_buffer_length);
  bool success = executeBlockingMethod(method, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit maxPreBufferLengthChangedSignal(max_pre_buffer_length);
  }
  else
  {
    emit maxPreBufferLengthChangedSignal(rec_server_->GetMaxPreBufferLength());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Error setting pre-buffer length");
      error_message.exec();
    }
  }

  return success;
}

bool QEcalRec::setPreBufferingEnabled(bool pre_buffering_enabled, bool omit_dialogs)
{
  std::function<bool()> method = std::bind(&eCAL::rec::RecServer::SetPreBufferingEnabled, rec_server_.get(), pre_buffering_enabled);
  bool success = executeBlockingMethod(method, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit preBufferingEnabledChangedSignal(pre_buffering_enabled);
  }
  else
  {
    emit preBufferingEnabledChangedSignal(rec_server_->GetPreBufferingEnabled());

    if (!omit_dialogs)
    {
      QMessageBox error_message;
      error_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText("Error enabling / disabling pre-buffering");
      error_message.exec();
    }
  }

  return true;
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

bool QEcalRec::setHostFilter(const std::string& hostname, const std::set<std::string>& host_filter, bool omit_dialogs)
{
  bool success = rec_server_->SetHostFilter(hostname, host_filter);
  success = executeBlockingMethod<bool>([this, &hostname, &host_filter]() { return rec_server_->SetHostFilter(hostname, host_filter); }, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit hostFilterChangedSignal(hostname, host_filter);
  }
  else
  {
    emit hostFilterChangedSignal(hostname, rec_server_->GetHostFilter(hostname));

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


void QEcalRec::setMeasRootDir(std::string meas_root_dir)
{
  rec_server_->SetMeasRootDir(meas_root_dir);
  emit measRootDirChangedSignal(meas_root_dir);
}

void QEcalRec::setMeasName(std::string meas_name)
{
  rec_server_->SetMeasName(meas_name);
  emit measNameChangedSignal(meas_name);
}

void QEcalRec::setMaxFileSizeMib(int max_file_size_mib)
{
  rec_server_->SetMaxFileSizeMib(max_file_size_mib);
  emit maxFileSizeMibChangedSignal(max_file_size_mib);
}

void QEcalRec::setDescription(std::string description)
{
  rec_server_->SetDescription(description);
  emit descriptionChangedSignal(description);
}

std::chrono::steady_clock::duration QEcalRec::maxPreBufferLength ()                            const { return rec_server_->GetMaxPreBufferLength();  };
bool                                QEcalRec::preBufferingEnabled()                            const { return rec_server_->GetPreBufferingEnabled(); };
eCAL::rec::RecordMode               QEcalRec::recordMode         ()                            const { return rec_server_->GetRecordMode();          };
std::set<std::string>               QEcalRec::topicBlacklist     ()                            const { return topic_blacklist_;                      };
std::set<std::string>               QEcalRec::topicWhitelist     ()                            const { return topic_whitelist_;                      };
std::set<std::string>               QEcalRec::hostFilter         (const std::string& hostname) const { return rec_server_->GetHostFilter(hostname);  };


std::string                         QEcalRec::measRootDir        ()                            const { return rec_server_->GetMeasRootDir();         };
std::string                         QEcalRec::measName           ()                            const { return rec_server_->GetMeasName();            };
size_t                              QEcalRec::maxFileSizeMib     ()                            const { return rec_server_->GetMaxFileSizeMib();      };
std::string                         QEcalRec::description        ()                            const { return rec_server_->GetDescription();         };

////////////////////////////////////
// Commands                       //
////////////////////////////////////

bool QEcalRec::sendRequestConnectToEcal(bool omit_dialogs)
{
  std::function<bool()> method = std::bind(&eCAL::rec::RecServer::SendRequestConnectToEcal, rec_server_.get());
  bool success = executeBlockingMethod(method, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit clientConnectionsEnabledSignal(true);
    emit connectedToEcalStateChangedSignal(true);
  }
  else
  {
    emit connectedToEcalStateChangedSignal(rec_server_->RecordersConnectedToEcal());

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

bool QEcalRec::sendRequestDisconnectFromEcal(bool omit_dialogs)
{
  std::function<bool()> method = std::bind(&eCAL::rec::RecServer::SendRequestDisconnectFromEcal, rec_server_.get());
  bool success = executeBlockingMethod(method, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit connectedToEcalStateChangedSignal(false);
  }
  else
  {
    emit connectedToEcalStateChangedSignal(rec_server_->RecordersConnectedToEcal());

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

bool QEcalRec::sendRequestSavePreBufferedData(bool omit_dialogs)
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

  std::function<bool()> method = std::bind(&eCAL::rec::RecServer::SendRequestSavePreBufferedData, rec_server_.get());
  bool success = executeBlockingMethod(method, widgetOf(sender()), omit_dialogs);
  if (!success)
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

bool QEcalRec::sendRequestStartRecording(bool omit_dialogs)
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
    emit recordingStateChangedSignal(rec_server_->RecordersRecording());
    return false;
  }

  std::function<bool()> method = std::bind(&eCAL::rec::RecServer::SendRequestStartRecording, rec_server_.get());
  bool success = executeBlockingMethod(method, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit clientConnectionsEnabledSignal(true);
    emit connectedToEcalStateChangedSignal(true);
    emit recordingStateChangedSignal(true);
  }
  else
  {
    emit recordingStateChangedSignal(rec_server_->RecordersRecording());

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

bool QEcalRec::sendRequestStopRecording(bool omit_dialogs)
{
  std::function<bool()> method = std::bind(&eCAL::rec::RecServer::SendRequestStopRecording, rec_server_.get());
  bool success = executeBlockingMethod(method, widgetOf(sender()), omit_dialogs);

  if (success)
  {
    emit recordingStateChangedSignal(false);
  }
  else
  {
    emit recordingStateChangedSignal(rec_server_->RecordersRecording());

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

////////////////////////////////////
// State                          //
////////////////////////////////////

bool QEcalRec::recordersConnectedToEcal() const                                  { return rec_server_->RecordersConnectedToEcal(); };

bool QEcalRec::recordersRecording() const                                        { return rec_server_->RecordersRecording(); };

std::map<std::string, eCAL::rec::RecorderState> QEcalRec::recorderStates() const { return rec_server_->GetRecorderStates(); };

bool QEcalRec::localRecorderInstanceBusy() const                                 { return rec_server_->IsLocalRecorderInstanceBusy(); };

eCAL::rec::RecorderState QEcalRec::localRecorderInstanceState() const            { return rec_server_->GetLocalRecorderInstanceState(); };

////////////////////////////////////
// Options                        //
////////////////////////////////////
void QEcalRec::setUsingBuiltInRecorderEnabled(bool enabled)
{
  rec_server_->SetUsingBuiltInRecorderEnabled(enabled);
  emit usingBuiltInRecorderEnabledChangedSignal(enabled);

  QMessageBox info_message;
  info_message.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
  info_message.setIcon(QMessageBox::Icon::Information);
  info_message.setWindowTitle("Info");
  info_message.setText("Please uncheck & check the local recorder in the recorder manager again to effectively change it!");
  info_message.exec();
}

bool QEcalRec::usingBuiltInRecorderEnabled() const
{
  return rec_server_->IsUsingBuiltInRecorderEnabled();
}

////////////////////////////////////
// Additional methods             //
////////////////////////////////////

std::map<std::string, eCAL::rec::TopicInfo> QEcalRec::monitorTopicInfo() const
{
  return monitoring_thread_->topicInfoMap();
}

std::map<std::string, bool> QEcalRec::hostsRunningEcalRec() const
{
  return monitoring_thread_->hostsRunningEcalRec();
}

void QEcalRec::sendStateUpdate()
{
  emit stateUpdateSignal(rec_server_->GetRecorderStates());
}

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
