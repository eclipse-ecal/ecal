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

#include <QtWidgets/QMainWindow>

#ifdef WIN32
#include <QWinTaskbarButton>
#include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>
#endif // WIN32

#include <QActionGroup>

#include "ui_main_window.h"

#include <chrono>
#include <memory>

#include "widgets/player_controls_widget/player_controls_widget.h"
#include "widgets/channel_widget/channel_widget.h"
#include "widgets/scenario_widget/scenario_widget.h"
#include "widgets/description_widget/description_widget.h"

class EcalplayGui : public QMainWindow
{
  Q_OBJECT

public:
  EcalplayGui(QWidget *parent = Q_NULLPTR);

  ~EcalplayGui();

protected:
  void closeEvent(QCloseEvent* event) override;

  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

private slots:
  void measurementLoaded(const QString& path);
  void measurementClosed();
  void publishersInitStateChanged(bool publishers_initialized);
  void playStateChanged(const EcalPlayState& current_state);

  void repeatEnabledChanged(bool enabled);
  void playSpeedChanged(double play_speed);
  void frameDroppingAllowedChanged(bool allowed);
  void enforceDelayAccuracyEnabledChanged(bool enabled);
  void limitPlaySpeedEnabledChanged(bool enabled);

  void stepReferenceChannelChanged(const QString& step_reference_channel);
  void channelMappingChanged(const std::map<std::string, std::string>& channel_mapping);

  void channelMappingFileActionChanged(QEcalPlay::ChannelMappingFileAction action);

  void populateRecentMeasurementsMenu();
  void addRecentMeasurement(const QString& measurement);

  void updateScenariosModified();

  void resetLayout();

  bool loadMeasurementFromFileDialog();
  bool loadMeasurement(const QString& path);
  bool closeMeasurement();

private:
  Ui::EcalplayMainWindow ui_;

  QActionGroup* default_channel_mapping_action_group_;

  bool play_pause_button_state_is_play_;
  bool connect_to_ecal_button_state_is_connect_;

  bool measurement_loaded_;
  std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> measurement_boundaries_;

  QLabel* measurement_path_label_;
  QLabel* current_speed_label_;
  QLabel* current_frame_label_;
  long long measurement_frame_count_;

  PlayerControlsWidget* player_control_widget_;
  ChannelWidget* channel_widget_;
  ScenarioWidget* scenario_widget_;
  DescriptionWidget* description_widget_;

  QByteArray initial_geometry_;
  QByteArray initial_state_;

  void setPlayPauseActionToPlay();
  void setPlayPauseActionToPause();

  void setConnectActionToConnect();
  void setConnectActionToDisconnect();

  void saveLayout();
  void restoreLayout();
  void saveInitialLayout();

  bool askToSaveScenarios();


#ifdef WIN32
////////////////////////////////////////////////////////////////////////////////
//// Windows specific                                                        ////
////////////////////////////////////////////////////////////////////////////////
protected:
  void showEvent(QShowEvent* event) override;

private slots:
  void updateTaskbarProgress(const EcalPlayState& current_state);
  void showConsole(bool show);

private:
  QWinTaskbarButton* taskbar_button_;
  QWinThumbnailToolBar* thumbnail_toolbar_;
  QWinThumbnailToolButton* thumbnail_play_pause_button_;
  QWinThumbnailToolButton* thumbnail_stop_button_;
  QWinThumbnailToolButton* thumbnail_step_button_;
  QWinThumbnailToolButton* thumbnail_step_channel_button_;

  QIcon taskbar_play_icon_;
  QIcon taskbar_play_icon_disabled_;
  QIcon taskbar_pause_icon_;
  QIcon taskbar_pause_icon_disabled_;
  QIcon taskbar_stop_icon_;
  QIcon taskbar_stop_icon_disabled_;
  QIcon taskbar_step_icon_;
  QIcon taskbar_step_icon_disabled_;
  QIcon taskbar_step_channel_icon_;
  QIcon taskbar_step_channel_icon_disabled_;

  void updateTaskbarProgressRange();
#endif // WIN32
};
