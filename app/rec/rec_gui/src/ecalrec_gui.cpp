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
#include <rec_client_core/ecal_rec_defs.h>

#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QSettings>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>

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
#include <widgets/wait_for_shutdown_dialog/wait_for_shutdown_dialog.h>
#include <widgets/upload_settings_dialog/upload_settings_dialog.h>

EcalRecGui::EcalRecGui(QWidget *parent)
  : QMainWindow                                        (parent)
  , activate_action_state_is_activate_                 (true)
  , enable_client_connections_action_state_is_enable_  (true)
  , connect_to_ecal_action_state_is_connect_           (true)
  , record_action_state_is_record_                     (true)
  , first_show_event_                                  (true)
#ifdef WIN32
  , taskbar_activate_icon_                             (":/ecalicons/TASKBAR_POWER_ON")
  , taskbar_deactivate_icon_                           (":/ecalicons/TASKBAR_POWER_OFF")
  , taskbar_record_icon_                               (":/ecalicons/TASKBAR_RECORD")
  , taskbar_record_icon_disabled_                      (":/ecalicons/TASKBAR_RECORD_DISABLED")
  , taskbar_save_buffer_icon_                          (":/ecalicons/TASKBAR_SAVE_BUFFER")
  , taskbar_save_buffer_icon_disabled_                 (":/ecalicons/TASKBAR_SAVE_BUFFER_DISABLED")
#endif // WIN32

