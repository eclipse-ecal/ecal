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

#include "ecal_play_gui.h"

#include <QFileDialog>
#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QMimeData>
#include <QScreen>
#include <QMessageBox>

#ifdef WIN32
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // WIN32

#include "q_ecal_play.h"

#include "ecal_play_globals.h"

#include "widgets/about_dialog/about_dialog.h"
#include "widgets/license_dialog/license_dialog.h"

EcalplayGui::EcalplayGui(QWidget *parent)
  : QMainWindow(parent)
  , play_pause_button_state_is_play_        (true)
  , connect_to_ecal_button_state_is_connect_(true)
  , measurement_loaded_                     (false)
  , measurement_boundaries_                 (eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(0)), eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(0)))
  , measurement_frame_count_                (0)
#ifdef WIN32
  , taskbar_play_icon_                      (":ecalicons/TASKBAR_PLAY")
  , taskbar_play_icon_disabled_             (":ecalicons/TASKBAR_PLAY_DISABLED")
  , taskbar_pause_icon_                     (":ecalicons/TASKBAR_PAUSE")
  , taskbar_pause_icon_disabled_            (":ecalicons/TASKBAR_PAUSE_DISABLED")
  , taskbar_stop_icon_                      (":ecalicons/TASKBAR_STOP")
  , taskbar_stop_icon_disabled_             (":ecalicons/TASKBAR_STOP_DISABLED")
  , taskbar_step_icon_                      (":ecalicons/TASKBAR_PLAY_NEXT")
  , taskbar_step_icon_disabled_             (":ecalicons/TASKBAR_PLAY_NEXT_DISABLED")
  , taskbar_step_channel_icon_              (":ecalicons/TASKBAR_FORWARD_TO")
  , taskbar_step_channel_icon_disabled_     (":ecalicons/TASKBAR_FORWARD_TO_DISABLED")
