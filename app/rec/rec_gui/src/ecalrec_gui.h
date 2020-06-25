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

#include <QIcon>

#ifdef WIN32
#include <QWinTaskbarButton>
#include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>
#include <QIcon>
#endif // WIN32

class EcalRecGui : public QMainWindow
{
  Q_OBJECT

//////////////////////////////////////////
// Constructor & Destructor
//////////////////////////////////////////
public:
  EcalRecGui(QWidget *parent = Q_NULLPTR);
  ~EcalRecGui();

protected:
  void closeEvent(QCloseEvent* event) override;

//////////////////////////////////////////
// Save layout
//////////////////////////////////////////
protected:
  void showEvent(QShowEvent *event) override;

private:
  void saveLayout();
  void restoreLayout();
  void saveInitialLayout();

public slots:
  void resetLayout();

//////////////////////////////////////////
// Private slots
//////////////////////////////////////////

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

  bool clearConfig();
  bool saveConfigAs();
  bool saveConfig();
  bool saveConfig(const QString& path);

  bool askToSaveFile();

  bool openConfigWithDialogs(const QString& path = "");
  bool openConfig(const QString& path);

  void configHasBeenModifiedChanged(bool modified);
  void loadedConfigPathChanged(const std::string& path, int version);

  void addToRecentFileList(const std::string& path);
  void updateRecentFilesMenu();
  void clearRecentFiles();

  void addCommentToLastMeas();

  void showUploadSettingsDialog();

////////////////////////////////////////////
// Member variables
////////////////////////////////////////////
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

  QStringList recent_file_list_;

  // initial layout
  bool       first_show_event_;
  QByteArray initial_geometry_;
  QByteArray initial_state_;
  bool       initial_show_disabled_elements_at_the_bottom_;
  bool       initial_alternating_row_colors_;

#ifdef WIN32
////////////////////////////////////////////
// Windows specific
////////////////////////////////////////////
private:
  void registerTaskbarButtons();

private slots:
  void showConsole(bool show);
  void updateTaskbarButton(const eCAL::rec_server::RecorderStatusMap_T& recorder_statuses);

private:
  QWinTaskbarButton* taskbar_button_;
  QWinThumbnailToolBar* thumbnail_toolbar_;
  QWinThumbnailToolButton* taskbar_activate_button_;
  QWinThumbnailToolButton* taskbar_record_button_;
  QWinThumbnailToolButton* taskbar_save_buffer_button_;

  QIcon taskbar_activate_icon_;
  QIcon taskbar_deactivate_icon_;

  QIcon taskbar_record_icon_;
  QIcon taskbar_record_icon_disabled_;

  QIcon taskbar_save_buffer_icon_;
  QIcon taskbar_save_buffer_icon_disabled_;
#endif // WIN32
};