{
  // TODO: Add an add / Remove recorder action to the menu
  ui_.setupUi(this);

  control_widget_            = new ControlWidget         (this);
  recorder_manager_widget_   = new RecorderManagerWidget (this);
  topic_widget_              = new TopicWidget           (this);
  config_widget_             = new ConfigWidget          (this);
  recording_history_widget_  = new RecordingHistoryWidget(this);

  remote_measurement_debug_widget_ = new RemoteMeasurementDebugWidget(this);

  // We cannot disable the cenral widget, but we can hide it
  centralWidget()->hide();

  // We want the control dockwidget to look like a central widget, so we "hide" the title bar.
  ui_.control_dockwidget->setTitleBarWidget(new QWidget(ui_.control_dockwidget));

  ui_.control_dockwidget_content_frame_layout               ->addWidget(control_widget_);
  ui_.recorder_manager_dockwidget_content_frame_layout      ->addWidget(recorder_manager_widget_);
  ui_.topics_dockwidget_content_frame_layout                ->addWidget(topic_widget_);
  ui_.config_dockwidget_content_frame_layout                ->addWidget(config_widget_);
  ui_.recording_history_dockwidget_content_frame_layout     ->addWidget(recording_history_widget_);

  ui_.remote_measurement_debug_dockwidget_content_frame_layout->addWidget(remote_measurement_debug_widget_);

  splitDockWidget(ui_.topics_dockwidget, ui_.config_dockwidget, Qt::Orientation::Horizontal);
  splitDockWidget(ui_.topics_dockwidget, ui_.control_dockwidget, Qt::Orientation::Vertical);
  splitDockWidget(ui_.config_dockwidget, ui_.recorder_manager_dockwidget, Qt::Orientation::Vertical);
  splitDockWidget(ui_.recorder_manager_dockwidget, ui_.recording_history_dockwidget, Qt::Orientation::Vertical);


  tabifyDockWidget(ui_.topics_dockwidget, ui_.remote_measurement_debug_dockwidget);
  ui_.config_dockwidget->raise();

  ui_.topics_dockwidget->raise();

  ui_.remote_measurement_debug_dockwidget->setHidden(true);

  // Config
  connect(QEcalRec::instance(), &QEcalRec::configHasBeenModifiedChangedSignal, this, &EcalRecGui::configHasBeenModifiedChanged);
  connect(QEcalRec::instance(), &QEcalRec::loadedConfigChangedSignal, this, &EcalRecGui::loadedConfigPathChanged);

  /////////////////////////////////////////////
  // New File
  /////////////////////////////////////////////
  connect(ui_.action_new, &QAction::triggered, this, &EcalRecGui::clearConfig);

  /////////////////////////////////////////////
  // Open File
  /////////////////////////////////////////////
  connect(ui_.action_open_config, &QAction::triggered, this, [this]() { openConfigWithDialogs(); });

  /////////////////////////////////////////////
  // Recent files
  /////////////////////////////////////////////
  connect(QEcalRec::instance(), &QEcalRec::loadedConfigChangedSignal, this, [this] (const std::string& path, int /*version*/) { addToRecentFileList(path); });

  /////////////////////////////////////////////
  // Save config
  /////////////////////////////////////////////
  connect(ui_.action_save_config, &QAction::triggered, this, static_cast<bool (EcalRecGui::*)(void)>(&EcalRecGui::saveConfig));

  /////////////////////////////////////////////
  // Save config as
  /////////////////////////////////////////////
  connect(ui_.action_save_config_as, &QAction::triggered, this, &EcalRecGui::saveConfigAs);

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
  connect(QEcalRec::instance(), &QEcalRec::enabledRecClientsChangedSignal,    this, &EcalRecGui::updateActivateActionAndAdvancedMenu);
  connect(QEcalRec::instance(), &QEcalRec::connectedToEcalStateChangedSignal, this, &EcalRecGui::updateActivateActionAndAdvancedMenu);
  connect(QEcalRec::instance(), &QEcalRec::connectionToClientsActiveChangedSignal,    this, &EcalRecGui::updateActivateActionAndAdvancedMenu);

  /////////////////////////////////////////////
  // Record
  /////////////////////////////////////////////
  connect(ui_.action_start_recording, &QAction::triggered, this, &EcalRecGui::recordActionTriggered);

  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,    this, &EcalRecGui::updateRecordAction);
  connect(QEcalRec::instance(), &QEcalRec::enabledRecClientsChangedSignal, this, &EcalRecGui::updateRecordAction);

  /////////////////////////////////////////////
  // Save buffer
  /////////////////////////////////////////////
  connect(ui_.action_save_pre_buffer, &QAction::triggered, QEcalRec::instance(), []() { QEcalRec::instance()->savePreBufferedData(); });

  connect(QEcalRec::instance(), &QEcalRec::preBufferingEnabledChangedSignal,       this, &EcalRecGui::updateSaveBufferAction);
  connect(QEcalRec::instance(), &QEcalRec::connectionToClientsActiveChangedSignal, this, &EcalRecGui::updateSaveBufferAction);
  connect(QEcalRec::instance(), &QEcalRec::connectedToEcalStateChangedSignal,      this, &EcalRecGui::updateSaveBufferAction);
  connect(QEcalRec::instance(), &QEcalRec::enabledRecClientsChangedSignal,         this, &EcalRecGui::updateSaveBufferAction);

  /////////////////////////////////////////////
  // Pre-buffer
  /////////////////////////////////////////////
  connect(ui_.action_pre_buffer_data, &QAction::toggled, QEcalRec::instance(), [](bool checked) { QEcalRec::instance()->setPreBufferingEnabled(checked); });
  
  connect(QEcalRec::instance(), &QEcalRec::preBufferingEnabledChangedSignal, this, &EcalRecGui::updateBufferingEnabledAction);

  /////////////////////////////////////////////
  // Add comment to last meas
  /////////////////////////////////////////////
  connect(ui_.action_add_comment_to_last_meas, &QAction::triggered, this, &EcalRecGui::addCommentToLastMeas);

  /////////////////////////////////////////////
  // Upload / Merge all
  /////////////////////////////////////////////
  connect(ui_.action_upload_all, &QAction::triggered, QEcalRec::instance(), []() { QEcalRec::instance()->uploadNonUploadedMeasurements(); });

  /////////////////////////////////////////////
  // Use built-in recorder
  /////////////////////////////////////////////
  connect(ui_.action_use_built_in_local_recorder, &QAction::toggled, QEcalRec::instance(), [](bool checked) { QEcalRec::instance()->setUsingBuiltInRecorderEnabled(checked); });
  connect(QEcalRec::instance(), &QEcalRec::usingBuiltInRecorderEnabledChangedSignal, this, &EcalRecGui::updateUsingBuiltInRecorderAction);

  /////////////////////////////////////////////
  // Upload Settings
  /////////////////////////////////////////////
  connect(ui_.action_upload_settings, &QAction::triggered, this, &EcalRecGui::showUploadSettingsDialog);

  /////////////////////////////////////////////
  // Debug console
  /////////////////////////////////////////////
#ifdef WIN32
  ui_.action_debug_console->setChecked(GetConsoleWindow());
  connect(ui_.action_debug_console, &QAction::triggered, [this](bool checked) {showConsole(checked); });
#else // WIN32
  ui_.action_debug_console->setVisible(false);
#endif // WIN32

