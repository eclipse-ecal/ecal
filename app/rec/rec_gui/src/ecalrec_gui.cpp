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

#include "ecalrec_gui.h"

#include "qecalrec.h"

#include <QMessageBox>

#include <iostream>

#ifdef WIN32
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // WIN32

#include <widgets/about_dialog/about_dialog.h>
#include <widgets/license_dialog/license_dialog.h>
#include <widgets/wait_for_local_recorder_dialog/wait_for_local_recorder_dialog.h>

EcalRecGui::EcalRecGui(QWidget *parent)
  : QMainWindow                             (parent)
  , activate_action_state_is_activate_      (true)
  , enable_client_connections_action_state_is_enable_  (true)
  , connect_to_ecal_action_state_is_connect_(true)
  , record_action_state_is_record_          (true)
{
  ui_.setupUi(this);

  control_widget_            = new ControlWidget         (this);
  recorder_manager_widget_   = new RecorderManagerWidget (this);
  topic_widget_              = new TopicWidget           (this);
  config_widget_             = new ConfigWidget          (this);
  recording_history_widget_  = new RecordingHistoryWidget(this);

  remote_measurement_debug_widget_ = new RemoteMeasurementDebugWidget(this);

  setCentralWidget(control_widget_);

  ui_.recorder_manager_dockwidget_content_frame_layout      ->addWidget(recorder_manager_widget_);
  ui_.topics_dockwidget_content_frame_layout                ->addWidget(topic_widget_);
  ui_.config_dockwidget_content_frame_layout                ->addWidget(config_widget_);
  ui_.recording_history_dockwidget_content_frame_layout     ->addWidget(recording_history_widget_);

  ui_.remote_measurement_debug_dockwidget_content_frame_layout->addWidget(remote_measurement_debug_widget_);

  splitDockWidget(ui_.topics_dockwidget, ui_.config_dockwidget, Qt::Orientation::Horizontal);
  splitDockWidget(ui_.config_dockwidget, ui_.recorder_manager_dockwidget, Qt::Orientation::Vertical);

  tabifyDockWidget(ui_.topics_dockwidget, ui_.remote_measurement_debug_dockwidget);
  ui_.config_dockwidget->raise();

  ui_.topics_dockwidget->raise();

  ui_.recording_history_dockwidget->setHidden(true);
  ui_.remote_measurement_debug_dockwidget->setHidden(true);
  
  /////////////////////////////////////////////
  // New File
  /////////////////////////////////////////////
  ui_.action_new->setEnabled(false); // TODO: implement

  /////////////////////////////////////////////
  // Open File
  /////////////////////////////////////////////
  ui_.action_open_config->setEnabled(false); // TODO: implement

  /////////////////////////////////////////////
  // Recent files
  /////////////////////////////////////////////
  ui_.menu_recent_files->setEnabled(false); // TODO: implement

  /////////////////////////////////////////////
  // Save config
  /////////////////////////////////////////////
  ui_.action_save_config->setEnabled(false); // TODO: implement

  /////////////////////////////////////////////
  // Save config as
  /////////////////////////////////////////////
  ui_.action_save_config_as->setEnabled(false); // TODO: implement

  /////////////////////////////////////////////
  // Exit
  /////////////////////////////////////////////
  connect(ui_.action_exit, &QAction::triggered, this, &QMainWindow::close);

  /////////////////////////////////////////////
  // Activate recorders & Menu
  /////////////////////////////////////////////
  connect(ui_.action_activate,                  &QAction::triggered, this, &EcalRecGui::activateActionTriggered);
  connect(ui_.action_enable_client_connections, &QAction::triggered, this, &EcalRecGui::enableClientConnectionsActionTriggered);
  connect(ui_.action_connect_to_ecal,           &QAction::triggered, this, &EcalRecGui::connectToEcalActionTriggered);

  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,       this, &EcalRecGui::updateActivateActionAndAdvancedMenu);
  connect(QEcalRec::instance(), &QEcalRec::recorderInstancesChangedSignal,    this, &EcalRecGui::updateActivateActionAndAdvancedMenu);
  connect(QEcalRec::instance(), &QEcalRec::connectedToEcalStateChangedSignal, this, &EcalRecGui::updateActivateActionAndAdvancedMenu);
  connect(QEcalRec::instance(), &QEcalRec::clientConnectionsEnabledSignal,    this, &EcalRecGui::updateActivateActionAndAdvancedMenu);

  updateActivateActionAndAdvancedMenu();

  /////////////////////////////////////////////
  // Record
  /////////////////////////////////////////////
  connect(ui_.action_start_recording, &QAction::triggered, this, &EcalRecGui::recordActionTriggered);

  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,    this, &EcalRecGui::updateRecordAction);
  connect(QEcalRec::instance(), &QEcalRec::recorderInstancesChangedSignal, this, &EcalRecGui::updateRecordAction);

  updateRecordAction();

  /////////////////////////////////////////////
  // Save buffer
  /////////////////////////////////////////////
  connect(ui_.action_save_pre_buffer, &QAction::triggered, QEcalRec::instance(), &QEcalRec::sendRequestSavePreBufferedData);

  connect(QEcalRec::instance(), &QEcalRec::preBufferingEnabledChangedSignal,  this, &EcalRecGui::updateSaveBufferAction);
  connect(QEcalRec::instance(), &QEcalRec::clientConnectionsEnabledSignal,    this, &EcalRecGui::updateSaveBufferAction);
  connect(QEcalRec::instance(), &QEcalRec::connectedToEcalStateChangedSignal, this, &EcalRecGui::updateSaveBufferAction);
  connect(QEcalRec::instance(), &QEcalRec::recorderInstancesChangedSignal,    this, &EcalRecGui::updateSaveBufferAction);

  updateSaveBufferAction();

  /////////////////////////////////////////////
  // Pre-buffer
  /////////////////////////////////////////////
  connect(ui_.action_pre_buffer_data, &QAction::toggled, QEcalRec::instance(), [](bool checked) { QEcalRec::instance()->setPreBufferingEnabled(checked); });
  
  connect(QEcalRec::instance(), &QEcalRec::preBufferingEnabledChangedSignal, this, &EcalRecGui::updateBufferingEnabledAction);

  updateBufferingEnabledAction(QEcalRec::instance()->preBufferingEnabled());

  /////////////////////////////////////////////
  // Use built-in recorder
  /////////////////////////////////////////////
  connect(ui_.action_use_built_in_local_recorder, &QAction::toggled, QEcalRec::instance(), [](bool checked) { QEcalRec::instance()->setUsingBuiltInRecorderEnabled(checked); });
  connect(QEcalRec::instance(), &QEcalRec::usingBuiltInRecorderEnabledChangedSignal, this, &EcalRecGui::updateUsingBuiltInRecorderAction);

  updateUsingBuiltInRecorderAction(QEcalRec::instance()->usingBuiltInRecorderEnabled());


  /////////////////////////////////////////////
  // Debug console
  /////////////////////////////////////////////