#endif // WIN32
{
  ui_.setupUi(this);
  splitDockWidget(ui_.scenario_dockwidget, ui_.description_dockwidget, Qt::Orientation::Vertical);

  player_control_widget_ = new PlayerControlsWidget(this);
  channel_widget_        = new ChannelWidget(this);
  scenario_widget_       = new ScenarioWidget(this);
  description_widget_    = new DescriptionWidget(this);

  setCentralWidget(player_control_widget_);
  ui_.channel_dockwidget_content_frame_layout    ->addWidget(channel_widget_);
  ui_.scenario_dockwidget_content_frame_layout   ->addWidget(scenario_widget_);
  ui_.description_dockwidget_content_frame_layout->addWidget(description_widget_);

  default_channel_mapping_action_group_ = new QActionGroup(this);
  default_channel_mapping_action_group_->addAction(ui_.action_default_mapping_file_ask);
  default_channel_mapping_action_group_->addAction(ui_.action_default_mapping_file_load);
  default_channel_mapping_action_group_->addAction(ui_.action_default_mapping_file_ignore);

  channelMappingFileActionChanged(QEcalPlay::instance()->channelMappingFileAction());
  connect(default_channel_mapping_action_group_, &QActionGroup::triggered, this,
    [this](QAction* action)
  {
    if      (action == ui_.action_default_mapping_file_ask)    QEcalPlay::instance()->setChannelMappingFileAction(QEcalPlay::ChannelMappingFileAction::Ask);
    else if (action == ui_.action_default_mapping_file_load)   QEcalPlay::instance()->setChannelMappingFileAction(QEcalPlay::ChannelMappingFileAction::Load);
    else if (action == ui_.action_default_mapping_file_ignore) QEcalPlay::instance()->setChannelMappingFileAction(QEcalPlay::ChannelMappingFileAction::Ignore);
  });

  //////////////////////////////////////////////////////////////////////////////
  //// Status Bar                                                           ////
  //////////////////////////////////////////////////////////////////////////////

  measurement_path_label_ = new QLabel("No measurement loaded", this);
  current_speed_label_    = new QLabel(" Current Speed: 99999.99 ", this);
  current_frame_label_    = new QLabel(" Frame: 999999 / 999999 ", this);

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
  int current_speed_label_size = current_speed_label_->fontMetrics().width(" Current Speed: 9999999.99 ");
  int current_frame_label_size = current_speed_label_->fontMetrics().width(" Frame: 999999999 / 999999999 ");
#else
  int current_speed_label_size = current_speed_label_->fontMetrics().horizontalAdvance(" Current Speed: 9999999.99 ");
  int current_frame_label_size = current_speed_label_->fontMetrics().horizontalAdvance(" Frame: 999999999 / 999999999 ");
#endif

  int common_size = std::max(current_speed_label_size, current_frame_label_size);

  current_speed_label_->setMaximumWidth(common_size);
  current_frame_label_->setMaximumWidth(common_size);
  current_speed_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  current_frame_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  measurement_path_label_->setMinimumWidth(10);
  current_speed_label_   ->setMinimumWidth(10);
  current_frame_label_   ->setMinimumWidth(10);

  ui_.statusbar->addWidget(measurement_path_label_, 1);
  ui_.statusbar->addPermanentWidget(current_speed_label_, 2);
  ui_.statusbar->addPermanentWidget(current_frame_label_, 2);


  //////////////////////////////////////////////////////////////////////////////
  //// Menu Bar Actions                                                     ////
  //////////////////////////////////////////////////////////////////////////////

  // Connect QEcalPlay -> this
  connect(QEcalPlay::instance(), &QEcalPlay::measurementLoadedSignal,                  this, &EcalplayGui::measurementLoaded);
  connect(QEcalPlay::instance(), &QEcalPlay::measurementClosedSignal,                  this, &EcalplayGui::measurementClosed);
  connect(QEcalPlay::instance(), &QEcalPlay::publishersInitStateChangedSignal,         this, &EcalplayGui::publishersInitStateChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::playStateChangedSignal,                   this, &EcalplayGui::playStateChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::repeatEnabledChangedSignal,               this, &EcalplayGui::repeatEnabledChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::playSpeedChangedSignal,                   this, &EcalplayGui::playSpeedChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::frameDroppingAllowedChangedSignal,        this, &EcalplayGui::frameDroppingAllowedChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::enforceDelayAccuracyEnabledChangedSignal, this, &EcalplayGui::enforceDelayAccuracyEnabledChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::limitPlaySpeedEnabledChangedSignal,       this, &EcalplayGui::limitPlaySpeedEnabledChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::stepReferenceChannelChangedSignal,        this, &EcalplayGui::stepReferenceChannelChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::channelMappingChangedSignal,              this, &EcalplayGui::channelMappingChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::exitSignal,                               this, &QMainWindow::close);

  connect(QEcalPlay::instance(), &QEcalPlay::scenariosChangedSignal,                   this, &EcalplayGui::updateScenariosModified);
  connect(QEcalPlay::instance(), &QEcalPlay::scenariosSavedSignal,                     this, &EcalplayGui::updateScenariosModified);
  connect(QEcalPlay::instance(), &QEcalPlay::measurementClosedSignal,                  this, &EcalplayGui::updateScenariosModified);
  connect(QEcalPlay::instance(), &QEcalPlay::measurementLoadedSignal,                  this, &EcalplayGui::updateScenariosModified);

  connect(QEcalPlay::instance(), &QEcalPlay::channelMappingFileActionChangedSignal,    this, &EcalplayGui::channelMappingFileActionChanged);

  // Connect this -> QEcalPlay
  connect(ui_.action_connect_to_ecal,       &QAction::triggered, 
      [this]()
      {
        if (connect_to_ecal_button_state_is_connect_)
          QEcalPlay::instance()->initializePublishers();
        else
          QEcalPlay::instance()->deInitializePublishers();
      });
  connect(ui_.action_load_measurement,       &QAction::triggered, this,                  &EcalplayGui::loadMeasurementFromFileDialog);
  connect(ui_.action_close_measurement,      &QAction::triggered, this,                  &EcalplayGui::closeMeasurement);
  connect(ui_.action_save_scenarios,         &QAction::triggered, QEcalPlay::instance(), []() { QEcalPlay::instance()->saveScenariosToDisk(); });
  connect(ui_.action_save_channel_mapping,   &QAction::triggered, QEcalPlay::instance(), &QEcalPlay::saveChannelMappingAs);
  connect(ui_.action_load_channel_mapping,   &QAction::triggered, QEcalPlay::instance(), &QEcalPlay::loadChannelMappingFileFromFileDialog);

  connect(ui_.action_exit,                   &QAction::triggered, this,                  &QMainWindow::close);

  connect(ui_.action_play,                   &QAction::triggered, 
      [this]()
      {
        if (play_pause_button_state_is_play_)
          QEcalPlay::instance()->play();
        else
          QEcalPlay::instance()->pause();
      });
  connect(ui_.action_stop,                   &QAction::triggered, QEcalPlay::instance(), &QEcalPlay::stop);
  connect(ui_.action_step_forward,           &QAction::triggered, QEcalPlay::instance(), []() {QEcalPlay::instance()->stepForward(); });
  connect(ui_.action_step_channel,           &QAction::triggered, QEcalPlay::instance(), []() {QEcalPlay::instance()->stepChannel(); });
  connect(ui_.action_repeat,                 &QAction::toggled,   QEcalPlay::instance(), &QEcalPlay::setRepeatEnabled);
  connect(ui_.action_allow_framedrops,       &QAction::toggled,   QEcalPlay::instance(), &QEcalPlay::setFrameDroppingAllowed);
  connect(ui_.action_enforce_delay_accuracy, &QAction::toggled,   QEcalPlay::instance(), &QEcalPlay::setEnforceDelayAccuracyEnabled);
  connect(ui_.action_limit_playback_speed,   &QAction::toggled,   QEcalPlay::instance(), &QEcalPlay::setLimitPlaySpeedEnabled);
  connect(ui_.action_reset_layout,           &QAction::triggered, this,                  &EcalplayGui::resetLayout);
  connect(ui_.action_settings,               &QAction::triggered, player_control_widget_, [this]() {player_control_widget_->setSettingsVisible(true); });


  //////////////////////////////////////////////////////////////////////////////
  //// Windows menu                                                         ////
  //////////////////////////////////////////////////////////////////////////////
  QList<QAction*> dock_actions = createPopupMenu()->actions();
  for (QAction* action : dock_actions)
  {
    ui_.menu_windows->addAction(action);
  }

  //////////////////////////////////////////////////////////////////////////////
  //// Help Menu                                                            ////
  //////////////////////////////////////////////////////////////////////////////
  connect(ui_.action_about,    &QAction::triggered, this, [this]() {(new AboutDialog(this))->exec(); });
  connect(ui_.action_licenses, &QAction::triggered, this, [this]() {(new LicenseDialog(this))->exec(); });

  //////////////////////////////////////////////////////////////////////////////
  //// Recent files                                                         ////
  //////////////////////////////////////////////////////////////////////////////
  connect(ui_.menu_recent_measurement, &QMenu::aboutToShow, this, &EcalplayGui::populateRecentMeasurementsMenu);

#ifdef WIN32
  //////////////////////////////////////////////////////////////////////////////
  //// Windows Taskbar                                                      ////
  //////////////////////////////////////////////////////////////////////////////
  taskbar_button_ = new QWinTaskbarButton(this);
  connect(QEcalPlay::instance(), &QEcalPlay::playStateChangedSignal,           this, &EcalplayGui::updateTaskbarProgress);
  connect(QEcalPlay::instance(), &QEcalPlay::publishersInitStateChangedSignal, this, [this]() { EcalplayGui::updateTaskbarProgress(QEcalPlay::instance()->currentPlayState()); });

  thumbnail_toolbar_             = new QWinThumbnailToolBar(this);
  thumbnail_play_pause_button_   = new QWinThumbnailToolButton(thumbnail_toolbar_);
  thumbnail_stop_button_         = new QWinThumbnailToolButton(thumbnail_toolbar_);
  thumbnail_step_button_         = new QWinThumbnailToolButton(thumbnail_toolbar_);
  thumbnail_step_channel_button_ = new QWinThumbnailToolButton(thumbnail_toolbar_);

  thumbnail_play_pause_button_  ->setToolTip("Play");
  thumbnail_play_pause_button_  ->setIcon(taskbar_play_icon_disabled_);
  thumbnail_play_pause_button_  ->setEnabled(false);


  thumbnail_stop_button_        ->setToolTip("Stop");
  thumbnail_stop_button_        ->setIcon(taskbar_stop_icon_disabled_);
  thumbnail_stop_button_        ->setEnabled(false);

  thumbnail_step_button_        ->setToolTip("Step frame");
  thumbnail_step_button_        ->setIcon(taskbar_play_icon_disabled_);
  thumbnail_step_button_        ->setEnabled(false);

  thumbnail_step_channel_button_->setToolTip("Step Channel");
  thumbnail_step_channel_button_->setIcon(taskbar_step_channel_icon_disabled_);
  thumbnail_step_channel_button_->setEnabled(false);

  connect(thumbnail_play_pause_button_, &QWinThumbnailToolButton::clicked,
      [this]()
      {
        if (play_pause_button_state_is_play_)
          QEcalPlay::instance()->play();
        else
          QEcalPlay::instance()->pause();
      });
  connect(thumbnail_stop_button_,         &QWinThumbnailToolButton::clicked, QEcalPlay::instance(), &QEcalPlay::stop);
  connect(thumbnail_step_button_,         &QWinThumbnailToolButton::clicked, QEcalPlay::instance(), []() {QEcalPlay::instance()->stepForward(); });
  connect(thumbnail_step_channel_button_, &QWinThumbnailToolButton::clicked, QEcalPlay::instance(), []() {QEcalPlay::instance()->stepChannel(); });

  thumbnail_toolbar_->addButton(thumbnail_play_pause_button_);
  thumbnail_toolbar_->addButton(thumbnail_stop_button_);
  thumbnail_toolbar_->addButton(thumbnail_step_button_);
  thumbnail_toolbar_->addButton(thumbnail_step_channel_button_);

  // Special show-console button for Windows
  ui_.action_debug_console->setChecked(GetConsoleWindow());
  connect(ui_.action_debug_console, &QAction::triggered, [this](bool checked) {showConsole(checked); });
#else //WIN32
  ui_.action_debug_console->setVisible(false);
#endif // WIN32

  //////////////////////////////////////////////////////////////////////////////
  //// Initial layout                                                       ////
  //////////////////////////////////////////////////////////////////////////////
  if (QEcalPlay::instance()->isMeasurementLoaded())
  {
    measurementLoaded(QEcalPlay::instance()->measurementPath());
  }
  else
  {
    measurementClosed();
  }

  frameDroppingAllowedChanged       (QEcalPlay::instance()->isFrameDroppingAllowed());
  enforceDelayAccuracyEnabledChanged(QEcalPlay::instance()->isEnforceDelayAccuracyEnabled());
  limitPlaySpeedEnabledChanged      (QEcalPlay::instance()->isLimitPlaySpeedEnabled());

  saveInitialLayout();
  restoreLayout();

  updateScenariosModified();

  //Drag & Drop
  setAcceptDrops(true);
}

