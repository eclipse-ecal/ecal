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

#include "q_ecal_play.h"

#include <algorithm>
#include <thread>

#include <QTimer>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPixmap>
#include <QIcon>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QApplication>
#include <QCheckBox>

#include "ecal_play_logger.h"

#ifdef WIN32
#include <QWinTaskbarButton>
#endif //WIN32

QEcalPlay::QEcalPlay()
  : ecal_play_()
  , scenarios_modified_(false)
{
  // Start the timer that periodically updates the state
  periodic_update_timer_ = new QTimer(this);
  connect(periodic_update_timer_, &QTimer::timeout, this, &QEcalPlay::periodicStateUpdate);
  periodic_update_timer_->start(40);

  play_service_ = std::shared_ptr<eCAL::pb::play::EcalPlayService>(new EcalPlayService());
  play_service_server_.Create(play_service_);

  // Load default channel mapping setting
  QSettings settings;
  auto action_variant = settings.value("channel_mapping_file_action");
  if (action_variant.isValid())
  {
    channel_mapping_file_action_ = static_cast<ChannelMappingFileAction>(action_variant.toInt());
  }
  else
  {
    channel_mapping_file_action_ = ChannelMappingFileAction::Ask;
  }
}

QEcalPlay::~QEcalPlay()
{}

QEcalPlay* QEcalPlay::instance()
{
  static QEcalPlay* instance = new QEcalPlay();
  return instance;
}

////////////////////////////////////////////////////////////////////////////////
//// Getter                                                                 ////
////////////////////////////////////////////////////////////////////////////////

// Measurement
QString QEcalPlay::measurementPath() const
{
  return ecal_play_.GetMeasurementPath().c_str();
}

QString QEcalPlay::measurementDirectory() const
{
  return QString::fromStdString(ecal_play_.GetMeasurementDirectory());
}

bool QEcalPlay::isMeasurementLoaded() const
{
  return ecal_play_.IsMeasurementLoaded();
}

QString QEcalPlay::description() const
{
  return ecal_play_.GetDescription().c_str();
}

std::chrono::nanoseconds QEcalPlay::measurementLength() const
{
  return ecal_play_.GetMeasurementLength();
}

std::set<std::string> QEcalPlay::channelNames() const
{
  return ecal_play_.GetChannelNames();
}

std::map<std::string, ContinuityReport> QEcalPlay::createContinuityReport() const
{
  return ecal_play_.CreateContinuityReport();
}

std::map<std::string, long long> QEcalPlay::messageCounters() const
{
  return ecal_play_.GetMessageCounters();
}

std::vector<EcalPlayScenario> QEcalPlay::scenarios() const
{
  return ecal_play_.GetScenarios();
}

bool QEcalPlay::scenariosModified() const
{
  return scenarios_modified_;
}

std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> QEcalPlay::measurementBoundaries() const
{
  return ecal_play_.GetMeasurementBoundaries();
}

long long QEcalPlay::frameCount() const
{
  return ecal_play_.GetFrameCount();
}

eCAL::Time::ecal_clock::time_point QEcalPlay::timestampOf(long long frame_index) const
{
  return ecal_play_.GetTimestampOf(frame_index);
}

// Settings
bool QEcalPlay::isRepeatEnabled() const
{
  return ecal_play_.IsRepeatEnabled();
}

double QEcalPlay::playSpeed() const
{
  return ecal_play_.GetPlaySpeed();
}

bool QEcalPlay::isLimitPlaySpeedEnabled() const
{
  return ecal_play_.IsLimitPlaySpeedEnabled();
}

bool QEcalPlay::isFrameDroppingAllowed() const
{
  return ecal_play_.IsFrameDroppingAllowed();
}

bool QEcalPlay::isEnforceDelayAccuracyEnabled() const
{
  return ecal_play_.IsEnforceDelayAccuracyEnabled();
}

QString QEcalPlay::stepReferenceChannel() const
{
  return step_reference_channel_;
}

std::map<std::string, std::string> QEcalPlay::channelMapping() const
{
  return channel_mapping_;
}

std::pair<long long, long long> QEcalPlay::limitInterval() const
{
  return ecal_play_.GetLimitInterval();
}

