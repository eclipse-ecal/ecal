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

#include "ui_main_window.h"

#include "widgets/control_widget/control_widget.h"
#include "widgets/recordermanager_widget/recordermanager_widget.h"
#include "widgets/topic_widget/topic_widget.h"
#include "widgets/config_widget/config_widget.h"
#include "widgets/recording_history_widget/recording_history_widget.h"

#include "widgets/remote_measurement_debug_widget/remote_measurement_debug_widget.h"

class EcalRecGui : public QMainWindow
{
  Q_OBJECT

public:
  EcalRecGui(QWidget *parent = Q_NULLPTR);

  ~EcalRecGui();

protected:
  void closeEvent(QCloseEvent* event) override;

private slots:
  void activateActionTriggered();
  void enableClientConnectionsActionTriggered();
  void connectToEcalActionTriggered();
  void recordActionTriggered();

  void updateActivateActionAndAdvancedMenu();
  void updateRecordAction();
  void updateSaveBufferAction();
  void updateBufferingEnabledAction(bool enabled);
  void updateUsingBuiltInRecorderAction(bool enabled);

private:
  Ui::EcalRecMainWindow ui_;

  ControlWidget*          control_widget_;
  RecorderManagerWidget*  recorder_manager_widget_;
  TopicWidget*            topic_widget_;
  ConfigWidget*           config_widget_;
  RecordingHistoryWidget* recording_history_widget_;

  RemoteMeasurementDebugWidget* remote_measurement_debug_widget_;

  bool activate_action_state_is_activate_;
  bool enable_client_connections_action_state_is_enable_;
  bool connect_to_ecal_action_state_is_connect_;
  bool record_action_state_is_record_;

#ifdef WIN32
private slots:
  void showConsole(bool show);
#endif // WIN32
};
