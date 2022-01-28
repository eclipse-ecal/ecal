/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "ecalsys_gui.h"
#include "ecalsys_service.h"
#include <sstream>
#include <thread>

#include "globals.h"
#include "ecalsys/ecal_sys_logger.h"

#include "widgets/runnerwidget/runner_window.h"
#include "widgets/about_dialog/about_dialog.h"
#include "widgets/license_dialog/license_dialog.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QColor>
#include <QSettings>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include <QFileInfo>
#include <QDesktopServices>
#include <QMimeData>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QDesktopServices>
#include <QUrl>
#include <QStyleFactory>

#ifdef WIN32
#include <Windows.h>
#endif

EcalsysGui::EcalsysGui(QWidget *parent)
  : QMainWindow(parent)
  , first_show_event_(true)
  , config_has_been_modified_(false)
{
  ui_.setupUi(this);

  if (Globals::EcalSysInstance()->GetCurrentConfigPath() != "")
  {
    setWindowFilePath(Globals::EcalSysInstance()->GetCurrentConfigPath().c_str());
  }
  else
  {
    setWindowFilePath("New Config");
  }

  Globals::EcalSysInstance()->SetMonitorUpdateCallback(std::bind(&EcalsysGui::monitorUpdateCallback, this));

  // Create the main custom widgets and connect all of their signals and slots
  createWidgets();

  task_target_action_group_ = new QActionGroup(this);
  task_target_action_group_->setExclusive(true);
  task_target_action_group_->addAction(ui_.action_options_use_configured_targets);
  task_target_action_group_->addAction(ui_.action_options_local_tasks_only);
  task_target_action_group_->addAction(ui_.action_options_use_localhost_for_all_tasks);

  // Set the initial layout
  applyInitialLayout();

  updateMenuOptions();

  
  // Windows menu
  QList<QAction*> dock_actions = createPopupMenu()->actions();
  for (QAction* action : dock_actions)
  {
    ui_.menu_windows->addAction(action);
  }
  
  // Menu Callbacks
  connect(ui_.action_file_new,                        SIGNAL(triggered()),                this, SLOT(menuFileNewTriggered()));
  connect(ui_.action_file_open,                       SIGNAL(triggered()),                this, SLOT(menuFileOpenTriggered()));
  connect(ui_.action_file_append,                     SIGNAL(triggered()),                this, SLOT(menuFileAppendTriggered()));
  connect(ui_.action_file_save,                       SIGNAL(triggered()),                this, SLOT(menuFileSaveTriggered()));
  connect(ui_.action_file_save_as,                    SIGNAL(triggered()),                this, SLOT(menuFileSaveAsTriggered()));
  connect(ui_.action_file_exit,                       SIGNAL(triggered()),                this, SLOT(close()));
  connect(ui_.action_view_runners,                    SIGNAL(triggered()),                this, SLOT(menuEditRunnersTriggered()));
  connect(ui_.action_view_reset_layout,               SIGNAL(triggered()),                this, SLOT(menuViewResetLayoutTriggered()));
  connect(ui_.action_options_check_targets_reachable, SIGNAL(toggled(bool)),              this, SLOT(menuOptionsCheckTargetsReachableToggled(bool)));
  connect(task_target_action_group_,                  SIGNAL(triggered(QAction*)),        this, SLOT(menuOptionsTargetGroupTriggered(QAction*)));
  connect(ui_.action_options_kill_all_on_close,       SIGNAL(toggled(bool)),              this, SLOT(menuOptionsKillAllOnCloseToggled(bool)));
  connect(ui_.action_help_documentation,              SIGNAL(triggered()),                this, SLOT(menuHelpDocumentationTriggered()));
  connect(ui_.action_help_about,                      SIGNAL(triggered()),                this, SLOT(menuHelpAboutTriggered()));
  connect(ui_.action_help_licenses,                   SIGNAL(triggered()),                this, SLOT(menuHelpLicensesTriggered()));

  theme_action_group_ = new QActionGroup(this);
  theme_action_group_->addAction(ui_.action_theme_default);
  theme_action_group_->addAction(ui_.action_theme_dark);
  connect(theme_action_group_, &QActionGroup::triggered, this, [this](QAction* action)
    {
      if(action == ui_.action_theme_default)
        this->setTheme(Theme::Default);
      else if(action == ui_.action_theme_dark)
        this->setTheme(Theme::Dark);
    });

  connect(ui_.action_task_add,                   &QAction::triggered, [=]() {ui_.tasks_dockwidget->raise(); task_widget_->addTask(); });
  connect(ui_.action_task_edit,                  &QAction::triggered, [=]() {ui_.tasks_dockwidget->raise(); task_widget_->setEditControlsVisibility(); });
  connect(ui_.action_task_duplicate,             &QAction::triggered, [=]() {ui_.tasks_dockwidget->raise(); task_widget_->duplicateSelectedTasks(); });
  connect(ui_.action_task_remove,                &QAction::triggered, [=]() {ui_.tasks_dockwidget->raise(); task_widget_->removeSelectedTasks(); });
  connect(ui_.action_task_start,                 &QAction::triggered, [=]() {task_widget_->startAllTasks(); });
  connect(ui_.action_task_stop,                  &QAction::triggered, [=]() {task_widget_->stopAllTasks(); });
  connect(ui_.action_task_restart,               &QAction::triggered, [=]() {task_widget_->restartAllTasks(); });
  connect(ui_.action_task_start_selected,        &QAction::triggered, [=]() {ui_.tasks_dockwidget->raise(); task_widget_->startSelectedTasks(); });
  connect(ui_.action_task_stop_selected,         &QAction::triggered, [=]() {ui_.tasks_dockwidget->raise(); task_widget_->stopSelectedTasks(); });
  connect(ui_.action_task_restart_selected,      &QAction::triggered, [=]() {ui_.tasks_dockwidget->raise(); task_widget_->restartSelectedTasks(); });

  connect(ui_.action_task_fast_kill,             &QAction::triggered, [=]() {task_widget_->stopAllTasks(false, true); });
  connect(ui_.action_task_fast_restart,          &QAction::triggered, [=]() {task_widget_->restartAllTasks(false, true); });
  connect(ui_.action_task_fast_kill_selected,    &QAction::triggered, [=]() {ui_.tasks_dockwidget->raise(); task_widget_->stopSelectedTasks(false, true); });
  connect(ui_.action_task_fast_restart_selected, &QAction::triggered, [=]() {ui_.tasks_dockwidget->raise(); task_widget_->restartSelectedTasks(false, true); });
  for (auto action : ui_.menu_task_target_override->actions())
  {
    if (action->menu())
    {
      connect(action->menu(), &QMenu::aboutToShow, [=]() {this->updateHostActions(action->menu()); });
    }
  }
  connect(ui_.menu_task_start_all_on_host,        &QMenu::triggered,   [=](QAction* action) {this->task_widget_->startAllTasks(action->text().toStdString()); });
  connect(ui_.menu_task_restart_all_on_host,      &QMenu::triggered,   [=](QAction* action) {this->task_widget_->restartAllTasks(true, true, action->text().toStdString()); });
  connect(ui_.menu_task_start_selected_on_host,   &QMenu::triggered,   [=](QAction* action) {ui_.tasks_dockwidget->raise(); this->task_widget_->startSelectedTasks(action->text().toStdString()); });
  connect(ui_.menu_task_restart_selected_on_host, &QMenu::triggered,   [=](QAction* action) {ui_.tasks_dockwidget->raise(); this->task_widget_->restartSelectedTasks(true, true, action->text().toStdString()); });
  connect(ui_.action_task_import_from_cloud,      &QAction::triggered, [=]() {this->showImportFromCloudWindow(); });
  connect(ui_.action_task_update_from_cloud,      &QAction::triggered, [=]() {task_widget_->updateFromCloud(); });

  connect(ui_.action_group_add,                   &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->addGroup(); });
  connect(ui_.action_group_edit,                  &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->showEditControls(); });
  connect(ui_.action_group_duplicate,             &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->duplicateSelectedGroups(); });
  connect(ui_.action_group_remove,                &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->removeSelectedGroups(); });
  connect(ui_.action_group_start_selected,        &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->startSelectedTasks(); });
  connect(ui_.action_group_stop_selected,         &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->stopSelectedTasks(); });
  connect(ui_.action_group_restart_selected,      &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->restartSelectedTasks(); });
  connect(ui_.action_group_fast_kill_selected,    &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->stopSelectedTasks(false, true); });
  connect(ui_.action_group_fast_restart_selected, &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->restartSelectedTasks(false, true); });
  for (auto action : ui_.menu_group_target_override->actions())
  {
    if (action->menu())
    {
      connect(action->menu(), &QMenu::aboutToShow, [=]() {this->updateHostActions(action->menu()); });
    }
  }
  connect(ui_.menu_group_start_selected_on_host,   &QMenu::triggered,   [=](QAction* action) {ui_.groups_dockwidget->raise(); group_widget_->startSelectedTasks(action->text().toStdString()); });
  connect(ui_.menu_group_restart_selected_on_host, &QMenu::triggered,   [=](QAction* action) {ui_.groups_dockwidget->raise(); group_widget_->restartSelectedTasks(true, true, action->text().toStdString()); });
  connect(ui_.action_group_expand,                 &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->expandGroups(); });
  connect(ui_.action_group_collapse,               &QAction::triggered, [=]() {ui_.groups_dockwidget->raise(); group_widget_->collapseGroups(); });

  // Special show-console button for Windows