QEcalPlay::ChannelMappingFileAction QEcalPlay::channelMappingFileAction() const
{
  return channel_mapping_file_action_;
}

std::map<std::string, std::string> QEcalPlay::loadChannelMappingFile(const QString& path) const
{
  return ecal_play_.LoadChannelMappingFile(path.toStdString());
}

void QEcalPlay::loadChannelMappingFileFromFileDialog()
{
  QWidget* caller = widgetOf(sender());

  QString start_dir = "";
  QString measurement_dir = measurementDirectory();
  if (!measurement_dir.isEmpty())
  {
    start_dir = measurement_dir + "/doc/";
  }
  QString channel_mapping_path = QFileDialog::getOpenFileName(caller, "Load Channel Mapping", start_dir, "Text files (*.txt);;All files (*)");
  if (!channel_mapping_path.isEmpty())
  {
    auto channel_mapping = loadChannelMappingFile(channel_mapping_path);
    setChannelMapping(channel_mapping);
  }
}

// State
EcalPlayState QEcalPlay::currentPlayState() const
{
  return ecal_play_.GetCurrentPlayState();
}

bool QEcalPlay::isInitialized() const
{
  return ecal_play_.IsInitialized();
}

bool QEcalPlay::isPlaying() const
{
  return ecal_play_.IsPlaying();
}

bool QEcalPlay::isPaused() const
{
  return ecal_play_.IsPaused();
}

double QEcalPlay::currentPlaySpeed() const
{
  return ecal_play_.GetCurrentPlaySpeed();
}

long long QEcalPlay::lastPublishedFrameIndex() const
{
  return ecal_play_.GetCurrentFrameIndex();
}

eCAL::Time::ecal_clock::time_point QEcalPlay::lastFrameTimestamp() const
{
  return ecal_play_.GetCurrentFrameTimestamp();
}

////////////////////////////////////////////////////////////////////////////////
//// Commands                                                               ////
////////////////////////////////////////////////////////////////////////////////

bool QEcalPlay::loadMeasurementFromFileDialog()
{
  QString last_measurement_folder = measurementDirectory();

  if (last_measurement_folder.isEmpty())
  {
    QSettings settings;
    QVariant last_measurement_folder_variant = settings.value("last_measurement_folder");
    if (last_measurement_folder_variant.isValid())
    {
      last_measurement_folder = last_measurement_folder_variant.toString();
    }
  }

  // Open measurement directory or file. We use the file chooser (not the
  // directory chooser). If the user selects a non-existing file we assume that
  // he wanted to choose the directory. This is a common workaround.

  QFileDialog open_dialog;
  open_dialog.setWindowTitle("Open measurement");
  open_dialog.setFileMode   (QFileDialog::AnyFile);
  open_dialog.setAcceptMode (QFileDialog::AcceptOpen);
  open_dialog.setDirectory  (last_measurement_folder);
  open_dialog.selectFile    ("Measurement directory.");

#ifdef __linux__
  // Disable the native open file dialog, as some Desktop environments are unable to open non-existing files (e.g. Gnome 3)
  open_dialog.setOption(QFileDialog::Option::DontUseNativeDialog);
#endif //__linux__

  int result = open_dialog.exec();

  if (result)
  {
    auto filenames = open_dialog.selectedFiles();
    if (!filenames.empty())
    {
      return loadMeasurement(filenames.front());
    }
  }

  return false;
}