#ifdef WIN32
  ui_.action_debug_console->setChecked(GetConsoleWindow());
  connect(ui_.action_debug_console, &QAction::triggered, [this](bool checked) {showConsole(checked); });
#else // WIN32
  ui_.action_debug_console->setVisible(false);
#endif // WIN32

  /////////////////////////////////////////////
  // Reset layout
  /////////////////////////////////////////////
  ui_.action_reset_layout->setEnabled(false); //TODO: implement

  /////////////////////////////////////////////
  // Windows menu
  /////////////////////////////////////////////
  QList<QAction*> dock_actions = createPopupMenu()->actions();
  for (QAction* action : dock_actions)
  {
    ui_.menu_windows->addAction(action);
  }

  /////////////////////////////////////////////
  // About
  /////////////////////////////////////////////
  connect(ui_.action_about, &QAction::triggered, this,
    [this]()
  {
    AboutDialog about_dialog(this);
    about_dialog.exec();
  });

  /////////////////////////////////////////////
  // Liceses
  /////////////////////////////////////////////
  connect(ui_.action_licenses, &QAction::triggered, this,
    [this]()
  {
    LicenseDialog license_dialog (this);
    license_dialog.exec();
  });

  connect(QEcalRec::instance(), &QEcalRec::exitSignal, this, &QMainWindow::close);
}