#ifdef WIN32
  /////////////////////////////////////////////
  // Windows taskbar
  /////////////////////////////////////////////
  taskbar_button_ = new QWinTaskbarButton(this);

  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,       this, [this]() { updateTaskbarButton(QEcalRec::instance()->recorderStatuses()); });
  connect(QEcalRec::instance(), &QEcalRec::connectedToEcalStateChangedSignal, this, [this]() { updateTaskbarButton(QEcalRec::instance()->recorderStatuses()); });
  connect(QEcalRec::instance(), &QEcalRec::recorderStatusUpdateSignal,        this,
          [this](const eCAL::rec_server::RecorderStatusMap_T& rec_statuses, const std::list<eCAL::rec_server::JobHistoryEntry>& /*job_history*/)
          {
            updateTaskbarButton(rec_statuses);
          });

  thumbnail_toolbar_          = new QWinThumbnailToolBar(this);
  taskbar_activate_button_    = new QWinThumbnailToolButton(thumbnail_toolbar_);
  taskbar_record_button_      = new QWinThumbnailToolButton(thumbnail_toolbar_);
  taskbar_save_buffer_button_ = new QWinThumbnailToolButton(thumbnail_toolbar_);

  taskbar_activate_button_    ->setToolTip(tr("Activate / Prepare"));
  taskbar_activate_button_    ->setIcon(taskbar_activate_icon_);

  taskbar_record_button_      ->setToolTip("Record");
  taskbar_record_button_      ->setIcon(taskbar_record_icon_disabled_);
  taskbar_record_button_      ->setEnabled(false);

  taskbar_save_buffer_button_ ->setToolTip("Save pre-buffer");
  taskbar_save_buffer_button_ ->setIcon(taskbar_save_buffer_icon_disabled_);
  taskbar_save_buffer_button_ ->setEnabled(false);

  connect(taskbar_activate_button_,    &QWinThumbnailToolButton::clicked, this,                 &EcalRecGui::activateActionTriggered);
  connect(taskbar_record_button_,      &QWinThumbnailToolButton::clicked, this,                 &EcalRecGui::recordActionTriggered);
  connect(taskbar_save_buffer_button_, &QWinThumbnailToolButton::clicked, QEcalRec::instance(), []() { QEcalRec::instance()->savePreBufferedData(); });

  thumbnail_toolbar_->addButton(taskbar_activate_button_);
  thumbnail_toolbar_->addButton(taskbar_record_button_);
  thumbnail_toolbar_->addButton(taskbar_save_buffer_button_);
#endif // WIN32

  /////////////////////////////////////////////
  // Reset layout
  /////////////////////////////////////////////
  connect(ui_.action_reset_layout, &QAction::triggered, this, &EcalRecGui::resetLayout);

  /////////////////////////////////////////////
  // Show disabled elements at the end
  /////////////////////////////////////////////
  ui_.action_show_disabled_elements_at_the_bottom->setChecked(QEcalRec::instance()->showDisabledElementsAtEnd());
  connect(ui_.action_show_disabled_elements_at_the_bottom, &QAction::toggled, QEcalRec::instance(), &QEcalRec::setShowDisabledElementsAtEnd);
  connect(QEcalRec::instance(), &QEcalRec::showDisabledElementsAtEndChanged, ui_.action_show_disabled_elements_at_the_bottom, &QAction::setChecked);

  /////////////////////////////////////////////
  // Alternating row colors
  /////////////////////////////////////////////
  ui_.action_alternating_row_colors->setChecked(QEcalRec::instance()->alternatingRowColorsEnabled());
  connect(ui_.action_alternating_row_colors, &QAction::toggled, QEcalRec::instance(), &QEcalRec::setAlternatingRowColorsEnabled);
  connect(QEcalRec::instance(), &QEcalRec::alternatingRowColorsEnabledChanged, ui_.action_alternating_row_colors, &QAction::setChecked);

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

  /////////////////////////////////////////////
  // Initial Layout
  /////////////////////////////////////////////

  updateActivateActionAndAdvancedMenu();
  updateRecordAction();
  updateSaveBufferAction();
  updateBufferingEnabledAction(QEcalRec::instance()->preBufferingEnabled());
  updateUsingBuiltInRecorderAction(QEcalRec::instance()->usingBuiltInRecorderEnabled());

  configHasBeenModifiedChanged(QEcalRec::instance()->configHasBeenModified());
  loadedConfigPathChanged(QEcalRec::instance()->loadedConfigPath(), QEcalRec::instance()->loadedConfigVersion());
  updateRecentFilesMenu();
}