bool QEcalPlay::loadMeasurement(const QString& path, bool suppress_blocking_dialogs)
{
  if (isMeasurementLoaded())
  {
    saveChannelMapping("", suppress_blocking_dialogs);
  }

  QWidget* caller = widgetOf(sender());

  periodic_update_timer_->stop();

  QProgressDialog dlg("Loading measurement...", "cancel", 0, 0, caller);
  QPixmap icon(":/ecalplay/APP_ICON");
  dlg.setWindowIcon(icon);
  dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowCloseButtonHint);
  dlg.setCancelButton(nullptr);
  dlg.setWindowModality(Qt::WindowModality::ApplicationModal);
  dlg.setMinimumDuration(500);
  dlg.setValue(0);
  dlg.setValue(1);

  QString path_to_load;
  if (QFile(path).exists())
  {
    path_to_load = path;
  }
  else
  {
    path_to_load = QDir::toNativeSeparators(QFileInfo(path).path());
  }

  QFuture<bool> success_future = QtConcurrent::run(&ecal_play_, &EcalPlay::LoadMeasurement, path_to_load.toStdString());

  while (!success_future.isFinished())
  {
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  dlg.close();
  bool success = success_future.result();

  if (success)
  {
    scenarios_modified_ = false;
    emit measurementLoadedSignal(path_to_load);
    emit publishersInitStateChangedSignal(false);

    setStepReferenceChannel("");

    // Save last measurment path
    QDir dir(path_to_load);
    QSettings settings;
    settings.setValue("last_measurement_folder", QDir::toNativeSeparators(dir.absolutePath()));

    // Load Default channel mapping
    bool apply_mapping = false;

    // Load channel mapping from the file, but don't tell the user, yet
    QFile default_channel_mapping_file(measurementDirectory() + "/doc/chn_name_mapping.txt");
    QFileInfo default_channel_mapping_file_info(default_channel_mapping_file);
    auto channel_mapping = QEcalPlay::instance()->loadChannelMappingFile(QDir::toNativeSeparators(default_channel_mapping_file_info.absoluteFilePath()));

    // Check if the channel mapping has any relevance.
    if (isChannelMappingRelevant(channel_mapping))
    {
      EcalPlayLogger::Instance()->info("Channel mapping found at " + QDir::toNativeSeparators(default_channel_mapping_file_info.absoluteFilePath()).toStdString());

      // Check if the user has decided to not being asked again
      if (channelMappingFileAction() == ChannelMappingFileAction::Load)
      {
        apply_mapping = true;
      }
      else if ((channelMappingFileAction() == ChannelMappingFileAction::Ask) && !suppress_blocking_dialogs)
      {
        QMessageBox load_channel_mapping_dialog(
          QMessageBox::Icon::Question
          , tr("Load channel mapping?")
          , tr("The measurement contains a channel mapping:\n") + QDir::toNativeSeparators(default_channel_mapping_file_info.absoluteFilePath()) + tr("\n\nDo you want to apply it?\n")
          , QMessageBox::Button::Yes | QMessageBox::Button::No
          , caller);
        load_channel_mapping_dialog.setDefaultButton(QMessageBox::Button::No);
        QCheckBox* remember_checkbox = new QCheckBox(tr("Remember my choice and don't ask again"), &load_channel_mapping_dialog);
        load_channel_mapping_dialog.setCheckBox(remember_checkbox);
        int user_choice = load_channel_mapping_dialog.exec();
        apply_mapping = (user_choice == QMessageBox::Button::Yes);

        if (remember_checkbox->isChecked())
        {
          QEcalPlay::setChannelMappingFileAction((user_choice == QMessageBox::Button::Yes) ? ChannelMappingFileAction::Load : ChannelMappingFileAction::Ignore);
        }
      }
    }
    else
    {
      EcalPlayLogger::Instance()->info("Channel mapping found, but irrelevant: " + QDir::toNativeSeparators(default_channel_mapping_file_info.absoluteFilePath()).toStdString());
    }

    if (apply_mapping)
    {
      QEcalPlay::instance()->setChannelMapping(channel_mapping);
    }
  }
  else 
  {
    if (!suppress_blocking_dialogs)
    {
      QMessageBox error_message(
        QMessageBox::Icon::Critical
        , tr("Error")
        , tr("Unable to load measurement:\n") + QDir::toNativeSeparators(path_to_load)
        , QMessageBox::Button::Ok
        , caller);
      error_message.setWindowIcon(icon);
      error_message.exec();
    }
  }

  periodic_update_timer_->start();
  return success;
}

bool QEcalPlay::saveChannelMappingAs()
{
  QWidget* caller = widgetOf(sender());

  QString start_dir = "";
  QString measurement_directory = measurementDirectory();
  if (!measurement_directory.isEmpty())
  {
    start_dir = measurement_directory + "/doc/chn_name_mapping.txt";
  }
  QString channel_mapping_path = QFileDialog::getSaveFileName(caller, "Export Channel Mapping", start_dir, "Text files (*.txt);;All files (*)");
  if (!channel_mapping_path.isEmpty())
  {
    return saveChannelMapping(channel_mapping_path);
  }
  else
  {
    return false;
  }
}