EcalRecGui::~EcalRecGui()
{}

void EcalRecGui::closeEvent(QCloseEvent* event)
{
  if (QEcalRec::instance()->recordersRecording())
  {
    QMessageBox warning(
      QMessageBox::Icon::Warning
      , tr("Exit eCAL Recorder")
      , tr("A recording is running. Stop recording?")
      , QMessageBox::Button::Yes | QMessageBox::Button::Cancel
      , this);

    int user_choice = warning.exec();

    if (user_choice != QMessageBox::StandardButton::Yes)
    {
      event->ignore();
      return;
    }
  }

  QEcalRec::instance()->initiateConnectionShutdown();
  QEcalRec::instance()->waitForPendingRequests();

  if (QEcalRec::instance()->localRecorderInstanceBusy())
  {
    WaitForLocalRecorderDialog wait_for_local_recorder_dialog(this);
    wait_for_local_recorder_dialog.setWindowIcon(QIcon(":/ecalrec/APP_ICON"));
    int result = wait_for_local_recorder_dialog.exec();

    if (result == QDialog::DialogCode::Rejected)
    {
      event->ignore();
      return;
    }
  }

  event->accept();
}

void EcalRecGui::activateActionTriggered()
{
  if (activate_action_state_is_activate_)
    QEcalRec::instance()->sendRequestConnectToEcal();
  else
    QEcalRec::instance()->initiateConnectionShutdown();
}

void EcalRecGui::enableClientConnectionsActionTriggered()
{
  if (enable_client_connections_action_state_is_enable_)
    QEcalRec::instance()->setClientConnectionsEnabled(true);
  else
    QEcalRec::instance()->setClientConnectionsEnabled(false);
}

void EcalRecGui::connectToEcalActionTriggered()
{
  if (connect_to_ecal_action_state_is_connect_)
    QEcalRec::instance()->sendRequestConnectToEcal();
  else
    QEcalRec::instance()->sendRequestDisconnectFromEcal();
}

void EcalRecGui::recordActionTriggered()
{
  if (record_action_state_is_record_)
    QEcalRec::instance()->sendRequestStartRecording();
  else
    QEcalRec::instance()->sendRequestStopRecording();
}

void EcalRecGui::updateActivateActionAndAdvancedMenu()
{
  // Activate button
  if ((!QEcalRec::instance()->clientConnectionsEnabled() || !QEcalRec::instance()->recordersConnectedToEcal())
    && !activate_action_state_is_activate_)
  {
    ui_.action_activate->setIcon(QIcon(":/ecalicons/POWER_ON"));
    ui_.action_activate->setText(tr("&Activate"));
    ui_.action_activate->setToolTip(tr("Start recorder clients and connect to eCAL"));
    activate_action_state_is_activate_ = true;
  }
  else if ((QEcalRec::instance()->clientConnectionsEnabled() && QEcalRec::instance()->recordersConnectedToEcal())
    && activate_action_state_is_activate_)
  {
    ui_.action_activate->setIcon(QIcon(":/ecalicons/POWER_OFF"));
    ui_.action_activate->setText(tr("De-&activate"));
    ui_.action_activate->setToolTip(tr("De-initialize clients and disconnect from them"));
    activate_action_state_is_activate_ = false;
  }

  ui_.action_activate->setEnabled                 (!QEcalRec::instance()->recordersRecording());
  ui_.action_enable_client_connections->setEnabled(!QEcalRec::instance()->recordersRecording());
  ui_.action_connect_to_ecal->setEnabled          (!QEcalRec::instance()->recordersRecording());

  // Menu
  if (QEcalRec::instance()->clientConnectionsEnabled() && enable_client_connections_action_state_is_enable_)
  {
    ui_.action_enable_client_connections->setText(tr("Disconnect from &clients"));
    ui_.action_enable_client_connections->setIcon(QIcon(":/ecalicons/DISCONNECTED"));
    enable_client_connections_action_state_is_enable_ = false;
  }
  else if (!QEcalRec::instance()->clientConnectionsEnabled() && !enable_client_connections_action_state_is_enable_)
  {
    ui_.action_enable_client_connections->setText(tr("Connect to &clients"));
    ui_.action_enable_client_connections->setIcon(QIcon(":/ecalicons/CONNECTED"));
    enable_client_connections_action_state_is_enable_ = true;
  }

  if (QEcalRec::instance()->recordersConnectedToEcal() && connect_to_ecal_action_state_is_connect_)
  {
    ui_.action_connect_to_ecal->setText(tr("Disconnect from &eCAL"));
    ui_.action_connect_to_ecal->setIcon(QIcon(":/ecalicons/CROSS"));
    connect_to_ecal_action_state_is_connect_ = false;
  }
  else if (!QEcalRec::instance()->recordersConnectedToEcal() && !connect_to_ecal_action_state_is_connect_)
  {
    ui_.action_connect_to_ecal->setText(tr("Connect to &eCAL"));
    ui_.action_connect_to_ecal->setIcon(QIcon(":/ecalicons/CHECKMARK"));
    connect_to_ecal_action_state_is_connect_ = true;
  }
}

