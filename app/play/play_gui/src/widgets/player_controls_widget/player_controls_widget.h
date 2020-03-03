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

#include <QWidget>
#include "ui_player_controls_widget.h"

#include <chrono>
#include <utility>
#include "ecal_play_state.h"

#include <CustomQt/QStableSortFilterProxyModel.h>
#include "widgets/models/channel_tree_model.h"
#include "widgets/models/channel_tree_item.h"
#include "widgets/settings_widget/settings_widget.h"


#include <ecal/ecal.h>

class PlayerControlsWidget : public QWidget
{
  Q_OBJECT

public:
  PlayerControlsWidget(QWidget *parent = Q_NULLPTR);
  ~PlayerControlsWidget();

public slots:
  void setSettingsVisible(bool visible);
  void resetLayout();

private slots:
  void measurementLoaded(const QString& path);
  void measurementClosed();
  void playStateChanged(const EcalPlayState& current_state);

  void repeatEnabledChanged(bool enabled);
  void stepReferenceChannelChanged(const QString& step_reference_channel);
  void channelMappingChanged(const std::map<std::string, std::string>& channel_mapping);

private:
  Ui::PlayerControlsWidget ui_;

  SettingsWidget* settings_widget_;

  bool play_button_state_is_play;
  bool settings_visible_;

  ChannelTreeModel* step_reference_channel_model_;
  QStableSortFilterProxyModel* step_reference_channel_proxy_model_;

  // Measurement information
  std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> measurement_boundaries_;
  long long measurement_frame_count_;

  void setPlayPauseButtonToPlay();
  void setPlayPauseButtonToPause();

  void setSelectedStepReferenceChannel(const QString& step_reference_channel);

  void saveLayout();
  void restoreLayout();
};