#ifdef WIN32
  ui_.action_show_console->setChecked(GetConsoleWindow());
  connect(ui_.action_show_console, &QAction::triggered, [this](bool checked) {showConsole(checked); });
#else //WIN32
  ui_.action_show_console->setVisible(false);
#endif // WIN32


  // Perform startup checks after a few seconds
  QTimer::singleShot(1000, this, SLOT(performStartupChecks()));

  updateStartStopButtons();
  updateTaskEditButtons();
  updateGroupEditButtons();

  // Create the eCALSys service
  std::shared_ptr<eCALSysServiceImpl> ecalsys_service_impl = std::make_shared<eCALSysServiceImpl>(this);
  ecalsys_service_.Create(ecalsys_service_impl);

  //Drag & Drop
  setAcceptDrops(true);
}

EcalsysGui::~EcalsysGui()
{}

void EcalsysGui::showEvent(QShowEvent* /*event*/)
{
  if (first_show_event_)
  {
    // Save the initial layout for restoring it later
    initial_geometry_ = saveGeometry();
    initial_state_    = saveState();

    initial_style_sheet_ = qApp->styleSheet();
    initial_palette_     = qApp->palette();
    initial_style_       = qApp->style();

    // Load settings from the last session
    loadGuiSettings();
  }
  first_show_event_ = false;
}

void EcalsysGui::saveGuiSettings()
{
  QSettings settings;
  settings.beginGroup("mainwindow");
  settings.setValue("geometry", saveGeometry());
  settings.setValue("window_state", saveState(Globals::ecalSysVersion()));

  if(ui_.action_theme_default->isChecked())
    settings.setValue("theme", static_cast<int>(Theme::Default));
  else if (ui_.action_theme_dark->isChecked())
    settings.setValue("theme", static_cast<int>(Theme::Dark));

  settings.endGroup();
}

void EcalsysGui::loadGuiSettings()
{
  QSettings settings;
  settings.beginGroup("mainwindow");

  QVariant theme_variant = settings.value("theme");
  if (theme_variant.isValid())
  {
    if (theme_variant.toInt() == static_cast<int>(Theme::Dark))
      setTheme(Theme::Dark);
  }

  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("window_state").toByteArray(), Globals::ecalSysVersion());

  last_config_list = settings.value("last_configs").toStringList();
  if (Globals::EcalSysInstance()->GetCurrentConfigPath() != "")
  {
    updateRecentFiles(Globals::EcalSysInstance()->GetCurrentConfigPath().c_str());
  }
  else
  {
    updateRecentFiles();
  }

  settings.endGroup();
}