void EcalplayGui::closeEvent(QCloseEvent* event)
{
  bool may_continue = askToSaveScenarios();

  if(!may_continue)
  {
    event->ignore();
    return;
  }

  if (QEcalPlay::instance()->isMeasurementLoaded())
  {
    QEcalPlay::instance()->saveChannelMapping();
  }
  saveLayout();
  QMainWindow::closeEvent(event);
}

EcalplayGui::~EcalplayGui()
{
}

////////////////////////////////////////////////////////////////////////////////
//// Slots for QEcalPlay                                                    ////
////////////////////////////////////////////////////////////////////////////////

void EcalplayGui::measurementLoaded(const QString& path)
{
  measurement_loaded_     = true;
  measurement_boundaries_ = QEcalPlay::instance()->measurementBoundaries();

  ui_.action_play                ->setEnabled(true);
  ui_.action_stop                ->setEnabled(true);
  ui_.action_step_forward        ->setEnabled(true);
  ui_.action_connect_to_ecal     ->setEnabled(true);

  ui_.action_save_channel_mapping->setEnabled(true);
  ui_.action_load_channel_mapping->setEnabled(true);

  measurement_frame_count_ = QEcalPlay::instance()->frameCount();

  setWindowFilePath(path);
  measurement_path_label_->setText(" " + QDir::toNativeSeparators(path) + " ");
  addRecentMeasurement(path);

#ifdef WIN32
  thumbnail_play_pause_button_  ->setEnabled(true);
  thumbnail_play_pause_button_  ->setIcon(play_pause_button_state_is_play_ ? taskbar_play_icon_ : taskbar_pause_icon_);

  thumbnail_stop_button_        ->setEnabled(true);
  thumbnail_stop_button_        ->setIcon(taskbar_stop_icon_);

  thumbnail_step_button_        ->setEnabled(true);
  thumbnail_step_button_        ->setIcon(taskbar_step_icon_);

  updateTaskbarProgressRange();
#endif //WIN32
}