void EcalRecGui::updateRecordAction()
{
  if (QEcalRec::instance()->recordersRecording() && record_action_state_is_record_)
  {
    ui_.action_start_recording->setIcon(QIcon(":/ecalicons/STOP"));
    ui_.action_start_recording->setText(tr("&Stop"));
    ui_.action_start_recording->setToolTip(tr("Stop recording"));
    ui_.action_start_recording->setEnabled(true);
    record_action_state_is_record_ = false;
  }
  else if (!QEcalRec::instance()->recordersRecording())
  {
    if (!record_action_state_is_record_)
    {
      ui_.action_start_recording->setIcon(QIcon(":/ecalicons/RECORD"));
      ui_.action_start_recording->setText(tr("&Record"));
      ui_.action_start_recording->setToolTip(tr("Start recording"));
      record_action_state_is_record_ = true;
    }

    ui_.action_start_recording->setEnabled(QEcalRec::instance()->recorderInstances().size() > 0);
  }
}

void EcalRecGui::updateSaveBufferAction()
{
  ui_.action_save_pre_buffer->setEnabled(QEcalRec::instance()->preBufferingEnabled()
    && QEcalRec::instance()->clientConnectionsEnabled()
    && QEcalRec::instance()->recordersConnectedToEcal()
    && QEcalRec::instance()->recorderInstances().size() > 0);
}

void EcalRecGui::updateBufferingEnabledAction(bool enabled)
{
  if (enabled != ui_.action_pre_buffer_data->isChecked())
  {
    ui_.action_pre_buffer_data->blockSignals(true);
    ui_.action_pre_buffer_data->setChecked(enabled);
    ui_.action_pre_buffer_data->blockSignals(false);
  }
}

void EcalRecGui::updateUsingBuiltInRecorderAction(bool enabled)
{
  if (enabled != ui_.action_use_built_in_local_recorder->isChecked())
  {
    ui_.action_use_built_in_local_recorder->blockSignals(true);
    ui_.action_use_built_in_local_recorder->setChecked(enabled);
    ui_.action_use_built_in_local_recorder->blockSignals(false);
  }
}

#ifdef WIN32
void EcalRecGui::showConsole(bool show)
{
  if (show)
  {
    if (AllocConsole())
    {
      if (!freopen("CONIN$", "r", stdin))
      {
        std::cerr << "Could not open console stdin stream" << std::endl;
      }
      if (!freopen("CONOUT$", "w", stdout))
      {
        std::cerr << "Could not open console stdout stream" << std::endl;
      }
      if (!freopen("CONOUT$", "w", stderr))
      {
        std::cerr << "Could not open console stderr stream" << std::endl;
      }
    }
  }
  else
  {
    FreeConsole();
  }

  ui_.action_debug_console->blockSignals(true);
  ui_.action_debug_console->setChecked(GetConsoleWindow());
  ui_.action_debug_console->blockSignals(false);
}

#endif // WIN32