EcalRecGui::~EcalRecGui()
{}

void EcalRecGui::closeEvent(QCloseEvent* event)
{
  if (QEcalRec::instance()->recording())
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
    else
    {
      QEcalRec::instance()->stopRecording(true);
      QEcalRec::instance()->setConnectionToClientsActive(false, true);
      QEcalRec::instance()->waitForPendingRequests();
    }
  }

  if (QEcalRec::instance()->configHasBeenModified())
  {
    bool continue_close = askToSaveFile();
    if (!continue_close)
    {
      event->ignore();
      return;
    }
  }

  if (QEcalRec::instance()->connectionToClientsActive())
  {
    QEcalRec::instance()->setConnectionToClientsActive(false, true);
    QEcalRec::instance()->waitForPendingRequests();
  }

  WaitForShutdownDialog wait_for_shutdown_dialog(this);
  if (!wait_for_shutdown_dialog.safeToExit())
  {
    wait_for_shutdown_dialog.exec();
    if (wait_for_shutdown_dialog.result() != QDialog::DialogCode::Accepted)
    {
      event->ignore();
      return;
    }
  }

  saveLayout();

  event->accept();
}

//////////////////////////////////////////
// Save layout
//////////////////////////////////////////

void EcalRecGui::showEvent(QShowEvent* /*event*/)
{
  if (first_show_event_)
  {
#ifdef WIN32
    registerTaskbarButtons();
#endif // WIN32

    saveInitialLayout();

    restoreLayout();
  }
  first_show_event_ = false;
}

void EcalRecGui::saveLayout()
{
  QSettings settings;
  settings.setValue("recent_files",                         recent_file_list_);
  settings.setValue("window_geometry",                      saveGeometry());
  settings.setValue("window_state",                         saveState(eCAL::rec::Version()));
  settings.setValue("show_disabled_elements_at_the_bottom", ui_.action_show_disabled_elements_at_the_bottom->isChecked());
  settings.setValue("alternating_row_colors",               ui_.action_alternating_row_colors->isChecked());
}

void EcalRecGui::restoreLayout()
{
  QSettings settings;

  QVariant recent_files_variant                         = settings.value("recent_files");
  QVariant geometry_variant                             = settings.value("window_geometry");
  QVariant state_variant                                = settings.value("window_state");
  QVariant show_disabled_elements_at_the_bottom_variant = settings.value("show_disabled_elements_at_the_bottom");
  QVariant alternating_row_colors_variant               = settings.value("alternating_row_colors");

  if (geometry_variant.isValid() && state_variant.isValid())
  {
    restoreGeometry(geometry_variant.toByteArray());
    restoreState(state_variant.toByteArray(), eCAL::rec::Version());
  }

  if (recent_files_variant.isValid())
  {
    recent_file_list_ = recent_files_variant.toStringList();
    updateRecentFilesMenu();
  }

  if (show_disabled_elements_at_the_bottom_variant.isValid())
  {
    ui_.action_show_disabled_elements_at_the_bottom->setChecked(show_disabled_elements_at_the_bottom_variant.toBool());
  }

  if (alternating_row_colors_variant.isValid())
  {
    ui_.action_alternating_row_colors->setChecked(alternating_row_colors_variant.toBool());
  }
}

void EcalRecGui::saveInitialLayout()
{
  initial_geometry_                             = saveGeometry();
  initial_state_                                = saveState();
  initial_show_disabled_elements_at_the_bottom_ = ui_.action_show_disabled_elements_at_the_bottom->isChecked();
  initial_alternating_row_colors_               = ui_.action_alternating_row_colors->isChecked();
}

void EcalRecGui::resetLayout()
{
  topic_widget_           ->resetLayout();
  recorder_manager_widget_->resetLayout();
  recording_history_widget_->resetLayout();

  int screen_number = QApplication::desktop()->screenNumber(this);
  restoreGeometry(initial_geometry_);
  restoreState(initial_state_);
  move(QGuiApplication::screens().at(screen_number)->availableGeometry().center() - rect().center());

  ui_.action_show_disabled_elements_at_the_bottom->setChecked(initial_show_disabled_elements_at_the_bottom_);
  ui_.action_alternating_row_colors              ->setChecked(initial_alternating_row_colors_);
}

//////////////////////////////////////////
// Private slots
//////////////////////////////////////////