void EcalplayGui::measurementClosed()
{
  measurement_loaded_ = false;
  measurement_boundaries_ = QEcalPlay::instance()->measurementBoundaries();

  ui_.action_play                ->setEnabled(false);
  ui_.action_stop                ->setEnabled(false);
  ui_.action_step_forward        ->setEnabled(false);
  ui_.action_connect_to_ecal     ->setEnabled(false);

  ui_.action_save_channel_mapping->setEnabled(false);
  ui_.action_load_channel_mapping->setEnabled(false);

  measurement_frame_count_ = 0;

  setWindowFilePath("");
  measurement_path_label_->setText(tr(" No measurement loaded "));

#ifdef WIN32
  thumbnail_play_pause_button_  ->setEnabled(false);
  thumbnail_play_pause_button_  ->setIcon(play_pause_button_state_is_play_ ? taskbar_play_icon_disabled_ : taskbar_pause_icon_disabled_);

  thumbnail_stop_button_        ->setEnabled(false);
  thumbnail_stop_button_        ->setIcon(taskbar_stop_icon_disabled_);

  thumbnail_step_button_        ->setEnabled(false);
  thumbnail_step_button_        ->setIcon(taskbar_step_icon_disabled_);

  updateTaskbarProgressRange();
#endif //WIN32
}

