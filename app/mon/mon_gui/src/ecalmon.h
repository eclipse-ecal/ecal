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
#include <QTimer>
#include <QActionGroup>

#include "ui_main_window.h"

#include "widgets/log_widget/log_widget.h"
#include "widgets/ecalmon_tree_widget/topic_widget.h"
#include "widgets/ecalmon_tree_widget/process_widget.h"
#include "widgets/ecalmon_tree_widget/host_widget.h"
#include "widgets/ecalmon_tree_widget/service_widget.h"
#include "widgets/system_information_widget/system_information_widget.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class Ecalmon : public QMainWindow
{
  Q_OBJECT
private:
  enum class Theme:int
  {
    Default,
    Dark,
  };

public:
  Ecalmon(QWidget *parent = Q_NULLPTR);

  ~Ecalmon();

protected:
  void showEvent(QShowEvent *event) override;

public:
  bool isMonitorUpdatePaused() const;
  bool isParseTimeEnabled() const;

public slots:
  void updateMonitor();

  void setMonitorUpdatePaused(bool paused);
  void setLogUpdatePaused(bool paused);
  void setParseTimeEnabled(bool enabled);
  void setTheme(Theme theme);

  void resetLayout();
  
private slots:
  void updateEcalTime();

signals:
  void monitorUpdatedSignal(const eCAL::pb::Monitoring&);

protected:
  void closeEvent(QCloseEvent *event);

private:
  Ui::EcalmonMainWindow ui_;

  TopicWidget*             topic_widget_;
  ProcessWidget*           process_widget_;
  HostWidget*              host_widget_;
  ServiceWidget*           service_widget_;
  LogWidget*               log_widget_;
  SystemInformationWidget* syste_information_widget_;

  QLabel* error_label_;
  QLabel* monitor_update_speed_label_;
  QLabel* log_update_speed_label_;
  QLabel* time_label_;

  QTimer* monitor_update_timer_;
  QTimer* ecal_time_update_timer_;

  QActionGroup*  monitor_update_speed_group_;
  QActionGroup*  log_update_speed_group_;
  QActionGroup*  theme_action_group_;

  bool       first_show_event_;
  QByteArray initial_geometry_;
  QByteArray initial_state_;
  bool       initial_alternating_row_colors_;
  bool       initial_parse_time_;
  QPalette   initial_palette_;
  QStyle*    initial_style_;
  QString    initial_style_sheet_;

  int monitor_error_counter_;

  void createDockWidgetMenu();

  void updateMonitorUpdateTimerAndStatusbar();
  void updateLogUpdateTimerAndStatusbar();

  void loadGuiSettings();

  void saveInitialState();
};
