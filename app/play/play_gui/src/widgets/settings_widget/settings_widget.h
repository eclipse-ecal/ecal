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
#include "ui_settings_widget.h"

#include <ecal/ecal.h>

class SettingsWidget : public QWidget
{
  Q_OBJECT

public:
  SettingsWidget(QWidget *parent = Q_NULLPTR);
  ~SettingsWidget();

public slots:
  void resetLayout();

private slots:
  void measurementLoaded(const QString& path);
  void measurementClosed();
  void playSpeedChanged(double play_speed);
  void limitPlaySpeedEnabledChanged(bool enabled);
  void frameDroppingAllowedChanged(bool allowed);
  void enforceDelayAccuracyEnabledChanged(bool enabled);

  void limitIntervalChanged(const std::pair<long long, long long> indexes, const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& time_points);
  void lowerLimitSpinboxChanged(double value);
  void upperLimitSpinboxChanged(double value);

private:
  Ui::SettingsWidget ui_;

  std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> measurement_boundaries_;


  void saveLayout();
  void restoreLayout();
};