bool QEcalPlay::saveChannelMapping(const QString& path, bool omit_blocking_dialogs)
{
  QWidget* caller = widgetOf(sender());
  QPixmap icon(":/ecalplay/APP_ICON");

  QFile channel_mapping_file;

  if (path == "")
  {
    QString measurement_directory = measurementDirectory();
    if (measurement_directory.isEmpty())
    {
      EcalPlayLogger::Instance()->error("Error saving Channel mapping to the default path: No measurement loaded or measurement folder unknown.");
      if (!omit_blocking_dialogs)
      {
        QMessageBox error_message(
          QMessageBox::Icon::Critical
          , tr("Error")
          , tr("Error saving Channel mapping to the default path: No measurement loaded or measurement folder unknown.")
          , QMessageBox::Button::Ok
          , caller);
        error_message.setWindowIcon(icon);
        error_message.exec();
      }
      return false;
    }
    channel_mapping_file.setFileName(measurement_directory + "/doc/chn_name_mapping.txt");
  }
  else
  {
    channel_mapping_file.setFileName(path);
  }

  QFileInfo channel_mapping_file_info(channel_mapping_file);

  QDir dir;
  dir.mkpath(channel_mapping_file_info.absoluteDir().absolutePath());

  if (channel_mapping_file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    QTextStream file_stream(&channel_mapping_file);
    for (auto& channel_mapping_pair : channel_mapping_)
    {
      file_stream << channel_mapping_pair.first.c_str() << "\t" << channel_mapping_pair.second.c_str() << "\n";
    }
    channel_mapping_file.close();
    return true;
  }
  else
  {
    EcalPlayLogger::Instance()->error("Error saving channel mapping to " + channel_mapping_file_info.absoluteFilePath().toStdString());
    if (!omit_blocking_dialogs)
    {
      QMessageBox error_message(
        QMessageBox::Icon::Critical
        , tr("Error")
        , tr("Error saving channel mapping to ") + channel_mapping_file_info.absoluteFilePath()
        , QMessageBox::Button::Ok
        , caller);
      error_message.setWindowIcon(icon);
      error_message.exec();
    }
    return false;
  }
}


void QEcalPlay::closeMeasurement(bool omit_blocking_dialogs)
{
  saveChannelMapping("", omit_blocking_dialogs);

  ecal_play_.CloseMeasurement();
  scenarios_modified_ = false;

  emit measurementClosedSignal();
  emit publishersInitStateChangedSignal(false);

  setChannelMapping(std::map<std::string, std::string>());
  setStepReferenceChannel("");
}

bool QEcalPlay::initializePublishers(bool suppress_error_dialog)
{
  QWidget* caller = widgetOf(sender());

  if (channel_mapping_.size() == 0)
  {
    QPixmap icon(":/ecalplay/APP_ICON");

    if (!suppress_error_dialog)
    {
      QMessageBox error_message(
        QMessageBox::Icon::Critical
        , tr("Error")
        , tr("You have to select at least one channel!")
        , QMessageBox::Button::Ok
        , 0);

      error_message.setWindowIcon(QIcon(icon));
      error_message.exec();
    }

    return false;
  }

  periodic_update_timer_->stop();

  QProgressDialog dlg("Connecting to eCAL...", "cancel", 0, 0, caller);
  QPixmap icon(":/ecalplay/APP_ICON");
  dlg.setWindowIcon(icon);
  dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowCloseButtonHint);
  dlg.setCancelButton(nullptr);
  dlg.setWindowModality(Qt::WindowModality::ApplicationModal);
  dlg.setMinimumDuration(500);
  dlg.setValue(0);
  dlg.setValue(1);

  QFuture<bool> success_future = QtConcurrent::run(&ecal_play_, &EcalPlay::InitializePublishers, channel_mapping_);

  while (!success_future.isFinished())
  {
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  dlg.close();
  bool success = success_future.result();

  if (success)
  {
    emit publishersInitStateChangedSignal(true);
  }

  periodic_update_timer_->start();
  return success;
}