void EcalsysGui::setTheme(Theme theme)
{
  if (theme == Theme::Default)
  {
    theme_action_group_->blockSignals(true);
    ui_.action_theme_default->setChecked(true);
    theme_action_group_->blockSignals(false);

    qApp->setStyle     (initial_style_);
    qApp->setPalette   (initial_palette_);
    qApp->setStyleSheet(initial_style_sheet_);
  }
  else if (theme == Theme::Dark)
  {
    theme_action_group_->blockSignals(true);
    ui_.action_theme_dark->setChecked(true);
    theme_action_group_->blockSignals(false);

    initial_style_->setParent(this); // Prevent deleting the initial style
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QColor darkGray( 58,  58,  58);
    QColor gray    (128, 128, 128);
    QColor black   ( 31,  31,  31);
    QColor blue    ( 42, 130, 218);

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window         , darkGray);
    darkPalette.setColor(QPalette::WindowText     , Qt::white);
    darkPalette.setColor(QPalette::Base           , black);
    darkPalette.setColor(QPalette::AlternateBase  , darkGray);
    darkPalette.setColor(QPalette::ToolTipBase    , blue);
    darkPalette.setColor(QPalette::ToolTipText    , Qt::white);
    darkPalette.setColor(QPalette::Text           , Qt::white);
    darkPalette.setColor(QPalette::Button         , darkGray);
    darkPalette.setColor(QPalette::ButtonText     , Qt::white);
    darkPalette.setColor(QPalette::Link           , blue);
    darkPalette.setColor(QPalette::Highlight      , blue);
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    darkPalette.setColor(QPalette::Active,   QPalette::Button,     gray.darker());
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text,       gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::Light,      darkGray);

    qApp->setPalette(darkPalette);

    QString style_sheet;
    style_sheet += "QToolTip { color: #ffffff; background-color: #2b2b2b; border: 1px solid #767676; }";
    //style_sheet += "QMenu { background-color: #3A3A3A; border: none; }";
    //style_sheet += "QMenu::item:selected { background-color: #FF0000}";

    qApp->setStyleSheet(style_sheet);
  }
}

