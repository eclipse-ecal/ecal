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

#include <QActionGroup>
#include <QTimer>
#include <QRect>
#include <QByteArray>
#include <QStringList>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/server.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/app/pb/sys/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ui_main_window.h"

#include "widgets/group_edit_widget/group_edit_widget.h"
#include "widgets/groupwidget/group_widget.h"
#include "widgets/import_from_cloud_widget/import_from_cloud_window.h"
#include "widgets/mmawidget/mma_widget.h"
#include "widgets/runnerwidget/runner_window.h"
#include "widgets/taskwidget/task_widget.h"

class EcalsysGui : public QMainWindow
{
  Q_OBJECT

private:
  enum class Theme:int
  {
    Default,
    Dark,
  };

public:
  EcalsysGui(QWidget *parent = Q_NULLPTR);

  ~EcalsysGui();

protected:
  void showEvent(QShowEvent *event) override;

public:
  void saveGuiSettings();
  void loadGuiSettings();

  void setTheme(Theme theme);

  void setUnmodified() { config_has_been_modified_ = false; };

protected:
  void closeEvent(QCloseEvent* event) override;

  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent (QDragMoveEvent* event)  override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dropEvent     (QDropEvent* event)      override;

private:
  Ui::EcalsysMainWindow  ui_;
  bool                   first_show_event_;

  QStringList last_config_list;

  // Values for reset-layout
  QByteArray             initial_geometry_;
  QByteArray             initial_state_;

  QPalette               initial_palette_;
  QStyle*                initial_style_;
  QString                initial_style_sheet_;

  bool                   config_has_been_modified_;
  QActionGroup*          task_target_action_group_;
  QTimer*                button_update_timer_;

  QActionGroup*          theme_action_group_;

  std::unique_ptr<eCAL::protobuf::CServiceServer<eCAL::pb::sys::Service>> ecalsys_service_;

  // Custom Widgets
  TaskWidget*            task_widget_;
  GroupWidget*           group_widget_;
  GroupEditWidget*       group_edit_widget_;
  MmaWidget*             mma_widget_;
  RunnerWindow*          runner_window_;
  ImportFromCloudWindow* import_from_cloud_window_;

  QWidget*               task_modified_statusbar_widget_;

  void applyInitialLayout();
  void monitorUpdateCallback();

  void createWidgets();
  static QMetaObject::Connection connectIfPossible(const QObject *sender, const char *signal, const QObject *receiver, const char *member, Qt::ConnectionType = Qt::AutoConnection);
  bool askToSaveFile();

  void updateHostActions(QMenu* menu);

  void updateRecentFiles();
  void updateRecentFiles(const QString& new_config_file);
  void clearRecentFiles();

private slots:
  void menuFileNewTriggered();
  void menuFileOpenTriggered();
  void menuFileAppendTriggered();
  void menuFileSaveTriggered();
  bool menuFileSaveAsTriggered();
  void menuEditRunnersTriggered();
  void menuViewResetLayoutTriggered();
  void menuOptionsCheckTargetsReachableToggled(bool enabled);
  void menuOptionsTargetGroupTriggered(QAction* action);
  void menuOptionsKillAllOnCloseToggled(bool enabled);
  void menuHelpDocumentationTriggered();
  void menuHelpAboutTriggered();
  void menuHelpLicensesTriggered();

#ifdef WIN32
  void showConsole(bool show);
#endif // WIN32


  void performStartupChecks();
  void updateTaskModifiedWarning();
  void setConfigModified();

  void updateMenuOptions();
  void updateStartStopButtons();
  void updateTaskEditButtons();
  void updateGroupEditButtons();
  void setWindowFilePathSlot(const QString& file_path);

  void showImportFromCloudWindow();

  void openFile(QString path, bool append = false);

signals:
  void monitorUpdatedSignal();
  void ecalsysOptionsChangedSignal();
  void configChangedSignal(const QString& current_config_path);                 /**< Everything has changed, e.g. by loading a new config*/
  void startStopActionTriggeredSignal();
};