bool QEcalPlay::deInitializePublishers()
{
  bool success = ecal_play_.DeInitializePublishers();

  if (success)
  {
    emit publishersInitStateChangedSignal(false);
  }
  return success;
}


bool QEcalPlay::play(long long play_until_index, bool suppress_error_dialog)
{
  bool initialized = ecal_play_.IsInitialized();
  if (!initialized)
  {
    initialized = initializePublishers(suppress_error_dialog);
  }

  if (initialized)
  {
    bool success = ecal_play_.Play(play_until_index);

    if (success)
    {
      last_state_ = currentPlayState();
      emit playStateChangedSignal(last_state_);
    }
    return success;
  }

  return false;
}

bool QEcalPlay::pause()
{
  bool success = ecal_play_.Pause();

  if (success)
  {
    last_state_ = currentPlayState();
    emit playStateChangedSignal(last_state_);
  }
  return success;
}

void QEcalPlay::stop()
{
  pause();
  jumpTo(0);
  deInitializePublishers();
}

bool QEcalPlay::stepForward(bool suppress_error_dialog)
{
  bool initialized = ecal_play_.IsInitialized();
  if (!initialized)
  {
    initialized = initializePublishers(suppress_error_dialog);
  }

  if (initialized)
  {
    bool success = ecal_play_.StepForward();

    if (success)
    {
      last_state_ = currentPlayState();
      emit playStateChangedSignal(last_state_);
    }
    return success;
  }
  
  return false;
}

bool QEcalPlay::stepChannel(bool suppress_error_dialog)
{

  if (step_reference_channel_.isEmpty())
  {
    if (!suppress_error_dialog)
    {
      QWidget* caller = widgetOf(sender());
      QMessageBox error_message(
        QMessageBox::Icon::Critical
        , tr("Error")
        , tr("Please select a channel for stepping, first")
        , QMessageBox::Button::Ok
        , caller);
      QPixmap icon(":/ecalplay/APP_ICON");
      error_message.setWindowIcon(icon);
      error_message.exec();
    }
    return false;
  }

  bool initialized = ecal_play_.IsInitialized();
  if (!initialized)
  {
    initialized = initializePublishers(suppress_error_dialog);
  }

  if (initialized)
  {

    bool success = ecal_play_.PlayToNextOccurenceOfChannel(step_reference_channel_.toStdString());

    if (success)
    {
      last_state_ = currentPlayState();
      emit playStateChangedSignal(last_state_);
    }
    return success;
  }

  return false;
}

bool QEcalPlay::jumpTo(long long index)
{
  bool success = ecal_play_.JumpTo(index);

  if (success)
  {
    last_state_ = currentPlayState();
    emit playStateChangedSignal(last_state_);
  }
  return success;
}

bool QEcalPlay::jumpTo(eCAL::Time::ecal_clock::time_point timestamp)
{
  bool success = ecal_play_.JumpTo(timestamp);

  if (success)
  {
    last_state_ = currentPlayState();
    emit playStateChangedSignal(last_state_);
  }
  return success;
}

void QEcalPlay::exit(bool omit_blocking_dialogs)
{
  closeMeasurement(omit_blocking_dialogs);
  emit exitSignal();
}

////////////////////////////////////////////////////////////////////////////////
//// Setter                                                                 ////
////////////////////////////////////////////////////////////////////////////////

void QEcalPlay::setScenarios(const std::vector<EcalPlayScenario>& scenarios)
{
  bool something_changed = (scenarios_modified_ || (ecal_play_.GetScenarios() != scenarios));

  if (something_changed)
  {
    scenarios_modified_ = true;
    ecal_play_.SetScenarios(scenarios);
    emit scenariosChangedSignal(scenarios);
  }
}

bool QEcalPlay::saveScenariosToDisk(bool suppress_blocking_dialogs)
{
  bool success = ecal_play_.SaveScenariosToDisk();


  if (success)
  {
    scenarios_modified_ = false;
    emit scenariosSavedSignal();
  }
  else
  {
    if (!suppress_blocking_dialogs)
    {
      QWidget* caller = widgetOf(sender());
      QMessageBox error_message(
        QMessageBox::Icon::Critical
        , tr("Error")
        , tr("Unable to save Labels to disk")
        , QMessageBox::Button::Ok
        , caller);
      QPixmap icon(":/ecalplay/APP_ICON");
      error_message.setWindowIcon(icon);
      error_message.exec();
    }
  } 

  return success;
}