void EcalsysGui::createWidgets()
{
  // Create all widgets
  task_widget_              = new TaskWidget(this);
  group_edit_widget_        = new GroupEditWidget(this);
  group_widget_             = new GroupWidget(group_edit_widget_, this);
  mma_widget_               = new MmaWidget(this);

  runner_window_            = new RunnerWindow();
  import_from_cloud_window_ = new ImportFromCloudWindow();

  // Add the three main widgets to their according frames
  ui_.tasks_dockwidget_content_frame_layout ->addWidget(task_widget_);
  ui_.groups_dockwidget_content_frame_layout->addWidget(group_widget_);
  ui_.mma_dockwidget_content_frame_layout   ->addWidget(mma_widget_);

  // Set the initial size
  int total_width = width();
  int mma_width = (int)((double)total_width * 0.27);
  ui_.tasks_dockwidget_content_frame ->setSizeHint(QSize(total_width - mma_width, 0));
  ui_.groups_dockwidget_content_frame->setSizeHint(QSize(total_width - mma_width, 0));
  ui_.mma_dockwidget_content_frame   ->setSizeHint(QSize(mma_width, 0));

  //Create the status bar widget that tells the user to restart tasks if he changed the configuration
  task_modified_statusbar_widget_ = new QWidget(ui_.status_bar);
  QHBoxLayout* task_modified_statusbar_widget_layout = new QHBoxLayout(task_modified_statusbar_widget_);
  task_modified_statusbar_widget_layout->setContentsMargins(0, 0, 0, 0);
  QLabel* task_modified_text_label = new QLabel(ui_.status_bar);
  QLabel* task_modified_icon_label = new QLabel(ui_.status_bar);
  task_modified_text_label->setText(tr("Restart task(s) for changes to take effect"));
  int label_height = task_modified_text_label->sizeHint().height();
  QPixmap warning_icon = Globals::Icons::warning().scaled(label_height, label_height, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
  task_modified_icon_label->setPixmap(warning_icon);
  task_modified_statusbar_widget_layout->addWidget(task_modified_icon_label);
  task_modified_statusbar_widget_layout->addWidget(task_modified_text_label);
  task_modified_statusbar_widget_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  task_modified_statusbar_widget_->setHidden(true);
  ui_.status_bar->addWidget(task_modified_statusbar_widget_);

  // Connect everything

  // The important slots that follow a more or less generic scheme are:
  //
  //    tasksAdded           (std::vector<std::shared_ptr<EcalSysTask>>)
  //    tasksRemoved         (std::vector<std::shared_ptr<EcalSysTask>>)
  //    tasksModified        (std::vector<std::shared_ptr<EcalSysTask>>)
  //
  //    runnersAdded         (std::vector<std::shared_ptr<EcalSysRunner>>)
  //    runnersRemoved       (std::vector<std::shared_ptr<EcalSysRunner>>)
  //    runnersModified      (std::vector<std::shared_ptr<EcalSysRunner>>)
  // 
  //    groupsAdded          (std::vector<std::shared_ptr<TaskGroup>>)
  //    groupsRemoved        (std::vector<std::shared_ptr<TaskGroup>>)
  //    groupsModified       (std::vector<std::shared_ptr<TaskGroup>>)
  //
  //    configChanged        (const QString&)
  //    ecalsysOptionsChanged()
  //    monitorUpdated       ()
  //
  // Signals are postfixed with Signal, so the tasksAddedSignal will be
  // connected to all tasksAdded Slots.

  std::vector<QObject*> widget_list =
  {
    task_widget_,
    group_edit_widget_,
    group_widget_,
    mma_widget_,
    runner_window_->runner_widget,
    import_from_cloud_window_->import_from_cloud_widget_,
  };

  for (unsigned int i = 0; i < widget_list.size(); i++)
  {
    for (unsigned int j = 0; j < widget_list.size(); j++)
    {
      if (i != j)
      {
        QObject* sender = widget_list[i];
        QObject* receiver = widget_list[j];

        connectIfPossible(sender, SIGNAL(tasksAddedSignal(std::vector<std::shared_ptr<EcalSysTask>>)),        receiver, SLOT(tasksAdded(std::vector<std::shared_ptr<EcalSysTask>>)));
        connectIfPossible(sender, SIGNAL(tasksRemovedSignal(std::vector<std::shared_ptr<EcalSysTask>>)),      receiver, SLOT(tasksRemoved(std::vector<std::shared_ptr<EcalSysTask>>)));
        connectIfPossible(sender, SIGNAL(tasksModifiedSignal(std::vector<std::shared_ptr<EcalSysTask>>)),     receiver, SLOT(tasksModified(std::vector<std::shared_ptr<EcalSysTask>>)));

        connectIfPossible(sender, SIGNAL(runnersAddedSignal(std::vector<std::shared_ptr<EcalSysRunner>>)),    receiver, SLOT(runnersAdded(std::vector<std::shared_ptr<EcalSysRunner>>)));
        connectIfPossible(sender, SIGNAL(runnersRemovedSignal(std::vector<std::shared_ptr<EcalSysRunner>>)),  receiver, SLOT(runnersRemoved(std::vector<std::shared_ptr<EcalSysRunner>>)));
        connectIfPossible(sender, SIGNAL(runnersModifiedSignal(std::vector<std::shared_ptr<EcalSysRunner>>)), receiver, SLOT(runnersModified(std::vector<std::shared_ptr<EcalSysRunner>>)));

        connectIfPossible(sender, SIGNAL(groupsAddedSignal(std::vector<std::shared_ptr<TaskGroup>>)),         receiver, SLOT(groupsAdded(std::vector<std::shared_ptr<TaskGroup>>)));
        connectIfPossible(sender, SIGNAL(groupsRemovedSignal(std::vector<std::shared_ptr<TaskGroup>>)),       receiver, SLOT(groupsRemoved(std::vector<std::shared_ptr<TaskGroup>>)));
        connectIfPossible(sender, SIGNAL(groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>)),      receiver, SLOT(groupsModified(std::vector<std::shared_ptr<TaskGroup>>)));


        connectIfPossible(sender, SIGNAL(tasksAddedSignal(std::vector<std::shared_ptr<EcalSysTask>>)),        this,     SLOT(setConfigModified()));
        connectIfPossible(sender, SIGNAL(tasksRemovedSignal(std::vector<std::shared_ptr<EcalSysTask>>)),      this,     SLOT(setConfigModified()));
        connectIfPossible(sender, SIGNAL(tasksModifiedSignal(std::vector<std::shared_ptr<EcalSysTask>>)),     this,     SLOT(setConfigModified()));

        connectIfPossible(sender, SIGNAL(runnersAddedSignal(std::vector<std::shared_ptr<EcalSysRunner>>)),    this,     SLOT(setConfigModified()));
        connectIfPossible(sender, SIGNAL(runnersRemovedSignal(std::vector<std::shared_ptr<EcalSysRunner>>)),  this,     SLOT(setConfigModified()));
        connectIfPossible(sender, SIGNAL(runnersModifiedSignal(std::vector<std::shared_ptr<EcalSysRunner>>)), this,     SLOT(setConfigModified()));

        connectIfPossible(sender, SIGNAL(groupsAddedSignal(std::vector<std::shared_ptr<TaskGroup>>)),         this,     SLOT(setConfigModified()));
        connectIfPossible(sender, SIGNAL(groupsRemovedSignal(std::vector<std::shared_ptr<TaskGroup>>)),       this,     SLOT(setConfigModified()));
        connectIfPossible(sender, SIGNAL(groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>)),      this,     SLOT(setConfigModified()));
      }
    }
  }

  for (unsigned int i = 0; i < widget_list.size(); i++)
  {
    QObject* receiver = widget_list[i];
    connectIfPossible(this, SIGNAL(configChangedSignal(const QString&)), receiver, SLOT(configChanged()));
    connectIfPossible(this, SIGNAL(ecalsysOptionsChangedSignal()),       receiver, SLOT(ecalsysOptionsChanged()));
    connectIfPossible(this, SIGNAL(monitorUpdatedSignal()),              receiver, SLOT(monitorUpdated()));
  }

  // Internal connections
  connect(this, SIGNAL(configChangedSignal(const QString&)), this, SLOT(updateMenuOptions()));
  connect(this, SIGNAL(configChangedSignal(const QString&)), this, SLOT(setWindowFilePathSlot(const QString&)));

  connect(this, SIGNAL(ecalsysOptionsChangedSignal()),       this, SLOT(updateMenuOptions()));
  connect(this, SIGNAL(ecalsysOptionsChangedSignal()),       this, SLOT(setConfigModified()));

  // Various additional Slots

  // Periodically update the buttons in the TaskWidget and GroupWidget
  button_update_timer_ = new QTimer(this);
  button_update_timer_->start(100);

  connect(button_update_timer_, SIGNAL(timeout()), task_widget_,  SLOT(updateStartStopButtons()));
  connect(button_update_timer_, SIGNAL(timeout()), group_widget_, SLOT(updateStartStopButtons()));
  connect(button_update_timer_, SIGNAL(timeout()), this,          SLOT(updateStartStopButtons()));
  connect(button_update_timer_, SIGNAL(timeout()), this,          SLOT(updateTaskModifiedWarning()));

  connect(group_widget_, &GroupWidget::setEditTaskSignal, [this](std::shared_ptr<EcalSysTask> task) {ui_.tasks_dockwidget->raise(); task_widget_->setEditTask(task); });

  // Also update the buttons whenever anybody triggered a start/stop/restart
  connect(task_widget_,  SIGNAL(startStopActionTriggeredSignal()), group_widget_, SLOT(updateStartStopButtons()));
  connect(task_widget_,  SIGNAL(startStopActionTriggeredSignal()), this,          SLOT(updateStartStopButtons()));
  connect(group_widget_, SIGNAL(startStopActionTriggeredSignal()), task_widget_,  SLOT(updateStartStopButtons()));
  connect(group_widget_, SIGNAL(startStopActionTriggeredSignal()), this,          SLOT(updateStartStopButtons()));
  connect(this,          SIGNAL(startStopActionTriggeredSignal()), task_widget_,  SLOT(updateStartStopButtons()));
  connect(this,          SIGNAL(startStopActionTriggeredSignal()), group_widget_, SLOT(updateStartStopButtons()));
  connect(this,          SIGNAL(startStopActionTriggeredSignal()), this,          SLOT(updateStartStopButtons()));

  connect(task_widget_,  SIGNAL(selectionChangedSignal()),         this,          SLOT(updateStartStopButtons()));
  connect(group_widget_, SIGNAL(selectionChangedSignal()),         this,          SLOT(updateStartStopButtons()));

  connect(task_widget_,  SIGNAL(selectionChangedSignal()),         this,          SLOT(updateTaskEditButtons()));
  connect(group_widget_, SIGNAL(selectionChangedSignal()),         this,          SLOT(updateGroupEditButtons()));

  connect(task_widget_, SIGNAL(importFromCloudButtonClickedSignal()), this, SLOT(showImportFromCloudWindow()));
  connect(task_widget_, &TaskWidget::showRunnerWindowSignal,
    [this](std::shared_ptr<EcalSysRunner> runner)
    {
      runner_window_->runner_widget->setEditRunner(runner);
      menuEditRunnersTriggered();
    });
}