void EcalRecGui::activateActionTriggered()
{
  if (activate_action_state_is_activate_)
    QEcalRec::instance()->connectToEcal();
  else
    QEcalRec::instance()->setConnectionToClientsActive(false);
}

void EcalRecGui::enableClientConnectionsActionTriggered()
{
  if (enable_client_connections_action_state_is_enable_)
    QEcalRec::instance()->setConnectionToClientsActive(true);
  else
    QEcalRec::instance()->setConnectionToClientsActive(false);
}

void EcalRecGui::connectToEcalActionTriggered()
{
  if (connect_to_ecal_action_state_is_connect_)
    QEcalRec::instance()->connectToEcal();
  else
    QEcalRec::instance()->disconnectFromEcal();
}

void EcalRecGui::recordActionTriggered()
{
  if (record_action_state_is_record_)
    QEcalRec::instance()->startRecording();
  else
    QEcalRec::instance()->stopRecording();
}

void EcalRecGui::updateActivateActionAndAdvancedMenu()
{
  // Activate button
  if ((!QEcalRec::instance()->connectionToClientsActive() || !QEcalRec::instance()->connectedToEcal())
    && !activate_action_state_is_activate_)
  {
    ui_.action_activate->setIcon(QIcon(":/ecalicons/POWER_ON"));
    ui_.action_activate->setText(tr("&Activate / Prepare"));
    ui_.action_activate->setToolTip(tr("Activate clients and start pre-buffering"));
    activate_action_state_is_activate_ = true;

#ifdef WIN32
    taskbar_activate_button_->setIcon(taskbar_activate_icon_);
    taskbar_activate_button_->setToolTip(tr("Activate / Prepare"));
#endif // WIN32
  }
  else if ((QEcalRec::instance()->connectionToClientsActive() && QEcalRec::instance()->connectedToEcal())
    && activate_action_state_is_activate_)
  {
    ui_.action_activate->setIcon(QIcon(":/ecalicons/POWER_OFF"));
    ui_.action_activate->setText(tr("De-&activate"));
    ui_.action_activate->setToolTip(tr("De-activate clients and stop pre-buffering"));
    activate_action_state_is_activate_ = false;

#ifdef WIN32
    taskbar_activate_button_->setIcon(taskbar_deactivate_icon_);
    taskbar_activate_button_->setToolTip(tr("De-activate"));
#endif // WIN32
  }

  ui_.action_activate->setEnabled                 (!QEcalRec::instance()->recording());
  ui_.action_enable_client_connections->setEnabled(!QEcalRec::instance()->recording());
  ui_.action_connect_to_ecal->setEnabled          (!QEcalRec::instance()->recording());

  // Menu
  if (QEcalRec::instance()->connectionToClientsActive() && enable_client_connections_action_state_is_enable_)
  {
    ui_.action_enable_client_connections->setText(tr("Disconnect from &clients"));
    ui_.action_enable_client_connections->setIcon(QIcon(":/ecalicons/DISCONNECTED"));
    enable_client_connections_action_state_is_enable_ = false;
  }
  else if (!QEcalRec::instance()->connectionToClientsActive() && !enable_client_connections_action_state_is_enable_)
  {
    ui_.action_enable_client_connections->setText(tr("Connect to &clients"));
    ui_.action_enable_client_connections->setIcon(QIcon(":/ecalicons/CONNECTED"));
    enable_client_connections_action_state_is_enable_ = true;
  }

  if (QEcalRec::instance()->connectedToEcal() && connect_to_ecal_action_state_is_connect_)
  {
    ui_.action_connect_to_ecal->setText(tr("Disconnect from &eCAL"));
    ui_.action_connect_to_ecal->setIcon(QIcon(":/ecalicons/CROSS"));
    connect_to_ecal_action_state_is_connect_ = false;
  }
  else if (!QEcalRec::instance()->connectedToEcal() && !connect_to_ecal_action_state_is_connect_)
  {
    ui_.action_connect_to_ecal->setText(tr("Connect to &eCAL"));
    ui_.action_connect_to_ecal->setIcon(QIcon(":/ecalicons/CHECKMARK"));
    connect_to_ecal_action_state_is_connect_ = true;
  }
}