void EcalplayGui::publishersInitStateChanged(bool publishers_initialized)
{
  if (publishers_initialized)
  {
    setConnectActionToDisconnect();
  }
  else
  {
    setConnectActionToConnect();
  }
}

void EcalplayGui::playStateChanged(const EcalPlayState& current_state)
{
  if (current_state.playing_)
  {
    setPlayPauseActionToPause();
  }
  else
  {
    setPlayPauseActionToPlay();
  }

  current_frame_label_->setText(tr(" Frame: ") + QString::number(current_state.current_frame_index + 1) + " / " + QString::number(measurement_frame_count_) + " ");
  current_speed_label_->setText(tr(" Current Speed: ") + QString::number(current_state.actual_play_rate_, 'f', 2) + " ");
}

void EcalplayGui::repeatEnabledChanged(bool enabled)
{
  if (enabled != ui_.action_repeat->isChecked())
  {
    ui_.action_repeat->blockSignals(true);
    ui_.action_repeat->setChecked(enabled);
    ui_.action_repeat->blockSignals(false);
  }
}

void EcalplayGui::playSpeedChanged(double /*play_speed*/)
{}

void EcalplayGui::frameDroppingAllowedChanged(bool allowed)
{
  if (allowed != ui_.action_allow_framedrops->isChecked())
  {
    ui_.action_allow_framedrops->blockSignals(true);
    ui_.action_allow_framedrops->setChecked(allowed);
    ui_.action_allow_framedrops->blockSignals(false);
  }
}

void EcalplayGui::enforceDelayAccuracyEnabledChanged(bool enabled)
{
  if (enabled != ui_.action_enforce_delay_accuracy->isChecked())
  {
    ui_.action_enforce_delay_accuracy->blockSignals(true);
    ui_.action_enforce_delay_accuracy->setChecked(enabled);
    ui_.action_enforce_delay_accuracy->blockSignals(false);
  }
}

void EcalplayGui::limitPlaySpeedEnabledChanged(bool enabled)
{
  if (enabled != ui_.action_limit_playback_speed->isChecked())
  {
    ui_.action_limit_playback_speed->blockSignals(true);
    ui_.action_limit_playback_speed->setChecked(enabled);
    ui_.action_limit_playback_speed->blockSignals(false);
  }
}