QMetaObject::Connection EcalsysGui::connectIfPossible(const QObject *sender, const char *signal, const QObject *receiver, const char *member, Qt::ConnectionType connection_type)
{
  // Check if the Signal / Slot exists
  // The SLOT   macro prefixes with '1'
  // The SIGNAL macro prefixes with '2'

  int receiver_index = -1;
  int sender_index = -1;

  if (signal[0] == '2')
    sender_index = sender->metaObject()->indexOfSignal(QMetaObject::normalizedSignature((const char*)(signal + sizeof(char))));

  if (member[0] == '1')
    receiver_index = receiver->metaObject()->indexOfSlot(QMetaObject::normalizedSignature((const char*)(member + sizeof(char))));
  else if (member[0] == '2')
    receiver_index = receiver->metaObject()->indexOfSignal(QMetaObject::normalizedSignature((const char*)(member + sizeof(char))));

  if ((receiver_index >= 0) && (sender_index >= 0))
  {
    return connect(sender, signal, receiver, member, connection_type);
  }
  else
  {
    return QMetaObject::Connection();
  }
}

void EcalsysGui::applyInitialLayout()
{
  ui_.central_widget->hide(); // We must always have a central widget. But we can hide it, because we only want to display stuff in DockWidgets
  tabifyDockWidget(ui_.tasks_dockwidget, ui_.groups_dockwidget);
  ui_.tasks_dockwidget->raise();
}

void EcalsysGui::updateMenuOptions()
{
  auto options = Globals::EcalSysInstance()->GetOptions();

  task_target_action_group_->blockSignals(true);
  ui_.action_options_kill_all_on_close->blockSignals(true);
  ui_.action_options_check_targets_reachable->blockSignals(true);


  ui_.action_options_check_targets_reachable->setChecked(options.check_target_reachability);
  ui_.action_options_kill_all_on_close->setChecked(options.kill_all_on_close);
  if (options.use_localhost_for_all_tasks)
  {
    ui_.action_options_use_localhost_for_all_tasks->setChecked(true);
  }
  else if (options.local_tasks_only)
  {
    ui_.action_options_local_tasks_only->setChecked(true);
  }
  else
  {
    ui_.action_options_use_configured_targets->setChecked(true);
  }

  task_target_action_group_->blockSignals(false);
  ui_.action_options_kill_all_on_close->blockSignals(false);
  ui_.action_options_check_targets_reachable->blockSignals(false);
}

void EcalsysGui::updateStartStopButtons()
{
  {
    // Task Menu
    auto selected_tasks = task_widget_->getSelectedTasks();
    if (selected_tasks.size() == 0)
    {
      // Deactivate if no Task is selected
      ui_.action_task_start_selected        ->setEnabled(false);
      ui_.action_task_stop_selected         ->setEnabled(false);
      ui_.action_task_restart_selected      ->setEnabled(false);
      ui_.menu_task_start_selected_on_host  ->setEnabled(false);
      ui_.menu_task_restart_selected_on_host->setEnabled(false);
      ui_.action_task_fast_kill_selected    ->setEnabled(false);
      ui_.action_task_fast_restart_selected ->setEnabled(false);
    }
    else {
      bool buttons_selected_active = true;
      for (auto& task : selected_tasks)
      {
        if (Globals::EcalSysInstance()->IsTaskActionRunning(task))
        {
          // Deactivate Buttons if any Task is already starting or stopping
          buttons_selected_active = false;
          break;
        }
      }
      ui_.action_task_start_selected        ->setEnabled(buttons_selected_active);
      ui_.action_task_stop_selected         ->setEnabled(buttons_selected_active);
      ui_.action_task_restart_selected      ->setEnabled(buttons_selected_active);
      ui_.menu_task_start_selected_on_host  ->setEnabled(buttons_selected_active);
      ui_.menu_task_restart_selected_on_host->setEnabled(buttons_selected_active);
      ui_.action_task_fast_kill_selected    ->setEnabled(buttons_selected_active);
      ui_.action_task_fast_restart_selected ->setEnabled(buttons_selected_active);
    }

    // Start/Stop/Restart All buttons
    auto all_tasks = Globals::EcalSysInstance()->GetTaskList();
    bool buttons_all_active = true;
    for (auto& task : all_tasks)
    {
      if (Globals::EcalSysInstance()->IsTaskActionRunning(task))
      {
        // Deactivate Buttons if any Task is already starting or stopping
        buttons_all_active = false;
        break;
      }
    }
    ui_.action_task_start            ->setEnabled(buttons_all_active);
    ui_.action_task_stop             ->setEnabled(buttons_all_active);
    ui_.action_task_restart          ->setEnabled(buttons_all_active);
    ui_.menu_task_start_all_on_host  ->setEnabled(buttons_all_active);
    ui_.menu_task_restart_all_on_host->setEnabled(buttons_all_active);
    ui_.action_task_fast_kill        ->setEnabled(buttons_all_active);
    ui_.action_task_fast_restart     ->setEnabled(buttons_all_active);
  }

  {
    {
      // Group Menu
      auto selected_tasks = group_widget_->getAllSelectedTasks();
      if (selected_tasks.size() == 0)
      {
        // Deactivate if no Task is selected
        ui_.action_group_start_selected->setEnabled(false);
        ui_.action_group_stop_selected->setEnabled(false);
        ui_.action_group_restart_selected->setEnabled(false);
        ui_.menu_group_start_selected_on_host->setEnabled(false);
        ui_.menu_group_restart_selected_on_host->setEnabled(false);
        ui_.action_group_fast_kill_selected->setEnabled(false);
        ui_.action_group_fast_restart_selected->setEnabled(false);
      }
      else {
        bool buttons_selected_active = true;
        for (auto& task : selected_tasks)
        {
          if (Globals::EcalSysInstance()->IsTaskActionRunning(task))
          {
            // Deactivate Buttons if any Task is already starting or stopping
            buttons_selected_active = false;
            break;
          }
        }
        ui_.action_group_start_selected->setEnabled(buttons_selected_active);
        ui_.action_group_stop_selected->setEnabled(buttons_selected_active);
        ui_.action_group_restart_selected->setEnabled(buttons_selected_active);
        ui_.menu_group_start_selected_on_host->setEnabled(buttons_selected_active);
        ui_.menu_group_restart_selected_on_host->setEnabled(buttons_selected_active);
        ui_.action_group_fast_kill_selected->setEnabled(buttons_selected_active);
        ui_.action_group_fast_restart_selected->setEnabled(buttons_selected_active);
      }
    }
  }
}