void QEcalPlay::setRepeatEnabled(bool enabled)
{
  ecal_play_.SetRepeatEnabled(enabled);

  emit repeatEnabledChangedSignal(enabled);
}

void QEcalPlay::setPlaySpeed(double play_speed)
{
  ecal_play_.SetPlaySpeed(play_speed);

  emit playSpeedChangedSignal(play_speed);
}

void QEcalPlay::setLimitPlaySpeedEnabled(bool enabled)
{
  ecal_play_.SetLimitPlaySpeedEnabled(enabled);

  emit limitPlaySpeedEnabledChangedSignal(enabled);
}

void QEcalPlay::setFrameDroppingAllowed(bool enabled)
{
  ecal_play_.SetFrameDroppingAllowed(enabled);

  emit frameDroppingAllowedChangedSignal(enabled);
}

void QEcalPlay::setEnforceDelayAccuracyEnabled(bool enabled)
{
  ecal_play_.SetEnforceDelayAccuracyEnabled(enabled);

  emit enforceDelayAccuracyEnabledChangedSignal(enabled);
}

bool QEcalPlay::setLimitInterval(const std::pair<long long, long long>& limit_interval)
{
  bool success = ecal_play_.SetLimitInterval(limit_interval);

  if (success)
  {
    auto indexes = ecal_play_.GetLimitInterval();
    auto time_points = std::make_pair(ecal_play_.GetTimestampOf(indexes.first), ecal_play_.GetTimestampOf(indexes.second));
    emit limitIntervalChangedSignal(indexes, time_points);
  }
  return success;
}

bool QEcalPlay::setLimitInterval(const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& limit_interval)
{
  bool success = ecal_play_.SetLimitInterval(limit_interval);

  if (success)
  {
    auto indexes = ecal_play_.GetLimitInterval();
    emit limitIntervalChangedSignal(indexes, limit_interval);
  }
  return success;
}

void QEcalPlay::setChannelMappingFileAction(ChannelMappingFileAction action)
{
  channel_mapping_file_action_ = action;
  QSettings settings;
  settings.setValue("channel_mapping_file_action", action);

  emit channelMappingFileActionChangedSignal(action);
}

void QEcalPlay::setChannelMapping(const std::map<std::string, std::string>& channel_mapping)
{
  channel_mapping_ = channel_mapping;

  emit channelMappingChangedSignal(channel_mapping_);

  if (channel_mapping_.find(step_reference_channel_.toStdString()) == channel_mapping_.end())
  {
    setStepReferenceChannel("");
  }
}

void QEcalPlay::setStepReferenceChannel(const QString& step_reference_channel)
{
  step_reference_channel_ = step_reference_channel;

  emit stepReferenceChannelChangedSignal(step_reference_channel_);
}

////////////////////////////////////////////////////////////////////////////////
//// State update                                                           ////
////////////////////////////////////////////////////////////////////////////////

void QEcalPlay::periodicStateUpdate()
{
  EcalPlayState current_state = currentPlayState();

  last_state_ = current_state;
  emit playStateChangedSignal(current_state);
}

////////////////////////////////////////////////////////////////////////////////
//// Helper                                                                 ////
////////////////////////////////////////////////////////////////////////////////

QWidget* QEcalPlay::widgetOf(QObject* q_object)
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

bool QEcalPlay::isChannelMappingRelevant(const std::map<std::string, std::string>& channel_mapping) const
{
  // An empty channel mapping is not interesting
  if (channel_mapping.empty())
  {
    return false;
  }
  else
  {
    auto all_channels = channelNames();

    // If one channel is NOT contained or it is renamed, the mapping is interesting
    for (auto& channel : all_channels)
    {
      if ((channel_mapping.find(channel) == channel_mapping.end())
        || (channel_mapping.at(channel) != channel))
      {
        return true;
      }
    }

    // Otherwise we have a full identity mapping, which is unteresting
    return false;
  }
}