void EcalplayGui::stepReferenceChannelChanged(const QString& step_reference_channel)
{
  if (step_reference_channel.isEmpty())
  {
    ui_.action_step_channel->setText("Step channel");
    ui_.action_step_channel->setEnabled(false);
#ifdef WIN32
    thumbnail_step_channel_button_->setEnabled(false);
    thumbnail_step_channel_button_->setToolTip("Step channel");
    thumbnail_step_channel_button_->setIcon(taskbar_step_channel_icon_disabled_);
#endif //WIN32
  }
  else
  {
    auto channel_mapping = QEcalPlay::instance()->channelMapping();
    auto target_channel_it = channel_mapping.find(step_reference_channel.toStdString());

    if (target_channel_it != channel_mapping.end())
    {
      QString description = tr("Step") + " \"" + target_channel_it->second.c_str() + "\"";
      ui_.action_step_channel->setText(tr("Step") + " \"" + target_channel_it->second.c_str() + "\"");
#ifdef WIN32
      thumbnail_step_channel_button_->setToolTip(description);
#endif //WIN32
    }
    else
    {
      QString description = tr("Step") + " \"" + step_reference_channel + "\"";
      ui_.action_step_channel->setText(description);
#ifdef WIN32
      thumbnail_step_channel_button_->setToolTip(description);
#endif //WIN32
    }

    ui_.action_step_channel->setEnabled(true);
#ifdef WIN32
    thumbnail_step_channel_button_->setEnabled(true);
    thumbnail_step_channel_button_->setIcon(taskbar_step_channel_icon_);
#endif //WIN32
  }
}

void EcalplayGui::channelMappingChanged(const std::map<std::string, std::string>& /*channel_mapping*/)
{
  stepReferenceChannelChanged(QEcalPlay::instance()->stepReferenceChannel());
}

void EcalplayGui::channelMappingFileActionChanged(QEcalPlay::ChannelMappingFileAction action)
{
  default_channel_mapping_action_group_->blockSignals(true);
  switch (action)
  {
  case QEcalPlay::ChannelMappingFileAction::Ask:
    ui_.action_default_mapping_file_ask->setChecked(true);
    break;
  case QEcalPlay::ChannelMappingFileAction::Load:
    ui_.action_default_mapping_file_load->setChecked(true);
    break;
  case QEcalPlay::ChannelMappingFileAction::Ignore:
    ui_.action_default_mapping_file_ignore->setChecked(true);
    break;
  }
  default_channel_mapping_action_group_->blockSignals(false);
}

////////////////////////////////////////////////////////////////////////////////
//// Helpers                                                                ////
////////////////////////////////////////////////////////////////////////////////

void EcalplayGui::setPlayPauseActionToPlay()
{
  if (!play_pause_button_state_is_play_)
  {
    ui_.action_play->setText(tr("Play"));
    ui_.action_play->setIcon(QPixmap(":/ecalicons/START"));
    play_pause_button_state_is_play_ = true;

#ifdef WIN32
    thumbnail_play_pause_button_->setToolTip("Play");
    thumbnail_play_pause_button_->setIcon(thumbnail_play_pause_button_->isEnabled() ? taskbar_play_icon_ : taskbar_pause_icon_disabled_);
#endif // WIN32
  }
}

void EcalplayGui::setPlayPauseActionToPause()
{
  if (play_pause_button_state_is_play_)
  {
    ui_.action_play->setText(tr("Pause"));
    ui_.action_play->setIcon(QPixmap(":/ecalicons/PAUSE"));
    play_pause_button_state_is_play_ = false;

#ifdef WIN32
    thumbnail_play_pause_button_->setToolTip("Pause");
    thumbnail_play_pause_button_->setIcon(thumbnail_play_pause_button_->isEnabled() ? taskbar_pause_icon_ : taskbar_pause_icon_disabled_);
#endif // WIN32
  }
}

void EcalplayGui::setConnectActionToConnect()
{
  if (!connect_to_ecal_button_state_is_connect_)
  {
    ui_.action_connect_to_ecal->setText(tr("Connect to eCAL"));
    ui_.action_connect_to_ecal->setIcon(QPixmap(":/ecalicons/CHECKMARK"));
    connect_to_ecal_button_state_is_connect_ = true;
  }
}