void EcalsysGui::updateTaskEditButtons()
{
  auto selected_tasks = task_widget_->getSelectedTasks();
  bool edit_buttons_enabled = (selected_tasks.size() > 0);
  
  ui_.action_task_edit     ->setEnabled(edit_buttons_enabled);
  ui_.action_task_duplicate->setEnabled(edit_buttons_enabled);
  ui_.action_task_remove   ->setEnabled(edit_buttons_enabled);
}

void EcalsysGui::updateGroupEditButtons()
{
  auto selected_groups = group_widget_->getSelectedGroups();

  ui_.action_group_edit     ->setEnabled((selected_groups.size() == 1));
  ui_.action_group_duplicate->setEnabled((selected_groups.size() > 0));
  ui_.action_group_remove   ->setEnabled((selected_groups.size() > 0));
}

void EcalsysGui::setWindowFilePathSlot(const QString& file_path)
{
  setWindowFilePath(file_path);
}

void EcalsysGui::monitorUpdateCallback()
{
  emit monitorUpdatedSignal();
}

void EcalsysGui::showImportFromCloudWindow()
{
  import_from_cloud_window_->setWindowState((import_from_cloud_window_->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
  import_from_cloud_window_->show();
  import_from_cloud_window_->raise();  // for MacOS
  import_from_cloud_window_->activateWindow(); // for Windows
}

bool EcalsysGui::askToSaveFile()
{
  bool may_continue = false;

  if (config_has_been_modified_)
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
      if (Globals::EcalSysInstance()->GetCurrentConfigPath() == "")
      {
        may_continue = menuFileSaveAsTriggered();
      }
      else
      {
        menuFileSaveTriggered();
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


////////////////////////////////////////////////////////////////////////////////
//// Menu Callbacks                                                         ////
////////////////////////////////////////////////////////////////////////////////

void EcalsysGui::menuFileNewTriggered()
{
  bool continue_resetting = true;
  if (config_has_been_modified_)
  {
    continue_resetting = askToSaveFile();
  }

  if (continue_resetting)
  {
    Globals::EcalSysInstance()->ClearConfig();
    setWindowFilePath("New Config");
    config_has_been_modified_ = false;
    setWindowModified(false);
    emit configChangedSignal("New Config");
  }
}

void EcalsysGui::menuFileOpenTriggered()
{
  bool continue_open = true;
  if (config_has_been_modified_)
  {
    continue_open = askToSaveFile();
  }

  if (continue_open)
  {
    QString start_path = Globals::EcalSysInstance()->GetCurrentConfigPath().c_str();

    if (start_path.isEmpty())
    {
      if (last_config_list.size() >= 1)
      {
        QFileInfo config_file_info(last_config_list.front());
        start_path = config_file_info.absoluteDir().absolutePath();
      }
    }

    QString config_path = QFileDialog::getOpenFileName(this, tr("Open eCAL Sys Config"), start_path, tr("Ecalsys files (*.ecalsys);;All Files (*)"));

    if (!config_path.isEmpty())
    {
      openFile(config_path, false);
    }
  }
}

void EcalsysGui::menuFileAppendTriggered()
{
  QString start_path  = Globals::EcalSysInstance()->GetCurrentConfigPath().c_str();

  if (start_path.isEmpty())
  {
    if (last_config_list.size() >= 1)
    {
      QFileInfo config_file_info(last_config_list.front());
      start_path = config_file_info.absoluteDir().absolutePath();
    }
  }

  QString config_path = QFileDialog::getOpenFileName(this, tr("Append eCAL Sys Config"), start_path, tr("Ecalsys files (*.ecalsys);;All Files (*)"));
  
  if (!config_path.isEmpty())
  {
    openFile(config_path, true);
  }
}


void EcalsysGui::menuFileSaveTriggered()
{
  // Clear the focus, so that the currently edited option will also be set and saved
  QWidget* current_focus = QApplication::focusWidget();
  if (current_focus)
    current_focus->clearFocus();

  std::string current_config_path = Globals::EcalSysInstance()->GetCurrentConfigPath();

  if (current_config_path == "")
  {
    menuFileSaveAsTriggered();
  }
  else
  {
    bool success = false;
    bool retry = false;

    do
    {
      retry = false;
      try
      {
        success = Globals::EcalSysInstance()->SaveConfig(current_config_path);
      }
      catch (const std::exception& e)
      {
        EcalSysLogger::Log(std::string(e.what()), spdlog::level::err);
        QMessageBox error_message(
          QMessageBox::Icon::Critical
          , "Error"
          , e.what()
          , QMessageBox::Button::Ok | QMessageBox::Button::Retry
          , this);
        int choice = error_message.exec();
        retry = (choice == QMessageBox::Button::Retry);
      }
    } while (retry);

    if (success)
    {
      config_has_been_modified_ = false;
      setWindowModified(false);
      updateRecentFiles(Globals::EcalSysInstance()->GetCurrentConfigPath().c_str());
    }
  }
}

bool EcalsysGui::menuFileSaveAsTriggered()
{
  std::string current_config_path = Globals::EcalSysInstance()->GetCurrentConfigPath();
  QString start_dir = (current_config_path == "" ? "New Config.ecalsys" : current_config_path.c_str());

  QString selected_file = QFileDialog::getSaveFileName(this, "Save as", start_dir, tr("Ecalsys files (*.ecalsys);;All Files (*)"));
  if (selected_file != "")
  {
    bool success = false;
    bool retry = false;

    do
    {
      retry = false;
      try
      {
        success = Globals::EcalSysInstance()->SaveConfig(selected_file.toStdString());
      }
      catch (const std::exception& e)
      {
        EcalSysLogger::Log(std::string(e.what()), spdlog::level::err);
        QMessageBox error_message(
          QMessageBox::Icon::Critical
          , "Error"
          , e.what()
          , QMessageBox::Button::Ok | QMessageBox::Button::Retry
          , this);
        int choice = error_message.exec();
        retry = (choice == QMessageBox::Button::Retry);
      }
    } while (retry);

    if (success)
    {
      setWindowFilePath(selected_file);
      config_has_been_modified_ = false;
      setWindowModified(false);
      updateRecentFiles(Globals::EcalSysInstance()->GetCurrentConfigPath().c_str());
    }
    return success;
  }
  return false;
}

void EcalsysGui::menuEditRunnersTriggered()
{
  runner_window_->setWindowState((runner_window_->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
  runner_window_->show();
  runner_window_->raise();  // for MacOS
  runner_window_->activateWindow(); // for Windows
}

void EcalsysGui::menuViewResetLayoutTriggered()
{
  setTheme(Theme::Default);

  // Back when we saved the initial window geometry, the window-manager might not have positioned the window on the screen, yet
  int screen_number = QApplication::desktop()->screenNumber(this);

  restoreGeometry(initial_geometry_);
  restoreState(initial_state_);

  move(QGuiApplication::screens().at(screen_number)->availableGeometry().center() - rect().center());

  task_widget_             ->resetLayout();
  runner_window_           ->resetLayout(QApplication::desktop()->screenNumber(this));
  group_widget_            ->resetLayout();
  group_edit_widget_       ->resetLayout();
  import_from_cloud_window_->resetLayout(QApplication::desktop()->screenNumber(this));
}

void EcalsysGui::menuOptionsCheckTargetsReachableToggled(bool enabled)
{
  auto options = Globals::EcalSysInstance()->GetOptions();
  options.check_target_reachability = enabled;
  Globals::EcalSysInstance()->SetOptions(options);
  emit ecalsysOptionsChangedSignal();
}

void EcalsysGui::menuOptionsTargetGroupTriggered(QAction* action)
{
  auto options = Globals::EcalSysInstance()->GetOptions();
  if (action == ui_.action_options_use_configured_targets)
  {
    options.use_localhost_for_all_tasks = false;
    options.local_tasks_only = false;
  }
  else if (action == ui_.action_options_local_tasks_only)
  {
    options.use_localhost_for_all_tasks = false;
    options.local_tasks_only = true;
  }
  else if (action == ui_.action_options_use_localhost_for_all_tasks)
  {
    options.use_localhost_for_all_tasks = true;
    options.local_tasks_only = false;
  }
  Globals::EcalSysInstance()->SetOptions(options);
  emit ecalsysOptionsChangedSignal();
}

void EcalsysGui::menuOptionsKillAllOnCloseToggled(bool enabled)
{
  auto options = Globals::EcalSysInstance()->GetOptions();
  options.kill_all_on_close = enabled;
  Globals::EcalSysInstance()->SetOptions(options);
  emit ecalsysOptionsChangedSignal();
}

void EcalsysGui::menuHelpDocumentationTriggered()
{
  QDesktopServices::openUrl(QUrl("http://ecal.io/"));
}

void EcalsysGui::menuHelpAboutTriggered()
{
  AboutDialog about_dialog(this);
  about_dialog.exec();
}

void EcalsysGui::menuHelpLicensesTriggered()
{
  LicenseDialog license_dialog(this);
  license_dialog.exec();
}

#ifdef WIN32
#pragma warning(push)
#pragma warning (disable : 4996)
void EcalsysGui::showConsole(bool show)
{
  if (show)
  {
    AllocConsole();
    if (!freopen("CONOUT$", "w", stdout))
    {
      std::cerr << "Could not open console stdout stream" << std::endl;
    }
    if (!freopen("CONOUT$", "w", stderr))
    {
      std::cerr << "Could not open console stderr stream" << std::endl;
    }
  }
  else
  {
    FreeConsole();
  }
}
#pragma warning(pop)
#endif // WIN32


void EcalsysGui::openFile(QString path, bool append)
{
  bool success = false;
  bool retry = false;

  do
  {
    retry = false;
    try {
      success = Globals::EcalSysInstance()->LoadConfig(path.toStdString(), append);
    }
    catch (const std::exception& e) {
      EcalSysLogger::Log(std::string(e.what()), spdlog::level::err);
      QMessageBox error_message(
        QMessageBox::Icon::Critical
        , "Error"
        , e.what()
        , QMessageBox::Button::Ok | QMessageBox::Button::Retry
        , this);
      int choice = error_message.exec();
      retry = (choice == QMessageBox::Button::Retry);
    }
  } while (retry);

  if (success) {
    if (append)
    {
      config_has_been_modified_ = true;
      setWindowModified(true);
      emit configChangedSignal(Globals::EcalSysInstance()->GetCurrentConfigPath() != "" ? Globals::EcalSysInstance()->GetCurrentConfigPath().c_str() : "New Config");
    }
    else
    {
      config_has_been_modified_ = false;
      setWindowModified(false);
      updateRecentFiles(Globals::EcalSysInstance()->GetCurrentConfigPath().c_str());
      emit configChangedSignal(Globals::EcalSysInstance()->GetCurrentConfigPath().c_str());
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//// Close Event                                                            ////
////////////////////////////////////////////////////////////////////////////////

void EcalsysGui::closeEvent(QCloseEvent *event)
{
  bool ok_to_close = true;
  if (config_has_been_modified_)
  {
    ok_to_close = askToSaveFile();
  }

  if (ok_to_close)
  {
    auto options = Globals::EcalSysInstance()->GetOptions();
    if (options.kill_all_on_close)
    {
      QProgressDialog dlg("Killing tasks", "Cancel", 0, 0, this);
      dlg.setLabelText("Please wait while eCAL Sys is killing all tasks. \nYou can disable this feature with \"Options/Kill all on close\".");
      dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowCloseButtonHint);
      dlg.setCancelButton(nullptr);
      dlg.setWindowModality(Qt::WindowModality::ApplicationModal);
      dlg.setMinimumDuration(500);
      dlg.setValue(0);
      dlg.setValue(1);

      QFuture<void> kill_all_future = QtConcurrent::run([]()
      {
        Globals::EcalSysInstance()->InterruptAllTaskActions();
        Globals::EcalSysInstance()->WaitForTaskActions();

        Globals::EcalSysInstance()->StopTaskList(Globals::EcalSysInstance()->GetTaskList(), false, true);
        Globals::EcalSysInstance()->WaitForTaskActions();
      });

      while (!kill_all_future.isFinished())
      {
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
      }
      dlg.close();
    }

    // Close all other windows that might be open. Otherwise, the application will not exit.
    import_from_cloud_window_->close();
    runner_window_->close();

    Globals::EcalSysInstance()->RemoveMonitorUpdateCallback();

    saveGuiSettings();
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

////////////////////////////////////////////////////////////////////////////////
//// Startup                                                                ////
////////////////////////////////////////////////////////////////////////////////

void EcalsysGui::performStartupChecks()
{
  // Check for other eCAL Sys instances
  auto hosts_running_ecalsys = Globals::EcalSysInstance()->GetHostsRunningEcalSys();

  decltype(hosts_running_ecalsys) filtered_hosts_running_ecalsys;
  for (auto& host : hosts_running_ecalsys)
  {
    if ((host.first != eCAL::Process::GetHostName()) && (host.second == eCAL::Process::GetProcessID()))
    {
      filtered_hosts_running_ecalsys.push_back(host);
    }
  }
  
  if (filtered_hosts_running_ecalsys.size() > 0)
  {
    std::stringstream ss;
    ss << "Other eCALSys instances have been detected on:" << std::endl << std::endl;
    for (auto& host : filtered_hosts_running_ecalsys)
    {
      ss << "\t" << host.first << " (PID: " << host.second << ")" << std::endl;
    }
    ss << std::endl;
    ss << "You should close other eCALSys instances in order to prevent unintended behaviour" << std::endl;

    QMessageBox warning_message_box(
      QMessageBox::Icon::Warning
      , "Multiple eCALSys instances detected"
      , ss.str().c_str()
      , QMessageBox::Button::Ok
      , this);
    warning_message_box.exec();
  }
}

void EcalsysGui::updateTaskModifiedWarning()
{
  bool any_task_modified = false;
  for (auto& task : Globals::EcalSysInstance()->GetTaskList())
  {
    if (task->IsConfigModifiedSinceStart())
    {
      any_task_modified = true;
      break;
    }
  }
  task_modified_statusbar_widget_->setVisible(any_task_modified);
}

void EcalsysGui::setConfigModified()
{
  setWindowModified(true);
  config_has_been_modified_ = true;
  updateTaskModifiedWarning();
}

////////////////////////////////////////////////////////////////////////////////
//// Drag & drop                                                            ////
////////////////////////////////////////////////////////////////////////////////
void EcalsysGui::dragEnterEvent(QDragEnterEvent* event)
{
  const QMimeData* mime_data = event->mimeData();

  if (mime_data->hasUrls())
  {
    QList<QUrl> url_list = mime_data->urls();

    if (url_list.size() == 1)
    {
      QFileInfo file_info(url_list.at(0).toLocalFile());
      if (file_info.exists() && file_info.isFile())
      {
        event->acceptProposedAction();
        return;
      }
    }
  }
  QWidget::dragEnterEvent(event);
}

void EcalsysGui::dragMoveEvent(QDragMoveEvent* event)
{
  QWidget::dragMoveEvent(event);
}

void EcalsysGui::dragLeaveEvent(QDragLeaveEvent* event)
{
  QWidget::dragLeaveEvent(event);
}

void EcalsysGui::dropEvent(QDropEvent* event)
{
  const QMimeData* mime_data = event->mimeData();

  if (mime_data->hasUrls())
  {
    QList<QUrl> url_list = mime_data->urls();

    if (url_list.size() == 1)
    {
      QFileInfo file_info(url_list.at(0).toLocalFile());
      if (file_info.exists())
      {
        event->acceptProposedAction();
        openFile(QDir::toNativeSeparators(file_info.absoluteFilePath()));
        return;
      }
    }
  }
  QWidget::dropEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
//// Auxiliary                                                              ////
////////////////////////////////////////////////////////////////////////////////

void EcalsysGui::updateHostActions(QMenu* menu)
{
  menu->clear();

  std::set<std::string> all_hosts = Globals::EcalSysInstance()->GetAllHosts();

  if (all_hosts.size() == 0)
  {
    QAction* dummy_action = new QAction(tr("No hosts found"), menu);
    dummy_action->setEnabled(false);
    menu->addAction(dummy_action);
  }
  else
  {
    for (auto& host : all_hosts)
    {
      QAction* host_action = new QAction(host.c_str(), menu);
      menu->addAction(host_action);
    }
  }
}

void EcalsysGui::updateRecentFiles(const QString& new_config_file)
{
#ifdef WIN32
  QString config_file = new_config_file;
  config_file.replace("/", "\\");

  // Remove duplicates
  QList<QString> new_last_config_list;
  for (const QString& last_config : last_config_list)
  {
    if (config_file.compare(last_config, Qt::CaseInsensitive) != 0)
    {
      new_last_config_list.push_back(last_config);
    }
  }
  last_config_list = new_last_config_list;

  // Add the config
  last_config_list.push_front(config_file);

#else
  // Remove duplicates
  last_config_list.removeAll(new_config_file);

  // Add the config
  last_config_list.push_front(new_config_file);
#endif // WIN32

  // trim the list to 10 elements
  while (last_config_list.size() > 10)
  {
    last_config_list.removeLast();
  }

  // update the menu
  updateRecentFiles();

  // save the recent file list, so that other instances that are started afterwards can immediatelly use them.
  QSettings settings;
  settings.beginGroup("mainwindow");
  settings.setValue("last_configs", last_config_list);
  settings.endGroup();
}

void EcalsysGui::updateRecentFiles()
{
  ui_.menu_recent_files->clear();
  
  if (last_config_list.size() == 0)
  {
    QAction* dummy_action = new QAction(tr("no files"), ui_.menu_recent_files);
    ui_.menu_recent_files->addAction(dummy_action);
    dummy_action->setEnabled(false);
  }
  else
  {
    int counter = 1;
    for (auto i = last_config_list.begin(); i != last_config_list.end(); ++i)
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

      QFileInfo config_file_info(*i);
      QString config_file_name = config_file_info.fileName();
      QAction* open_file_action = new QAction(label + ": " + config_file_name, ui_.menu_recent_files);

      connect(open_file_action, &QAction::triggered, 
        [this, path = *i]()
        {
          bool continue_open = true;
          if (config_has_been_modified_)
          {
            continue_open = askToSaveFile();
          }
          if (continue_open)
          {
            openFile(path, false);
          }
        }
      );

      open_file_action->setToolTip(*i);

      ui_.menu_recent_files->addAction(open_file_action);

      counter++;
    }
  }

  QAction* clear_recent_files_action = new QAction(tr("Clear list"), ui_.menu_recent_files);
  connect(clear_recent_files_action, &QAction::triggered, [this]() {clearRecentFiles(); });
  clear_recent_files_action->setEnabled(last_config_list.size() > 0);
  ui_.menu_recent_files->addSeparator();
  ui_.menu_recent_files->addAction(clear_recent_files_action);
}

void EcalsysGui::clearRecentFiles()
{
  last_config_list.clear();
  updateRecentFiles();

  // save the recent file list, so that other instances that are started afterwards can immediatelly use them.
  QSettings settings;
  settings.beginGroup("mainwindow");
  settings.setValue("last_configs", last_config_list);
  settings.endGroup();
}
