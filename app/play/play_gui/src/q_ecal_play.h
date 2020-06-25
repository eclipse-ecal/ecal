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

#include <set>
#include <mutex>

#include "ecal_play.h"
#include <ecal_play_service.h>
#include <ecal/msg/protobuf/server.h>

class QTimer;

class QEcalPlay : public QObject
{
  Q_OBJECT

public:
  enum ChannelMappingFileAction : int
  {
    Ask    = 0,
    Load   = 1,
    Ignore = 2,
  };

  static QEcalPlay* instance();
  ~QEcalPlay();

  //////////////////////////////////////////////////////////////////////////////
  //// Getter                                                               ////
  //////////////////////////////////////////////////////////////////////////////

  // Measurement
  QString                            measurementPath() const;
  QString                            measurementDirectory() const;
  bool                               isMeasurementLoaded() const;
  QString                            description() const;
  std::chrono::nanoseconds           measurementLength() const;
  std::set<std::string>              channelNames() const;
  std::map<std::string, ContinuityReport> createContinuityReport() const;
  std::map<std::string, long long>   messageCounters() const;
  std::vector<EcalPlayScenario>      scenarios() const;
  bool                               scenariosModified() const;
  std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> measurementBoundaries() const;

  long long                          frameCount() const;
  eCAL::Time::ecal_clock::time_point timestampOf(long long frame_index) const;


  // Settings
  bool    isRepeatEnabled() const;
  double  playSpeed() const;
  bool    isLimitPlaySpeedEnabled() const;
  bool    isFrameDroppingAllowed() const;
  bool    isEnforceDelayAccuracyEnabled() const;
  QString stepReferenceChannel() const;
  std::map<std::string, std::string> channelMapping() const;
  std::pair<long long, long long> limitInterval() const;

  ChannelMappingFileAction channelMappingFileAction() const;

  std::map<std::string, std::string> loadChannelMappingFile(const QString& path) const;
  void loadChannelMappingFileFromFileDialog();

  // State
  EcalPlayState currentPlayState() const;
  bool          isInitialized() const;
  bool          isPlaying() const;
  bool          isPaused() const;

  double                   currentPlaySpeed() const;
  long long                lastPublishedFrameIndex() const;
  eCAL::Time::ecal_clock::time_point lastFrameTimestamp() const;

public slots:
  //////////////////////////////////////////////////////////////////////////////
  //// Commands                                                             ////
  //////////////////////////////////////////////////////////////////////////////
  bool loadMeasurementFromFileDialog();
  bool loadMeasurement(const QString& path, bool suppress_blocking_dialogs = false);
  void closeMeasurement(bool omit_blocking_dialogs = false);

  bool saveChannelMappingAs();
  bool saveChannelMapping(const QString& path = "", bool omit_blocking_dialogs = false);

  bool initializePublishers(bool suppress_error_dialog = false);
  bool deInitializePublishers();

  bool play(long long play_until_index = -1, bool suppress_error_dialog = false);
  bool pause();
  void stop();
  bool stepForward(bool suppress_error_dialog = false);
  bool stepChannel(bool suppress_error_dialog = false);

  bool jumpTo(long long index);
  bool jumpTo(eCAL::Time::ecal_clock::time_point timestamp);

  void exit(bool omit_blocking_dialogs = false);

  //////////////////////////////////////////////////////////////////////////////
  //// Setters                                                              ////
  //////////////////////////////////////////////////////////////////////////////
  void setScenarios(const std::vector<EcalPlayScenario>& scenarios);
  bool saveScenariosToDisk(bool suppress_blocking_dialogs = false);
  void setRepeatEnabled(bool enabled);
  void setPlaySpeed(double play_speed);
  void setLimitPlaySpeedEnabled(bool enabled);
  void setFrameDroppingAllowed(bool allowed);
  void setStepReferenceChannel(const QString& step_reference_chanel);
  void setChannelMapping(const std::map<std::string, std::string>& channel_mapping);
  void setEnforceDelayAccuracyEnabled(bool enabled);

  bool setLimitInterval(const std::pair<long long, long long>& limit_interval);
  bool setLimitInterval(const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& limit_interval);

  void setChannelMappingFileAction(ChannelMappingFileAction action);

signals:
  void scenariosChangedSignal(const std::vector<EcalPlayScenario>& scenarios);
  void scenariosSavedSignal() const;
  void measurementLoadedSignal(const QString& path);
  void measurementClosedSignal();
  void publishersInitStateChangedSignal(bool publishers_initialized);
  void playStateChangedSignal(const EcalPlayState& current_state);

  void repeatEnabledChangedSignal(bool enabled); 
  void playSpeedChangedSignal(double play_speed);
  void limitPlaySpeedEnabledChangedSignal(bool enabled);
  void frameDroppingAllowedChangedSignal(bool allowed);
  void enforceDelayAccuracyEnabledChangedSignal(bool enabled);

  void stepReferenceChannelChangedSignal(const QString& step_reference_channel);
  void channelMappingChangedSignal(const std::map<std::string, std::string>& channel_mapping);

  void limitIntervalChangedSignal(const std::pair<long long, long long> indexes, const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& time_points);

  void exitSignal();

  void channelMappingFileActionChangedSignal(ChannelMappingFileAction action);

private:
  QEcalPlay();
  QEcalPlay(const QEcalPlay&) = delete;
  QEcalPlay& operator=(const QEcalPlay&) = delete;

  ChannelMappingFileAction channel_mapping_file_action_;

  // Ecal play instance
  EcalPlay ecal_play_;

  // Service provider
  std::shared_ptr<eCAL::pb::play::EcalPlayService> play_service_;
  eCAL::protobuf::CServiceServer<eCAL::pb::play::EcalPlayService> play_service_server_;

  // State Update
  QTimer* periodic_update_timer_;
  EcalPlayState last_state_; // todo: remove

  // Internal
  std::map<std::string, std::string> channel_mapping_;
  QString step_reference_channel_;

  bool scenarios_modified_;

  static QWidget* widgetOf(QObject* q_object);
  bool isChannelMappingRelevant(const std::map<std::string, std::string>& channel_mapping) const;

private slots:
  void periodicStateUpdate();
};