void EcalplayGui::setConnectActionToDisconnect()
{
  if (connect_to_ecal_button_state_is_connect_)
  {
    ui_.action_connect_to_ecal->setText(tr("Disconnect from eCAL"));
    ui_.action_connect_to_ecal->setIcon(QPixmap(":/ecalicons/CROSS"));
    connect_to_ecal_button_state_is_connect_ = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
//// Recent measurements                                                    ////
////////////////////////////////////////////////////////////////////////////////
void EcalplayGui::populateRecentMeasurementsMenu()
{
  ui_.menu_recent_measurement->clear();
  
  QSettings settings;
  QVariant recent_measurements_variant = settings.value("recent_measurements");
  
  QStringList recent_measurements = recent_measurements_variant.toStringList();

  if (!recent_measurements.empty())
  {
    int counter = 1;
    for (const QString& measurement : recent_measurements)
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

      QAction* meas_action = ui_.menu_recent_measurement->addAction(label + ": " + measurement);
      meas_action->setToolTip(measurement);
      connect(meas_action, &QAction::triggered, this, [this, measurement]() { this->loadMeasurement(measurement); });
      counter++;
    }
  }
  else
  {
    QAction* dummy_action = ui_.menu_recent_measurement->addAction("No recent measurements");
    dummy_action->setEnabled(false);
  }

  ui_.menu_recent_measurement->addSeparator();
  QAction* clear_recent_measurements_list = ui_.menu_recent_measurement->addAction("Clear list");
  connect(clear_recent_measurements_list, &QAction::triggered, this,
      []()
      {
        QSettings settings;
        settings.setValue("recent_measurements", QStringList());
      });
}

void EcalplayGui::addRecentMeasurement(const QString& measurement)
{
  QDir meas_dir(measurement);
  QString absolute_path = meas_dir.absolutePath();
  absolute_path = QDir::toNativeSeparators(absolute_path);
  
  QSettings settings;
  QVariant recent_measurements_variant = settings.value("recent_measurements");
  QStringList recent_measurements = recent_measurements_variant.toStringList();

  if (recent_measurements.contains(absolute_path))
  {
    recent_measurements.removeAll(absolute_path);
  }
  recent_measurements.push_front(absolute_path);

  while (recent_measurements.size() > 10)
  {
    recent_measurements.removeLast();
  }

  settings.setValue("recent_measurements", recent_measurements);
}

void EcalplayGui::updateScenariosModified()
{
  ui_.action_save_scenarios->setEnabled(QEcalPlay::instance()->scenariosModified());
  setWindowModified(QEcalPlay::instance()->scenariosModified());
}


////////////////////////////////////////////////////////////////////////////////
//// Save Layout                                                            ////
////////////////////////////////////////////////////////////////////////////////

void EcalplayGui::saveLayout()
{
  QSettings settings;
  settings.beginGroup("main_window");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("state", saveState(EcalPlayGlobals::Version()));
  settings.endGroup();
}

void EcalplayGui::restoreLayout()
{
  QSettings settings;
  settings.beginGroup("main_window");

  QVariant geometry_variant = settings.value("geometry");
  QVariant state_variant = settings.value("state");

  if (geometry_variant.isValid() && state_variant.isValid())
  {
    restoreGeometry(geometry_variant.toByteArray());
    restoreState(state_variant.toByteArray(), EcalPlayGlobals::Version());
  }

  settings.endGroup();
}

void EcalplayGui::saveInitialLayout()
{
  initial_geometry_ = saveGeometry();
  initial_state_    = saveState();
}

void EcalplayGui::resetLayout()
{
  player_control_widget_->resetLayout();
  channel_widget_       ->resetLayout();
  scenario_widget_      ->resetLayout();
  
  int screen_number = QApplication::desktop()->screenNumber(this);
  restoreGeometry(initial_geometry_);
  restoreState(initial_state_);
  move(QGuiApplication::screens().at(screen_number)->availableGeometry().center() - rect().center());
}

bool EcalplayGui::loadMeasurementFromFileDialog()
{
  const bool may_continue = askToSaveScenarios();

  if (may_continue)
  {
    return QEcalPlay::instance()->loadMeasurementFromFileDialog();
  }

  return false;
}

bool EcalplayGui::loadMeasurement(const QString& path)
{
  const bool may_continue = askToSaveScenarios();

  if (may_continue)
  {
    return QEcalPlay::instance()->loadMeasurement(path);
  }

  return false;
}

bool EcalplayGui::closeMeasurement()
{
  const bool may_continue = askToSaveScenarios();

  if (may_continue)
  {
    QEcalPlay::instance()->closeMeasurement();
  }

  return may_continue;
}

bool EcalplayGui::askToSaveScenarios()
{
  bool may_continue = true;

  if (QEcalPlay::instance()->scenariosModified())
  {
    QMessageBox config_modified_warning(
      QMessageBox::Icon::Warning
      , tr("The Labels have been modified")
      , tr("Do you want to save your changes?")
      , QMessageBox::Button::Save | QMessageBox::Button::Discard | QMessageBox::Button::Cancel
      , this);

    int user_choice = config_modified_warning.exec();

    if (user_choice == QMessageBox::Button::Save)
    {
      may_continue = QEcalPlay::instance()->saveScenariosToDisk();
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

////////////////////////////////////////////////////////////////////////////////
//// Drag & drop                                                            ////
////////////////////////////////////////////////////////////////////////////////
void EcalplayGui::dragEnterEvent(QDragEnterEvent* event)
{
  const QMimeData* mime_data = event->mimeData();

  if (mime_data->hasUrls())
  {
    QList<QUrl> url_list = mime_data->urls();

    if (url_list.size() == 1)
    {
      event->acceptProposedAction();
      return;
    }
  }
  QWidget::dragEnterEvent(event);

}

void EcalplayGui::dragMoveEvent(QDragMoveEvent* event)
{
  QWidget::dragMoveEvent(event);
}

void EcalplayGui::dragLeaveEvent(QDragLeaveEvent* event)
{
  QWidget::dragLeaveEvent(event);
}

void EcalplayGui::dropEvent(QDropEvent* event)
{
  const QMimeData* mime_data = event->mimeData();

  if (mime_data->hasUrls())
  {
    QList<QUrl> url_list = mime_data->urls();

    if (url_list.size() == 1)
    {
      event->acceptProposedAction();
      loadMeasurement(url_list.front().toLocalFile());
      return;
    }
  }
  QWidget::dropEvent(event);
}

#ifdef WIN32
////////////////////////////////////////////////////////////////////////////////
//// Windows Taskbar                                                        ////
////////////////////////////////////////////////////////////////////////////////
void EcalplayGui::showEvent(QShowEvent* /*event*/)
{
  taskbar_button_   ->setWindow(this->windowHandle());
  thumbnail_toolbar_->setWindow(this->windowHandle());
}

void EcalplayGui::updateTaskbarProgress(const EcalPlayState& current_state)
{
  QWinTaskbarProgress* progress = taskbar_button_->progress();
  progress->setPaused(!current_state.playing_);
  progress->setValue(std::chrono::duration_cast<std::chrono::milliseconds>(current_state.current_frame_timestamp - measurement_boundaries_.first).count());

  if (measurement_loaded_ && QEcalPlay::instance()->isInitialized())
  {
    if (current_state.playing_)
    {
      taskbar_button_->setOverlayIcon(QIcon(":/ecalicons/TASKBAR_PLAY"));
    }
    else
    {
      taskbar_button_->setOverlayIcon(QIcon(":/ecalicons/TASKBAR_PAUSE"));
    }
  }
  else
  {
    taskbar_button_->clearOverlayIcon();
  }
}

void EcalplayGui::updateTaskbarProgressRange()
{
  QWinTaskbarProgress* progress = taskbar_button_->progress();
  progress->setVisible(measurement_loaded_);

  progress->setMinimum(0);
  progress->setMaximum(std::chrono::duration_cast<std::chrono::milliseconds>(measurement_boundaries_.second - measurement_boundaries_.first).count());
}

void EcalplayGui::showConsole(bool show)
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