void EcalRecGui::updateRecordAction()
{
  if (QEcalRec::instance()->recording() && record_action_state_is_record_)
  {
    ui_.action_start_recording->setIcon(QIcon(":/ecalicons/STOP"));
    ui_.action_start_recording->setText(tr("&Stop"));
    ui_.action_start_recording->setToolTip(tr("Stop recording"));
    ui_.action_start_recording->setEnabled(true);
    record_action_state_is_record_ = false;

#ifdef WIN32
    taskbar_record_button_->setIcon(QIcon(":/ecalicons/TASKBAR_STOP"));
    taskbar_record_button_->setToolTip(tr("Stop recording"));
    taskbar_record_button_->setEnabled(true);
#endif // WIN32
  }
  else if (!QEcalRec::instance()->recording())
  {
    if (!record_action_state_is_record_)
    {
      ui_.action_start_recording->setIcon(QIcon(":/ecalicons/RECORD"));
      ui_.action_start_recording->setText(tr("&Record"));
      ui_.action_start_recording->setToolTip(tr("Start recording"));
      record_action_state_is_record_ = true;

#ifdef WIN32
      taskbar_record_button_->setIcon(QIcon(":/ecalicons/TASKBAR_RECORD"));
      taskbar_record_button_->setToolTip(tr("Start recording"));
#endif // WIN32
    }

    bool enabled = (QEcalRec::instance()->enabledRecClients().size() > 0);
    ui_.action_start_recording->setEnabled(enabled);
#ifdef WIN32
    taskbar_record_button_->setEnabled(enabled);
    taskbar_record_button_->setIcon(enabled ? taskbar_record_icon_ : taskbar_record_icon_disabled_);
#endif // WIN32
  }
}

void EcalRecGui::updateSaveBufferAction()
{
  bool enabled = (QEcalRec::instance()->preBufferingEnabled()
    && QEcalRec::instance()->connectionToClientsActive()
    && QEcalRec::instance()->connectedToEcal()
    && QEcalRec::instance()->enabledRecClients().size() > 0);

  ui_.action_save_pre_buffer->setEnabled(enabled);
#ifdef WIN32
  taskbar_save_buffer_button_->setEnabled(enabled);
  taskbar_save_buffer_button_->setIcon(enabled ? taskbar_save_buffer_icon_ : taskbar_save_buffer_icon_disabled_);
#endif // WIN32
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

bool EcalRecGui::clearConfig()
{
  if (QEcalRec::instance()->configHasBeenModified())
  {
    const bool continue_open = askToSaveFile();
    if (!continue_open)
      return false;
  }

  if (QEcalRec::instance()->recording())
  {
    QMessageBox warning(
      QMessageBox::Icon::Warning
      , tr("New config")
      , tr("A recording is running. Stop recording?")
      , QMessageBox::Button::Yes | QMessageBox::Button::Cancel
      , this);

    int user_choice = warning.exec();

    if (user_choice == QMessageBox::StandardButton::Yes)
    {
      QEcalRec::instance()->stopRecording();
    }
    else
    {
      return false;
    }
  }

  return QEcalRec::instance()->clearConfig();
}

bool EcalRecGui::saveConfigAs()
{
  std::string current_config_path = QEcalRec::instance()->loadedConfigPath();
  QString start_dir = (current_config_path == "" ? "New Config.ecalrec" : QString::fromStdString(current_config_path));

  QString selected_file = QFileDialog::getSaveFileName(this, "Save as", start_dir, tr("eCAL Rec files (*.ecalrec);;All Files (*)"));

  if (selected_file != "")
    return saveConfig(selected_file);

  return false;
}

bool EcalRecGui::saveConfig()
{
  if (QEcalRec::instance()->loadedConfigPath() == "")
  {
    return saveConfigAs();
  }
  else
  {
    return saveConfig(QString::fromStdString(QEcalRec::instance()->loadedConfigPath()));
  }
}

bool EcalRecGui::saveConfig(const QString& path)
{
  // Clear the focus, so that the currently edited option will also be set and saved
  QWidget* current_focus = QApplication::focusWidget();
  if (current_focus)
    current_focus->clearFocus();

  if (path != "")
  {
    bool success = false;
    bool retry = false;

    do
    {
      retry   = false;
      success = QEcalRec::instance()->saveConfigToFile(path.toStdString());

      if (!success)
      {
        QMessageBox error_message(
          QMessageBox::Icon::Critical
          , "Error"
          , "Error saving file to \"" + path + "\""
          , QMessageBox::Button::Ok | QMessageBox::Button::Retry
          , this);
        int choice = error_message.exec();
        retry = (choice == QMessageBox::Button::Retry);
      }
    } while (retry);

    return success;
  }
  return false;
}

bool EcalRecGui::openConfigWithDialogs(const QString& path)
{
  if (QEcalRec::instance()->configHasBeenModified())
  {
    const bool continue_open = askToSaveFile();
    if (!continue_open)
      return false;
  }

  if (QEcalRec::instance()->recording())
  {
    QMessageBox warning(
      QMessageBox::Icon::Warning
      , tr("Open config")
      , tr("A recording is running. Stop recording?")
      , QMessageBox::Button::Yes | QMessageBox::Button::Cancel
      , this);

    int user_choice = warning.exec();

    if (user_choice == QMessageBox::StandardButton::Yes)
    {
      QEcalRec::instance()->stopRecording();
    }
    else
    {
      return false;
    }
  }

  if (!path.isEmpty())
  {
    return openConfig(path);
  }
  else
  {
    QString start_path = QString::fromStdString(QEcalRec::instance()->loadedConfigPath());
    if (start_path.isEmpty() && !recent_file_list_.empty())
    {
      QFileInfo config_file_info(recent_file_list_.front());
      start_path = config_file_info.absoluteDir().absolutePath();
    }

    QString config_path = QFileDialog::getOpenFileName(this, tr("Open eCAL Rec Config"), start_path, tr("Ecal rec files (*.ecalrec);;All Files (*)"));

    if (!config_path.isEmpty())
    {
      return openConfig(config_path);
    }
    else
    {
      return false;
    }
  }
}

bool EcalRecGui::openConfig(const QString& path)
{
  bool success = false;
  bool retry = false;

  do
  {
    retry = false;

    success = QEcalRec::instance()->loadConfigFromFile(path.toStdString());

    if (!success)
    {
      QMessageBox error_message(
        QMessageBox::Icon::Critical
        , "Error"
        , "Error loading config from \"" + path + "\""
        , QMessageBox::Button::Ok | QMessageBox::Button::Retry
        , this);
      int choice = error_message.exec();
      retry = (choice == QMessageBox::Button::Retry);
    }
  } while (retry);

  return success;
}

bool EcalRecGui::askToSaveFile()
{
  bool may_continue = false;

  if (QEcalRec::instance()->configHasBeenModified())
  {
    QMessageBox config_modified_warning(
      QMessageBox::Icon::Warning
      , tr("The config has been modified")
      , tr("Do you want to save your changes?")
      , QMessageBox::Button::Save | QMessageBox::Button::Discard | QMessageBox::Button::Cancel
      , this);

    int user_choice = config_modified_warning.exec();

    if (user_choice == QMessageBox::Button::Save)
    {
      if (QEcalRec::instance()->loadedConfigPath() == "")
      {
        may_continue = saveConfigAs();
      }
      else
      {
        saveConfig();
        may_continue = true;
      }
    }
    else if (user_choice == QMessageBox::Button::Discard)
    {
      may_continue = true;
    }
    else if (user_choice == QMessageBox::Button::Cancel)
    {
      may_continue = false;
    }
  }
  return may_continue;
}


void EcalRecGui::configHasBeenModifiedChanged(bool modified)
{
  setWindowModified(modified);
}

void EcalRecGui::loadedConfigPathChanged(const std::string& path, int version)
{
  if (path.empty())
  {
    setWindowFilePath("New config");
  }
  else
  {
    QString file_path = QString::fromStdString(path);
    if (version != QEcalRec::instance()->nativeConfigVersion())
      file_path += " [Compatibility mode]";

    setWindowFilePath(file_path);
  }
}

void EcalRecGui::addToRecentFileList(const std::string& path)
{
  QFileInfo config_file_info(QString::fromStdString(path));
  QString normalized_path = config_file_info.absoluteFilePath();

#ifdef WIN32
  normalized_path.replace("/", "\\");

  // Remove duplicates
  auto recent_file_it = recent_file_list_.begin();
  while (recent_file_it != recent_file_list_.end())
  {
    if (recent_file_it->compare(normalized_path, Qt::CaseInsensitive) == 0)
    {
      recent_file_it = recent_file_list_.erase(recent_file_it);
    }
    else
    {
      ++recent_file_it;
    }
  }

  // Add the config
  recent_file_list_.push_front(normalized_path);

#else
  // Remove duplicates
  recent_file_list_.removeAll(normalized_path);

  // Add the config
  recent_file_list_.push_front(normalized_path);
#endif // WIN32

  // trim the list to 10 elements
  while (recent_file_list_.size() > 10)
  {
    recent_file_list_.removeLast();
  }

  // update the menu
  updateRecentFilesMenu();
}

void EcalRecGui::updateRecentFilesMenu()
{
  ui_.menu_recent_files->clear();

  if (recent_file_list_.size() == 0)
  {
    QAction* dummy_action = new QAction(tr("no files"), ui_.menu_recent_files);
    ui_.menu_recent_files->addAction(dummy_action);
    dummy_action->setEnabled(false);
  }
  else
  {
    int counter = 1;
    for (auto i = recent_file_list_.begin(); i != recent_file_list_.end(); ++i)
    {
      QString label;
      if (counter > 0 && counter <= 9)
      {
        label = "&" + QString::number(counter);
      }
      else if (counter == 10)
      {
        label = "1&0";
      }
      else
      {
        label = QString::number(counter);
      }

      QAction* open_file_action = new QAction(label + ": " + *i, ui_.menu_recent_files);
      connect(open_file_action, &QAction::triggered, [this, i]() { openConfigWithDialogs(*i); });
      ui_.menu_recent_files->addAction(open_file_action);

      counter++;
    }
  }

  QAction* clear_recent_files_action = new QAction(tr("&Clear list"), ui_.menu_recent_files);
  connect(clear_recent_files_action, &QAction::triggered, [this]() { clearRecentFiles(); });
  clear_recent_files_action->setEnabled(recent_file_list_.size() > 0);
  ui_.menu_recent_files->addSeparator();
  ui_.menu_recent_files->addAction(clear_recent_files_action);
}

void EcalRecGui::clearRecentFiles()
{
  recent_file_list_.clear();
  updateRecentFilesMenu();
}

void EcalRecGui::addCommentToLastMeas()
{
  auto job_history = QEcalRec::instance()->jobHistory();
  if (job_history.empty())
  {
    QMessageBox error_message(this);
    error_message.setIcon(QMessageBox::Icon::Critical);
    error_message.setWindowTitle("Error");
    error_message.setText(tr("Error adding comment: There are no recent measurements."));
    error_message.exec();
  }
  else
  {
    int64_t last_meas_id = job_history.back().local_evaluated_job_config_.GetJobId();

    auto simulated_error = QEcalRec::instance()->simulateAddComment(last_meas_id);
    if(!simulated_error)
    {
      QEcalRec::instance()->addCommentWithDialog(last_meas_id);
    }
    else
    {
      QMessageBox error_message(this);
      error_message.setIcon(QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText(tr("Error adding comment"));
      error_message.setInformativeText(tr("Unable to add comment to measurement ") + QString::number(last_meas_id) + ":\n" + QString::fromStdString(simulated_error.ToString()));
      error_message.exec();
    }
  }
}

void EcalRecGui::showUploadSettingsDialog()
{
  UploadSettingsDialog upload_settings_dialog(QEcalRec::instance()->uploadConfig(), this);

  int user_choice = upload_settings_dialog.exec();
  if (user_choice == QDialog::Accepted)
  {
    QEcalRec::instance()->setUploadConfig(upload_settings_dialog.uploadConfig());
  }
}

#ifdef WIN32
////////////////////////////////////////////
// Windows specific
////////////////////////////////////////////
void EcalRecGui::registerTaskbarButtons()
{
  taskbar_button_   ->setWindow(this->windowHandle());
  thumbnail_toolbar_->setWindow(this->windowHandle());
}

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

void EcalRecGui::updateTaskbarButton(const eCAL::rec_server::RecorderStatusMap_T& recorder_statuses)
{
  if (QEcalRec::instance()->recording())
  {
    taskbar_button_->setOverlayIcon(QIcon(":/ecalicons/TASKBAR_RECORD"));
  }
  else
  {
    bool anybody_flushing = false;
    for (const auto& recorder_status : recorder_statuses)
    {
      for (const auto& job_status : recorder_status.second.first.job_statuses_)
      {
        if (job_status.state_ == eCAL::rec::JobState::Flushing)
        {
          anybody_flushing = true;
          goto endloop;
        }
      }
    }
    endloop:
    
    if (anybody_flushing)
    {
      taskbar_button_->setOverlayIcon(QIcon(":/ecalicons/TASKBAR_SAVE_BUFFER"));
    }
    else
    {
      if (QEcalRec::instance()->connectedToEcal())
      {
        taskbar_button_->setOverlayIcon(QIcon(":/ecalicons/TASKBAR_POWER_ON"));
      }
      else
      {
        taskbar_button_->clearOverlayIcon();
      }
    }
  }
}

#endif // WIN32
