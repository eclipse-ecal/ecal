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

#include "task_widget.h"

#include <chrono>
#include <functional>
#include <memory>
#include <map>
#include <cmath>

#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QMenu>
#include <QPoint>
#include <QTimer>
#include <QSettings>

#include <EcalParser/EcalParser.h>

#include "globals.h"

#include "widgets/targets_reachable_dialog/targets_reachable_dialog.h"
#include "widgets/import_from_cloud_widget/import_from_cloud_widget.h"

#include "widgets/treemodels/task_tree_model.h"
#include "widgets/treemodels/task_tree_item.h"
#include "widgets/treemodels/tree_data_roles.h"
#include "CustomQt/QStableSortFilterProxyModel.h"

#include <sys_client_core/task.h>
#include <ecalsys/task/ecal_sys_task_helper.h>

TaskWidget::TaskWidget(QWidget *parent)
  : QFrame(parent)
{
  ui_.setupUi(this);

  // Set custom Checkbox settings
  ui_.enable_monitoring_checkbox      ->setStateAfterPartiallyChecked(Qt::CheckState::Checked);
  ui_.restart_by_severity_checkbox    ->setStateAfterPartiallyChecked(Qt::CheckState::Unchecked);

  // Setup the target completer
  target_completer_       = new QCompleter(this);
  target_completer_model_ = new QStringListModel(target_completer_);
  target_completer_       ->setModel(target_completer_model_);
  target_completer_       ->setCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  target_completer_       ->setCompletionMode(QCompleter::CompletionMode::PopupCompletion);

  updateTargetCompleter();
  connect(target_completer_, static_cast<void(QCompleter::*)(const QString &)>(&QCompleter::activated),
    [=]() {targetEditingFinished(); });
  ui_.target_lineedit->setCompleter(target_completer_);

  // Model For the task-tree
  task_tree_model_ = new TaskTreeModel(this);
  task_tree_model_->reload(Globals::EcalSysInstance()->GetTaskList());
  task_tree_sort_filter_proxy_model_ = new QStableSortFilterProxyModel(this);
  task_tree_sort_filter_proxy_model_->setSourceModel(task_tree_model_);
  task_tree_sort_filter_proxy_model_->setSortRole(ItemDataRoles::SortRole);
  task_tree_sort_filter_proxy_model_->setFilterKeyColumn((int)TaskTreeModel::Columns::TASK_NAME);
  task_tree_sort_filter_proxy_model_->setDynamicSortFilter(false);
  task_tree_sort_filter_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  ui_.task_tree->setModel(task_tree_sort_filter_proxy_model_);
  ui_.task_tree->sortByColumn((int)TaskTreeModel::Columns::TASK_NAME, Qt::SortOrder::AscendingOrder);

  // initialize column width
  autoSizeColumns();

  // only show some colums in the task-tree
  static std::vector<TaskTreeModel::Columns> intial_colum_selection =
  {
    TaskTreeModel::Columns::LAUNCH_ORDER,
    TaskTreeModel::Columns::TASK_NAME,
    TaskTreeModel::Columns::TARGET_NAME,
    TaskTreeModel::Columns::CURRENT_PID,
    TaskTreeModel::Columns::CURRENT_HOST,
    TaskTreeModel::Columns::STATE,
    TaskTreeModel::Columns::INFO
  };
  for (int col = 0; col < (int)TaskTreeModel::Columns::COLUMN_COUNT; col++)
  {
    if (std::find(intial_colum_selection.begin(), intial_colum_selection.end(), (TaskTreeModel::Columns)col) == intial_colum_selection.end())
    {
      ui_.task_tree->hideColumn(col);
    }
  }

  // Disable hiding the name column
  ui_.task_tree->setForcedColumns(QVector<int>{(int)TaskTreeModel::Columns::TASK_NAME});

  //  Model for Severity / -Level Comboboxes
  severity_model_                       = new SeverityModel(this);
  severity_level_model_                 = new SeverityLevelModel(this);
  ui_.restart_by_severity_level_combobox ->setModel(severity_level_model_);
  ui_.restart_by_severity_combobox       ->setModel(severity_model_);

  runner_tree_model_ = new RunnerTreeModel(this);
  runner_tree_model_->reload(Globals::EcalSysInstance()->GetRunnerList(), true);
  ui_.runner_combobox->setModel(runner_tree_model_);
  ui_.runner_combobox->setModelColumn((int)RunnerTreeModel::Columns::NAME);

  ui_.filter_lineedit->setClearIcon(QIcon(":/ecalicons/FILTER_CANCEL"));
  task_tree_sort_filter_proxy_model_->setFilterRole(ItemDataRoles::FilterRole);

  // Task Table
  connect(ui_.task_tree->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this,
    SLOT(taskTableSelectionChanged(/*const QItemSelection &, const QItemSelection &*/)));
  connect(ui_.task_tree->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this,
    SIGNAL(selectionChangedSignal()));
  connect(ui_.task_tree->selectionModel(),
    SIGNAL(currentColumnChanged(const QModelIndex &, const QModelIndex &)),
    this,
    SLOT(taskTableCurrentChanged(const QModelIndex &, const QModelIndex &)));
  connect(ui_.task_tree,           SIGNAL(doubleClicked(QModelIndex)),                this, SLOT(setEditControlsVisibility(/*true*/)));
  connect(this,                    SIGNAL(monitorUpdatedSignal()),                    this, SLOT(updateTaskTableMonitorInformation()));
  connect(ui_.filter_lineedit,     SIGNAL(textChanged(QString)),                      this, SLOT(filterTextChanged(QString)));

  connect(ui_.task_tree,           SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(taskTableContextMenu(const QPoint&)));

  connect(ui_.task_tree,           SIGNAL(keySequenceDeletePressed()),                        this, SLOT(removeSelectedTasks()));

  // Button bar
  connect(ui_.add_task_button,                        SIGNAL(clicked()),                   this, SLOT(addTask()));
  connect(ui_.edit_task_button,                       SIGNAL(clicked()),                   this, SLOT(setEditControlsVisibility(/*true*/)));
  connect(ui_.remove_task_button,                     SIGNAL(clicked()),                   this, SLOT(removeSelectedTasks()));
  connect(ui_.start_selected_tasks_button,            SIGNAL(clicked()),                   this, SLOT(startSelectedTasks()));
  connect(ui_.stop_selected_tasks_button,             SIGNAL(clicked()),                   this, SLOT(stopSelectedTasks()));
  connect(ui_.restart_selected_tasks_button,          SIGNAL(clicked()),                   this, SLOT(restartSelectedTasks()));
  connect(ui_.start_all_tasks_button,                 SIGNAL(clicked()),                   this, SLOT(startAllTasks()));
  connect(ui_.stop_all_tasks_button,                  SIGNAL(clicked()),                   this, SLOT(stopAllTasks()));
  connect(ui_.restart_all_tasks_button,               SIGNAL(clicked()),                   this, SLOT(restartAllTasks()));
  connect(ui_.import_from_cloud_button,               SIGNAL(clicked()),                   this, SIGNAL(importFromCloudButtonClickedSignal()));
  connect(ui_.update_form_cloud_button,               SIGNAL(clicked()),                   this, SLOT(updateFromCloud()));

  // "Hidden" stop / restart menus
  ui_.stop_selected_tasks_button->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.stop_selected_tasks_button, &QPushButton::customContextMenuRequested,
    [this](const QPoint &pos)
    {
      QMenu context_menu(tr("Context menu"), this);
      QAction fast_stop_action(tr("Fast-kill selected"), this);
      context_menu.addAction(&fast_stop_action);
      connect(&fast_stop_action, &QAction::triggered, [this]() {stopSelectedTasks(false, true); });
      context_menu.exec(ui_.stop_selected_tasks_button->mapToGlobal(pos));
    }
  );
  ui_.restart_selected_tasks_button->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.restart_selected_tasks_button, &QPushButton::customContextMenuRequested,
    [this](const QPoint &pos)
    {
      QMenu context_menu(tr("Context menu"), this);
      QAction fast_restart_option(tr("Fast-kill && restart selected"), this);
      context_menu.addAction(&fast_restart_option);
      connect(&fast_restart_option, &QAction::triggered, [this]() {restartSelectedTasks(false, true); });
      context_menu.exec(ui_.restart_selected_tasks_button->mapToGlobal(pos));
    }
  );
  ui_.stop_all_tasks_button->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.stop_all_tasks_button, &QPushButton::customContextMenuRequested,
    [this](const QPoint &pos)
    {
      QMenu context_menu(tr("Context menu"), this);
      QAction fast_stop_action(tr("Fast-kill"), this);
      context_menu.addAction(&fast_stop_action);
      connect(&fast_stop_action, &QAction::triggered, [this]() {stopAllTasks(false, true); });
      context_menu.exec(ui_.stop_all_tasks_button->mapToGlobal(pos));
    }
  );
  ui_.restart_all_tasks_button->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.restart_all_tasks_button, &QPushButton::customContextMenuRequested,
    [this](const QPoint &pos)
    {
      QMenu context_menu(tr("Context menu"), this);
      QAction fast_restart_option(tr("Fast-kill && restart"), this);
      context_menu.addAction(&fast_restart_option);
      connect(&fast_restart_option, &QAction::triggered, [this]() {restartAllTasks(false, true); });
      context_menu.exec(ui_.restart_all_tasks_button->mapToGlobal(pos));
    }
  );

  // Edit controls
  connect(ui_.hide_edit_control_button,               SIGNAL(clicked()),                   this, SLOT(toggleEditControlsVisibility()));
  connect(ui_.task_name_lineedit,                     SIGNAL(editingFinished()),           this, SLOT(nameEditingFinished()));
  connect(ui_.target_lineedit,                        SIGNAL(editingFinished()),           this, SLOT(targetEditingFinished()));
  connect(ui_.runner_combobox,                        SIGNAL(currentIndexChanged(int)),    this, SLOT(runnerIndexChanged(int)));
  connect(ui_.show_runners_button,                    SIGNAL(clicked()),                   this, SLOT(showRunnersButtonClicked()));
  connect(ui_.algo_path_lineedit,                     SIGNAL(editingFinished()),           this, SLOT(algoPathEditingFinished()));
  connect(ui_.algo_path_button,                       SIGNAL(clicked()),                   this, SLOT(algoPathChooseButtonClicked()));
  connect(ui_.working_dir_lineedit,                   SIGNAL(editingFinished()),           this, SLOT(workingDirEditingFinished()));
  connect(ui_.working_dir_button,                     SIGNAL(clicked()),                   this, SLOT(workingDirChooseButtonClicked()));
  connect(ui_.cmd_args_lineedit,                      SIGNAL(editingFinished()),           this, SLOT(cmdArgsEditingFinished()));
  connect(ui_.visibility_combobox,                    SIGNAL(currentTextChanged(QString)), this, SLOT(visibilityTextChanged()));
  connect(ui_.launch_order_spinbox,                   SIGNAL(valueChanged(int)),           this, SLOT(launchOrderValueChanged()));
  connect(ui_.waiting_time_spinbox,                   SIGNAL(valueChanged(int)),           this, SLOT(waitingTimeValueChanged()));
  connect(ui_.enable_monitoring_checkbox,             SIGNAL(stateChanged(int)),           this, SLOT(enableMonitoringStateChanged(int)));
  connect(ui_.restart_by_severity_checkbox,           SIGNAL(stateChanged(int)),           this, SLOT(restartBySeverityEnabledStateChanged(int)));
  connect(ui_.restart_by_severity_combobox,           SIGNAL(currentIndexChanged(int)),    this, SLOT(restartBySeverityIndexChanged(int)));
  connect(ui_.restart_by_severity_level_combobox,     SIGNAL(currentIndexChanged(int)),    this, SLOT(restartBySeverityLevelIndexChanged(int)));

  // Warning signs in the edit controls
  int label_height = ui_.restart_by_severity_checkbox->sizeHint().height();
  QPixmap warning_icon = Globals::Icons::warning().scaled(label_height, label_height, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);

  ui_.restart_by_severity_warning_label->setPixmap(warning_icon);

  // Update buttons
  connect(this, SIGNAL(startStopActionTriggeredSignal()), this, SLOT(updateStartStopButtons()));

  // Load initial config 
  configChanged();

  // Hide the edit controls
  setEditControlsVisibility(false);

  // Save the tree state for resetLayout()
  initial_task_tree_state_ = ui_.task_tree->saveState();

  QSettings settings;
  settings.beginGroup("taskwidget");
  ui_.task_tree->restoreState(settings.value("task_tree_state").toByteArray(), Globals::ecalSysVersion());
  settings.endGroup();
}

TaskWidget::~TaskWidget()
{
  QSettings settings;
  settings.beginGroup("taskwidget");
  settings.setValue("task_tree_state", ui_.task_tree->saveState(Globals::ecalSysVersion()));
  settings.endGroup();
}

void TaskWidget::resetLayout()
{
  ui_.task_tree->restoreState(initial_task_tree_state_);
}

void TaskWidget::autoSizeColumns()
{
  // We don't actually adapt to content here, as we don't have any, yet. We
  // instead create a reference example task and resize the tree to fit it. If
  // the user is not satisfied, he can resize the columns manually, anyway.
  std::shared_ptr<EcalSysTask> example_task(new EcalSysTask());
  example_task->SetId                          (999999);
  example_task->SetLaunchOrder                 (999);
  example_task->SetName                        ("CameraSensorMapFusionCAF___");
  example_task->SetTarget                      ("CARPC00___");
  std::shared_ptr<EcalSysRunner> example_runner(new EcalSysRunner());
  example_runner->SetName                      ("RunTask (new)___");
  example_task->SetRunner                      (example_runner);
  example_task->SetVisibility                  (eCAL_Process_eStartMode::proc_smode_maximized);
  example_task->SetTimeoutAfterStart           (std::chrono::milliseconds(99999));
  example_task->SetMonitoringEnabled           (false);
  example_task->SetRestartBySeverityEnabled    (false);
  example_task->SetRestartAtSeverity           (TaskState(eCAL_Process_eSeverity::proc_sev_warning, eCAL_Process_eSeverity_Level::proc_sev_level5));
  example_task->SetPids                        (std::vector<int>{9999999});
  example_task->SetHostStartedOn               ("CARPC00___");
  example_task->SetMonitoringTaskState         (TaskState(eCAL_Process_eSeverity::proc_sev_warning, eCAL_Process_eSeverity_Level::proc_sev_level5));

  // We don't want to resize all Columns, as e.g. for the Algo Path we really can't know how much text will be in there.
  static const std::vector<TaskTreeModel::Columns> columns_to_resize =
  {
    TaskTreeModel::Columns::ID,
    TaskTreeModel::Columns::LAUNCH_ORDER,
    TaskTreeModel::Columns::TASK_NAME,
    TaskTreeModel::Columns::TARGET_NAME,
    TaskTreeModel::Columns::RUNNER_NAME,
    TaskTreeModel::Columns::VISIBILITY,
    TaskTreeModel::Columns::TIMEOUT_AFTER_START,
    TaskTreeModel::Columns::MONITORING_ENABLED,
    TaskTreeModel::Columns::RESTART_BY_SEVERITY_ENABLED,
    TaskTreeModel::Columns::RESTART_AT_SEVERITY,
    TaskTreeModel::Columns::CURRENT_PID,
    TaskTreeModel::Columns::CURRENT_HOST,
    TaskTreeModel::Columns::STATE,
  };

  // Add the example task, resize the columns and quickly remove the task, again.
  task_tree_model_->addTask(example_task);
  for (TaskTreeModel::Columns column : columns_to_resize)
  {
    ui_.task_tree->resizeColumnToContents((int)column);
  }
  task_tree_model_->removeTask(example_task);
}

void TaskWidget::toggleEditControlsVisibility()
{
  setEditControlsVisibility(!ui_.edit_task_widget->isVisible());
}

void TaskWidget::setEditControlsVisibility(bool visibile)
{
  ui_.edit_task_widget->setVisible(visibile);

  if (visibile)
  {
    //                                                   ▼ Hide ▼
    ui_.hide_edit_control_button->setText(tr("\342\226\274 Hide \342\226\274"));
  }
  else
  {
    //                                                   ▲ Edit ▲
    ui_.hide_edit_control_button->setText(tr("\342\226\262 Edit \342\226\262"));
  }
}

void TaskWidget::filterTextChanged(QString text)
{
  task_tree_sort_filter_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  task_tree_sort_filter_proxy_model_->setFilterRegExp(text);
}

////////////////////////////////////////////////////////////////////////////////
//// Management of the task list                                            ////
////////////////////////////////////////////////////////////////////////////////

QList<QModelIndex> TaskWidget::getSelectedIndices()
{
  auto selected_proxy_indices = ui_.task_tree->selectionModel()->selectedRows();

  QList<QModelIndex> selected_indices;
  for (auto& proxy_index : selected_proxy_indices)
  {
    selected_indices.push_back(task_tree_sort_filter_proxy_model_->mapToSource(proxy_index));
  }

  return selected_indices;
}

std::vector<std::shared_ptr<EcalSysTask>> TaskWidget::getSelectedTasks()
{
  auto selected_proxy_rows = ui_.task_tree->selectionModel()->selectedRows();

  std::vector<std::shared_ptr<EcalSysTask>> selected_tasks;
  selected_tasks.reserve(selected_proxy_rows.size());

  for (auto& s : selected_proxy_rows)
  {
    TaskTreeItem* task_tree_item = static_cast<TaskTreeItem*>(task_tree_sort_filter_proxy_model_->mapToSource(s).internalPointer());
    selected_tasks.push_back(task_tree_item->getTask());
  }
  return selected_tasks;
}

void TaskWidget::taskTableSelectionChanged(/*const QItemSelection & selected, const QItemSelection & deselected*/)
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() >= 1)
  {
    activateEditArea(selected_tasks);
    ui_.edit_task_button->setEnabled(true);
  }
  else
  {
    deactivateEditArea();
    ui_.edit_task_button->setEnabled(false);
  }

  if (selected_tasks.size() == 0)
  {
    ui_.remove_task_button->setEnabled(false);
  }
  else
  {
    ui_.remove_task_button->setEnabled(true);
  }
  updateStartStopButtons();
}

void TaskWidget::taskTableCurrentChanged(const QModelIndex & current, const QModelIndex & /*previous*/)
{
  // This is necessary for "searching" for tasks with the keyboard. The index must always stay in the name column.
  if ((TaskTreeModel::Columns)(current.column()) != TaskTreeModel::Columns::TASK_NAME)
  {
    QModelIndex new_current_index = task_tree_sort_filter_proxy_model_->index(current.row(), (int)TaskTreeModel::Columns::TASK_NAME);
    ui_.task_tree->selectionModel()->setCurrentIndex(new_current_index, QItemSelectionModel::NoUpdate);
  }
}

void TaskWidget::deactivateEditArea()
{
  ui_.edit_task_widget->setEnabled(false);

  std::vector<std::shared_ptr<EcalSysTask>> task_list;
  updateEditAreaName                       (task_list);
  updateEditAreaTarget                     (task_list);
  updateEditAreaRunner                     (task_list);
  updateEditAreaAlgoPath                   (task_list);
  updateEditAreaWorkingDir                 (task_list);
  updateEditAreaCommandLine                (task_list);
  updateEditAreaVisibility                 (task_list);
  updateEditAreaLaunchOrder                (task_list);
  updateEditAreaWaitingTime                (task_list);
  updateEditAreaMonitoringEnabled          (task_list);
  updateEditAreaRestartBySeverityEnabled   (task_list);
  updateEditAreaRestartAtSeverity          (task_list);
  updateEditAreaRestartAtSeverityLevel     (task_list);

  updateCommandLineWorkingDirPreviews();
}

void TaskWidget::activateEditArea(std::vector<std::shared_ptr<EcalSysTask>> task_list)
{
  if (task_list.size() <= 0)
    return;

  updateEditAreaName                       (task_list);
  updateEditAreaTarget                     (task_list);
  updateEditAreaRunner                     (task_list);
  updateEditAreaAlgoPath                   (task_list);
  updateEditAreaWorkingDir                 (task_list);
  updateEditAreaCommandLine                (task_list);
  updateEditAreaVisibility                 (task_list);
  updateEditAreaLaunchOrder                (task_list);
  updateEditAreaWaitingTime                (task_list);
  updateEditAreaMonitoringEnabled          (task_list);
  updateEditAreaRestartBySeverityEnabled   (task_list);
  updateEditAreaRestartAtSeverity          (task_list);
  updateEditAreaRestartAtSeverityLevel     (task_list);

  updateCommandLineWorkingDirPreviews();

  // Enable Everything
  ui_.edit_task_widget->setEnabled(true);

  // Disable some items, again
  updateMonitoringControlsEnabledState();
}

void TaskWidget::focusTaskNameLineedit(bool select_text)
{
  ui_.task_name_lineedit->setFocus();
  if (select_text)
  {
    ui_.task_name_lineedit->selectAll();
  }
}

void TaskWidget::updateTargetCompleter()
{
  std::set<std::string> target_set;
  for (auto& task : Globals::EcalSysInstance()->GetTaskList())
  {
    std::string target = task->GetTarget();
    if (!target.empty())
    {
      target_set.emplace(target);
    }
  }
  QStringList target_list;
  for (auto& target : target_set)
  {
    target_list.push_back(target.c_str());
  }
  target_list.sort(Qt::CaseSensitivity::CaseInsensitive);
  target_completer_model_->setStringList(target_list);
}

void TaskWidget::updateCommandLineWorkingDirPreviews()
{
  auto selected_tasks = getSelectedTasks();

  if (selected_tasks.size() == 0)
  {
    ui_.cmd_args_lineedit->setToolTip(tr("Command line arguments"));

    ui_.command_line_preview_label->setText   ("");
    ui_.command_line_preview_label->setToolTip("Command line preview");

    ui_.working_dir_preview_label->setText    ("");
    ui_.working_dir_preview_label->setToolTip ("Working dir preview");
  }
  else
  {
    QString command_line;
    if (commonRunner(selected_tasks)
      && commonAlgoPath(selected_tasks)
      && commonWorkingDir(selected_tasks)
      && commonCommandLine(selected_tasks))
    {
      eCAL::sys_client::Task temp_task = eCAL::sys::task_helpers::ToSysClientTask_NoLock(selected_tasks[0]);
      eCAL::sys_client::EvaluateEcalParserFunctions(temp_task, true, std::chrono::system_clock::now());
      eCAL::sys_client::MergeRunnerIntoTask(temp_task, EcalUtils::Filesystem::OsStyle::Current);

      command_line = QString::fromStdString(temp_task.path);
      if (!temp_task.arguments.empty())
      {
        command_line += " ";
        command_line += QString::fromStdString(temp_task.arguments);
      }

      ui_.command_line_preview_label->setText   (command_line);
      ui_.command_line_preview_label->setToolTip(command_line);

      ui_.working_dir_preview_label->setText    (QString::fromStdString(temp_task.working_dir));
      ui_.working_dir_preview_label->setToolTip (QString::fromStdString(temp_task.working_dir));
    }
    else
    {
      command_line = tr(">> Multiple Values <<");

      ui_.command_line_preview_label->setText   (">> Multiple Values <<");
      ui_.command_line_preview_label->setToolTip(">> Multiple Values <<");

      ui_.working_dir_preview_label->setText    (">> Multiple Values <<");
      ui_.working_dir_preview_label->setToolTip (">> Multiple Values <<");
    }

    ui_.cmd_args_lineedit->setToolTip(tr("Resulting command line:\n") + command_line);
  }
}

void TaskWidget::updateMonitoringControlsEnabledState()
{
  if (!ui_.enable_monitoring_checkbox->isChecked())
  {
    ui_.restart_by_severity_checkbox            ->setEnabled(false);
    ui_.restart_by_severity_label               ->setEnabled(false);
    ui_.restart_by_severity_combobox            ->setEnabled(false);
    ui_.restart_by_severity_level_combobox      ->setEnabled(false);
  }
  else
  {
    ui_.restart_by_severity_checkbox            ->setEnabled(true);

    if (!ui_.restart_by_severity_checkbox->isChecked())
    {
      ui_.restart_by_severity_label             ->setEnabled(false);
      ui_.restart_by_severity_combobox          ->setEnabled(false);
      ui_.restart_by_severity_level_combobox    ->setEnabled(false);
    }
    else
    {
      ui_.restart_by_severity_label             ->setEnabled(true);
      ui_.restart_by_severity_combobox          ->setEnabled(true);
      ui_.restart_by_severity_level_combobox    ->setEnabled(true);
    }
  }
}

void TaskWidget::updateTaskTableMonitorInformation()
{
  task_tree_model_->updateAll();
}

void TaskWidget::taskTableContextMenu(const QPoint &pos)
{
  QMenu context_menu(tr("Context menu"), this);

  QAction start_action         (tr("Start"),           this);
  QAction stop_action          (tr("Stop"),            this);
  QAction restart_action       (tr("Restart"),         this);
  QMenu   start_on_host_menu   (tr("Start on host"),   this);
  QMenu   restart_on_host_menu (tr("Restart on host"), this);
  QAction fast_kill_action     (tr("Fast-kill"),       this);
  QAction fast_restart_action  (tr("Fast-kill && restart"), this);
  QAction add_action           (tr("Add Task"),             this);
  QAction edit_action          (tr("Edit"),            this);
  QAction duplicate_action     (tr("Duplicate"),       this);
  QAction remove_action        (tr("Remove"),          this);

  start_action    .setIcon(Globals::Icons::startSelected());
  stop_action     .setIcon(Globals::Icons::stopSelected());
  restart_action  .setIcon(Globals::Icons::restartSelected());
  add_action      .setIcon(Globals::Icons::add());
  edit_action     .setIcon(Globals::Icons::edit());
  duplicate_action.setIcon(Globals::Icons::duplicate());
  remove_action   .setIcon(Globals::Icons::remove());

  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() == 0)
  {
    // Deactivate everything, if no task is selected
    start_action        .setEnabled(false);
    stop_action         .setEnabled(false);
    restart_action      .setEnabled(false);
    fast_kill_action    .setEnabled(false);
    fast_restart_action .setEnabled(false);
    start_on_host_menu  .setEnabled(false);
    restart_on_host_menu.setEnabled(false);
    edit_action         .setEnabled(false);
    duplicate_action    .setEnabled(false);
    remove_action       .setEnabled(false);
  }
  else {
    for (auto& task : selected_tasks)
    {
      if (Globals::EcalSysInstance()->IsTaskActionRunning(task))
      {
        // Deactivate Start/Stop/Restart actions, if any task is already starting/stopping/restarting
        start_action        .setEnabled(false);
        stop_action         .setEnabled(false);
        restart_action      .setEnabled(false);
        fast_kill_action    .setEnabled(false);
        fast_restart_action .setEnabled(false);
        start_on_host_menu  .setEnabled(false);
        restart_on_host_menu.setEnabled(false);
        break;
      }
    }
  }

  // Fill the start_on_host_menu and restart_to_host_menu
  std::set<std::string> all_hosts = Globals::EcalSysInstance()->GetAllHosts();
  if (all_hosts.size() == 0)
  {
    QAction* dummy_action1 = new QAction(tr("No hosts found"), &context_menu);
    QAction* dummy_action2 = new QAction(tr("No hosts found"), &context_menu);
    dummy_action1->setEnabled(false);
    dummy_action2->setEnabled(false);
    start_on_host_menu  .addAction(dummy_action1);
    restart_on_host_menu.addAction(dummy_action2);
  }
  else {
    for (std::string host : all_hosts)
    {
      QAction* start_on_host_action   = new QAction(host.c_str(), &context_menu);
      QAction* restart_to_host_action = new QAction(host.c_str(), &context_menu);
      start_on_host_menu  .addAction(start_on_host_action);
      restart_on_host_menu.addAction(restart_to_host_action);
    }
  }

  context_menu.addAction   (&start_action);
  context_menu.addAction   (&stop_action);
  context_menu.addAction   (&restart_action);
  context_menu.addSeparator();
  context_menu.addMenu     (&start_on_host_menu);
  context_menu.addMenu     (&restart_on_host_menu);
  context_menu.addSeparator();
  context_menu.addAction   (&fast_kill_action);
  context_menu.addAction   (&fast_restart_action);
  context_menu.addSeparator();
  context_menu.addAction   (&add_action);
  context_menu.addAction   (&edit_action);
  context_menu.addAction   (&duplicate_action);
  context_menu.addAction   (&remove_action);

  connect(&start_action,         SIGNAL(triggered()),         this, SLOT(startSelectedTasks()));
  connect(&stop_action,          SIGNAL(triggered()),         this, SLOT(stopSelectedTasks()));
  connect(&restart_action,       SIGNAL(triggered()),         this, SLOT(restartSelectedTasks()));
  connect(&start_on_host_menu,   &QMenu::triggered,                 [this](QAction* action){startSelectedTasks(action->text().toStdString());});
  connect(&restart_on_host_menu, &QMenu::triggered,                 [this](QAction* action){restartSelectedTasks(true, true, action->text().toStdString());});
  connect(&add_action,           SIGNAL(triggered()),         this, SLOT(addTask()));
  connect(&edit_action,          SIGNAL(triggered()),         this, SLOT(setEditControlsVisibility(/*true*/)));
  connect(&duplicate_action,     SIGNAL(triggered()),         this, SLOT(duplicateSelectedTasks()));
  connect(&remove_action,        SIGNAL(triggered()),         this, SLOT(removeSelectedTasks()));

  connect(&fast_kill_action,    &QAction::triggered, [this]() {stopSelectedTasks(false, true); });
  connect(&fast_restart_action, &QAction::triggered, [this]() {restartSelectedTasks(false, true); });

  context_menu.exec(ui_.task_tree->viewport()->mapToGlobal(pos));
}

void TaskWidget::updateStartStopButtons()
{
  // Selected Start/stop buttons
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() == 0)
  {
    // Deactivate Buttons if no Task is selected
    ui_.start_selected_tasks_button  ->setEnabled(false);
    ui_.stop_selected_tasks_button   ->setEnabled(false);
    ui_.restart_selected_tasks_button->setEnabled(false);
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
    ui_.start_selected_tasks_button  ->setEnabled(buttons_selected_active);
    ui_.stop_selected_tasks_button   ->setEnabled(buttons_selected_active);
    ui_.restart_selected_tasks_button->setEnabled(buttons_selected_active);
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
  ui_.start_all_tasks_button  ->setEnabled(buttons_all_active);
  ui_.stop_all_tasks_button   ->setEnabled(buttons_all_active);
  ui_.restart_all_tasks_button->setEnabled(buttons_all_active);
}

void TaskWidget::setEditTask(std::shared_ptr<EcalSysTask> task)
{
  QModelIndex proxy_index = task_tree_sort_filter_proxy_model_->mapFromSource(task_tree_model_->index(task, (int)TaskTreeModel::Columns::TASK_NAME));
  if (proxy_index.isValid())
  {
    ui_.task_tree->selectionModel()->select(proxy_index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
    ui_.task_tree->scrollTo(proxy_index);
  }
  setEditControlsVisibility(true);
  focusTaskNameLineedit(true);
}

////////////////////////////////////////////////////////////////////////////////
//// Callbacks for Button Row                                               ////
////////////////////////////////////////////////////////////////////////////////

void TaskWidget::addTask()
{
  // Crate a new task
  std::shared_ptr<EcalSysTask> new_task(new EcalSysTask());
  new_task->SetTarget(eCAL::Process::GetHostName());

  TaskState min_acceptable_severity;
  min_acceptable_severity.severity       = eCAL_Process_eSeverity::proc_sev_failed;
  min_acceptable_severity.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level1;
  new_task->SetRestartAtSeverity(min_acceptable_severity);

  // Add Task to the actual model
  Globals::EcalSysInstance()->AddTask(new_task);

  // clear the filter
  ui_.filter_lineedit->setText("");

  // Add Task to the treemodel
  task_tree_model_->addTask(new_task);

  // Select the task and scroll to it
  QModelIndex added_source_index = task_tree_model_->index(new_task, (int)TaskTreeModel::Columns::TASK_NAME);
  QModelIndex added_proxy_index = task_tree_sort_filter_proxy_model_->mapFromSource(added_source_index);
  ui_.task_tree->selectionModel()->select(added_proxy_index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
  ui_.task_tree->scrollTo(added_proxy_index);

  // Open the edit area
  setEditControlsVisibility(true);
  focusTaskNameLineedit();

  // update the completer
  updateTargetCompleter();

  emit tasksAddedSignal(std::vector<std::shared_ptr<EcalSysTask>>{new_task});
}

void TaskWidget::duplicateSelectedTasks()
{
  auto selected_tasks = getSelectedTasks();

  if (selected_tasks.size() > 0)
  {
    std::vector<std::shared_ptr<EcalSysTask>> added_tasks;
    added_tasks.reserve(selected_tasks.size());

    // clone all taks
    for (auto& task : selected_tasks)
    {
      std::shared_ptr<EcalSysTask> new_task(new EcalSysTask());
      new_task->SetName                       (task->GetName());
      new_task->SetTarget                     (task->GetTarget());
      new_task->SetAlgoPath                   (task->GetAlgoPath());
      new_task->SetWorkingDir                 (task->GetWorkingDir());
      new_task->SetRunner                     (task->GetRunner());
      new_task->SetLaunchOrder                (task->GetLaunchOrder());
      new_task->SetTimeoutAfterStart          (task->GetTimeoutAfterStart());
      new_task->SetVisibility                 (task->GetVisibility());
      new_task->SetCommandLineArguments       (task->GetCommandLineArguments());
      new_task->SetMonitoringEnabled          (task->IsMonitoringEnabled());
      new_task->SetRestartBySeverityEnabled   (task->IsRestartBySeverityEnabled());
      new_task->SetRestartAtSeverity          (task->GetRestartAtSeverity());

      Globals::EcalSysInstance()->AddTask(new_task);
      task_tree_model_->addTask(new_task);

      added_tasks.push_back(new_task);
    }

    // Select the new taks
    ui_.task_tree->selectionModel()->clearSelection();
    for (auto& task : added_tasks)
    {
      QModelIndex source_index = task_tree_model_->index(task);
      QModelIndex proxy_index = task_tree_sort_filter_proxy_model_->mapFromSource(source_index);
      ui_.task_tree->selectionModel()->select(proxy_index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    // Scroll to the new taks
    QModelIndex first_index = task_tree_sort_filter_proxy_model_->mapFromSource(task_tree_model_->index(added_tasks[0], (int)TaskTreeModel::Columns::TASK_NAME));
    ui_.task_tree->scrollTo(first_index, QAbstractItemView::ScrollHint::PositionAtTop);

    if (added_tasks.size() == 1)
    {
      setEditControlsVisibility(true);
      focusTaskNameLineedit(true);
    }

    // update the completer
    updateTargetCompleter();

    emit tasksAddedSignal(added_tasks);
  }
}

void TaskWidget::removeSelectedTasks()
{
  auto selected_indices = getSelectedIndices();

  // Keep a list of removed tasks
  std::vector<std::shared_ptr<EcalSysTask>> removed_tasks;
  removed_tasks.reserve(selected_indices.size());

  // Keep a list of groups that the tasks have been removed from
  std::set<std::shared_ptr<TaskGroup>> modified_groups;

  for (auto& index : selected_indices)
  {
    // Remove the task from the Global model
    auto task = static_cast<TaskTreeItem*>(index.internalPointer())->getTask();
    auto affected_groups = Globals::EcalSysInstance()->RemoveTask(task);
    removed_tasks.push_back(task);
    for (auto& group : affected_groups)
    {
      modified_groups.emplace(group);
    }
  }
  task_tree_model_->removeItems(selected_indices);

  // update the completer
  updateTargetCompleter();

  emit tasksRemovedSignal(removed_tasks);

  if (modified_groups.size() > 0)
  {
    // Send signal about modified groups
    std::vector<std::shared_ptr<TaskGroup>> modified_groups_vector;
    modified_groups_vector.reserve(modified_groups.size());
    for (auto& modified_group : modified_groups)
    {
      modified_groups_vector.push_back(modified_group);
    }
    emit groupsModifiedSignal(modified_groups_vector);
  }
}

void TaskWidget::startSelectedTasks(const std::string& target_override)
{
  auto selected_tasks_vector = getSelectedTasks();
  std::list<std::shared_ptr<EcalSysTask>> selected_tasks_list;
  for (auto& task : selected_tasks_vector)
  {
    selected_tasks_list.push_back(task);
  }

  auto options = Globals::EcalSysInstance()->GetOptions();
  if (options.check_target_reachability)
  {
    if (target_override != "")
    {
      if (!checkTargetsReachable(std::set<std::string>{target_override}))
      {
        return;
      }
    }
    else if (!options.use_localhost_for_all_tasks)
    {
      if (!checkTargetsReachable(selected_tasks_list))
      {
        return;
      }
    }
  }

  Globals::EcalSysInstance()->StartTaskList(selected_tasks_list, target_override);
  emit startStopActionTriggeredSignal();
}

void TaskWidget::stopSelectedTasks(bool shutdown_request, bool kill_tasks)
{
  auto selected_tasks_vector = getSelectedTasks();
  std::list<std::shared_ptr<EcalSysTask>> selected_tasks_list;
  for (auto& task : selected_tasks_vector)
  {
    selected_tasks_list.push_back(task);
  }
  Globals::EcalSysInstance()->StopTaskList(selected_tasks_list, shutdown_request, kill_tasks);
  emit startStopActionTriggeredSignal();
}

void TaskWidget::restartSelectedTasks(bool shutdown_request, bool kill_tasks, const std::string& target_override)
{
  auto selected_tasks_vector = getSelectedTasks();
  std::list<std::shared_ptr<EcalSysTask>> selected_tasks_list;
  for (auto& task : selected_tasks_vector)
  {
    selected_tasks_list.push_back(task);
  }

  auto options = Globals::EcalSysInstance()->GetOptions();
  if (options.check_target_reachability)
  {
    if (target_override != "")
    {
      if (!checkTargetsReachable(std::set<std::string>{target_override}))
      {
        return;
      }
    }
    else if (!options.use_localhost_for_all_tasks)
    {
      if (!checkTargetsReachable(selected_tasks_list))
      {
        return;
      }
    }
  }

  Globals::EcalSysInstance()->RestartTaskList(selected_tasks_list, shutdown_request, kill_tasks, target_override);
  emit startStopActionTriggeredSignal();
}

void TaskWidget::startAllTasks(const std::string& target_override)
{
  auto tasks = Globals::EcalSysInstance()->GetTaskList();
  auto options = Globals::EcalSysInstance()->GetOptions();

  if (target_override == "")
  {
    if (options.local_tasks_only)
    {
      // Remove all non-local tasks
      tasks.remove_if(
        [](std::shared_ptr<EcalSysTask> task) 
        {
          return QString::compare(task->GetTarget().c_str(), eCAL::Process::GetHostName().c_str(), Qt::CaseSensitivity::CaseInsensitive) != 0;
        });
    }
    else if (!options.use_localhost_for_all_tasks && options.check_target_reachability)
    {
      if (!checkTargetsReachable(tasks))
      {
        return;
      }
    }
  }
  else if (options.check_target_reachability)
  {
    if (!checkTargetsReachable(std::set<std::string>{target_override}))
    {
      return;
    }
  }

  Globals::EcalSysInstance()->StartTaskList(tasks, target_override);
  emit startStopActionTriggeredSignal();
}

void TaskWidget::stopAllTasks(bool shutdown_request, bool kill_tasks)
{
  auto tasks = Globals::EcalSysInstance()->GetTaskList();
  auto options = Globals::EcalSysInstance()->GetOptions();

  if (options.local_tasks_only)
  {
    // Remove all non-local tasks
    tasks.remove_if(
      [](std::shared_ptr<EcalSysTask> task)
      {
        return QString::compare(task->GetTarget().c_str(), eCAL::Process::GetHostName().c_str(), Qt::CaseSensitivity::CaseInsensitive) != 0;
      });
  }
  Globals::EcalSysInstance()->StopTaskList(tasks, shutdown_request, kill_tasks);
  emit startStopActionTriggeredSignal();
}

void TaskWidget::restartAllTasks(bool shutdown_request, bool kill_tasks, const std::string& target_override)
{
  auto tasks = Globals::EcalSysInstance()->GetTaskList();
  auto options = Globals::EcalSysInstance()->GetOptions();

  if (target_override == "")
  {
    if (options.local_tasks_only)
    {
      // Remove all non-local tasks
      tasks.remove_if(
        [](std::shared_ptr<EcalSysTask> task) 
        {
          return QString::compare(task->GetTarget().c_str(), eCAL::Process::GetHostName().c_str(), Qt::CaseSensitivity::CaseInsensitive) != 0;
        });
    }
    else if (!options.use_localhost_for_all_tasks && options.check_target_reachability)
    {
      if (!checkTargetsReachable(tasks))
      {
        return;
      }
    }
  }
  else if (options.check_target_reachability)
  {
    if (!checkTargetsReachable(std::set<std::string>{target_override}))
    {
      return;
    }
  }

  Globals::EcalSysInstance()->RestartTaskList(tasks, shutdown_request, kill_tasks, target_override);
  emit startStopActionTriggeredSignal();
}

void TaskWidget::updateFromCloud()
{
  Globals::EcalSysInstance()->UpdateFromCloud();

  // Update the local task tree
  task_tree_model_->updateAll();
}

////////////////////////////////////////////////////////////////////////////////
//// Update individual edit controls                                        ////
////////////////////////////////////////////////////////////////////////////////

bool TaskWidget::commonName(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->GetName() != task_list[i]->GetName())
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonTarget(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->GetTarget() != task_list[i]->GetTarget())
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonRunner(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
    bool common_value = true;
    for (unsigned int i = 1; i < task_list.size(); i++)
    {
      if (task_list[0]->GetRunner() != task_list[i]->GetRunner())
      {
        common_value = false;
        break;
      }
    }
    return common_value;
}

bool TaskWidget::commonAlgoPath(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->GetAlgoPath() != task_list[i]->GetAlgoPath())
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonWorkingDir(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->GetWorkingDir() != task_list[i]->GetWorkingDir())
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonCommandLine(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->GetCommandLineArguments() != task_list[i]->GetCommandLineArguments())
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonVisibility(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->GetVisibility() != task_list[i]->GetVisibility())
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonLaunchOrder(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->GetLaunchOrder() != task_list[i]->GetLaunchOrder())
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonWaitingTime(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->GetTimeoutAfterStart() != task_list[i]->GetTimeoutAfterStart())
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonMonitoringEnabled(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->IsMonitoringEnabled() != task_list[i]->IsMonitoringEnabled())
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonRestartBySeverityEnabled(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->IsRestartBySeverityEnabled() != task_list[i]->IsRestartBySeverityEnabled())
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonRestartAtSeverity(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->GetRestartAtSeverity().severity != task_list[i]->GetRestartAtSeverity().severity)
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}

bool TaskWidget::commonRestartAtSeverityLevel(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  bool common_value = true;
  for (unsigned int i = 1; i < task_list.size(); i++)
  {
    if (task_list[0]->GetRestartAtSeverity().severity_level != task_list[i]->GetRestartAtSeverity().severity_level)
    {
      common_value = false;
      break;
    }
  }
  return common_value;
}


void TaskWidget::updateEditAreaName(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.task_name_lineedit->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.task_name_lineedit->setPlaceholderText(tr("Task Name"));
    ui_.task_name_lineedit->setText("");
  }
  else
  {
    // Set the appropriate values
    if (commonName(task_list))
    {
      ui_.task_name_lineedit->setText(QString(task_list[0]->GetName().c_str()));
      ui_.task_name_lineedit->setPlaceholderText(tr("Task Name"));
    }
    else
    {
      ui_.task_name_lineedit->setText("");
      ui_.task_name_lineedit->setPlaceholderText(tr(">> Multiple values <<"));
    }
  }
  ui_.task_name_lineedit->blockSignals(false);
}

void TaskWidget::updateEditAreaTarget(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.target_lineedit->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.target_lineedit->setPlaceholderText(tr("Hostname"));
    ui_.target_lineedit->setText("");
  }
  else
  {
    // Set the appropriate values
    if (commonTarget(task_list))
    {
      ui_.target_lineedit->setText(QString(task_list[0]->GetTarget().c_str()));
      ui_.target_lineedit->setPlaceholderText(tr("Hostname"));
    }
    else
    {
      ui_.target_lineedit->setText("");
      ui_.target_lineedit->setPlaceholderText(tr(">> Multiple values <<"));
    }
  }
  ui_.target_lineedit->blockSignals(false);
}

void TaskWidget::updateEditAreaRunner(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.runner_combobox->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.runner_combobox->setCurrentIndex(runner_tree_model_->rowOf(std::shared_ptr<EcalSysRunner>(nullptr)));
  }
  else
  {
    // Set the appropriate values
    if (commonRunner(task_list))
    {
      ui_.runner_combobox->setCurrentIndex(runner_tree_model_->rowOf(task_list[0]->GetRunner()));
    }
    else
    {
      ui_.runner_combobox->setCurrentIndex(-1);
    }
  }
  ui_.runner_combobox->blockSignals(false);
}

void TaskWidget::updateEditAreaAlgoPath(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.algo_path_lineedit->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.algo_path_lineedit->setPlaceholderText(tr("Path"));
    ui_.algo_path_lineedit->setText("");
  }
  else
  {
    // Set the appropriate values
    if (commonAlgoPath(task_list))
    {
      ui_.algo_path_lineedit->setText(QString(task_list[0]->GetAlgoPath().c_str()));
      ui_.algo_path_lineedit->setPlaceholderText(tr("Path"));
    }
    else
    {
      ui_.algo_path_lineedit->setText("");
      ui_.algo_path_lineedit->setPlaceholderText(tr(">> Multiple values <<"));
    }
  }
  ui_.algo_path_lineedit->blockSignals(false);
}

void TaskWidget::updateEditAreaWorkingDir(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.working_dir_lineedit->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.working_dir_lineedit->setPlaceholderText(tr("Path"));
    ui_.working_dir_lineedit->setText("");
  }
  else
  {
    // Set the appropriate values
    if (commonWorkingDir(task_list))
    {
      ui_.working_dir_lineedit->setText(QString(task_list[0]->GetWorkingDir().c_str()));
      ui_.working_dir_lineedit->setPlaceholderText(tr("Path"));
    }
    else
    {
      ui_.working_dir_lineedit->setText("");
      ui_.working_dir_lineedit->setPlaceholderText(tr(">> Multiple values <<"));
    }
  }
  ui_.working_dir_lineedit->blockSignals(false);
}

void TaskWidget::updateEditAreaCommandLine(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.cmd_args_lineedit->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.cmd_args_lineedit->setPlaceholderText(tr("Arguments"));
    ui_.cmd_args_lineedit->setText("");
  }
  else
  {
    // Set the appropriate values
    if (commonCommandLine(task_list))
    {
      ui_.cmd_args_lineedit->setText(QString(task_list[0]->GetCommandLineArguments().c_str()));
      ui_.cmd_args_lineedit->setPlaceholderText(tr("Arguments"));
    }
    else
    {
      ui_.cmd_args_lineedit->setText("");
      ui_.cmd_args_lineedit->setPlaceholderText(tr(">> Multiple values <<"));
    }
  }
  ui_.cmd_args_lineedit->blockSignals(false);
}

void TaskWidget::updateEditAreaVisibility(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.visibility_combobox->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.visibility_combobox->setCurrentIndex(0);
  }
  else
  {
    // Set the appropriate values
    if (commonVisibility(task_list))
    {
      QString visibility_string;
      switch (task_list[0]->GetVisibility())
      {
      case eCAL_Process_eStartMode::proc_smode_hidden:
        visibility_string = "Hidden";
        break;
      case eCAL_Process_eStartMode::proc_smode_normal:
        visibility_string = "Normal";
        break;
      case eCAL_Process_eStartMode::proc_smode_minimized:
        visibility_string = "Minimized";
        break;
      case eCAL_Process_eStartMode::proc_smode_maximized:
        visibility_string = "Maximized";
        break;
      default:
        visibility_string = "";
      }
      ui_.visibility_combobox->setCurrentText(visibility_string);
    }
    else
    {
      ui_.visibility_combobox->setCurrentIndex(-1);
    }
  }
  ui_.visibility_combobox->blockSignals(false);
}

void TaskWidget::updateEditAreaLaunchOrder(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.launch_order_spinbox->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.launch_order_spinbox->setRange(0, ui_.launch_order_spinbox->maximum());
    ui_.launch_order_spinbox->setSpecialValueText("");
    ui_.launch_order_spinbox->setValue(0);
  }
  else
  {
    // Set the appropriate values
    if (commonLaunchOrder(task_list))
    {
      ui_.launch_order_spinbox->setRange(0, ui_.launch_order_spinbox->maximum());
      ui_.launch_order_spinbox->setSpecialValueText("");
      ui_.launch_order_spinbox->setValue(task_list[0]->GetLaunchOrder());
    }
    else
    {
      ui_.launch_order_spinbox->setRange(-1, ui_.launch_order_spinbox->maximum());
      ui_.launch_order_spinbox->setValue(-1);
      ui_.launch_order_spinbox->setSpecialValueText(" ");
    }
  }
  ui_.launch_order_spinbox->blockSignals(false);
}

void TaskWidget::updateEditAreaWaitingTime(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.waiting_time_spinbox->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.waiting_time_spinbox->setRange(0, ui_.waiting_time_spinbox->maximum());
    ui_.waiting_time_spinbox->setSpecialValueText("");
    ui_.waiting_time_spinbox->setValue(0);
  }
  else
  {
    // Set the appropriate values
    if (commonWaitingTime(task_list))
    {
      ui_.waiting_time_spinbox->setRange(0, ui_.waiting_time_spinbox->maximum());
      ui_.waiting_time_spinbox->setSpecialValueText("");
      ui_.waiting_time_spinbox->setValue(std::chrono::duration_cast<std::chrono::duration<int, std::milli>>(task_list[0]->GetTimeoutAfterStart()).count());
    }
    else
    {
      ui_.waiting_time_spinbox->setRange(-1, ui_.waiting_time_spinbox->maximum());
      ui_.waiting_time_spinbox->setValue(-1);
      ui_.waiting_time_spinbox->setSpecialValueText(" ");
    }
  }
  ui_.waiting_time_spinbox->blockSignals(false);
}

void TaskWidget::updateEditAreaMonitoringEnabled(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.enable_monitoring_checkbox->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.enable_monitoring_checkbox->setCheckState(Qt::CheckState::Checked);
  }
  else
  {
    // Set the appropriate values
    if (commonMonitoringEnabled(task_list))
    {
      ui_.enable_monitoring_checkbox->setCheckState((task_list[0]->IsMonitoringEnabled() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked));
    }
    else
    {
      ui_.enable_monitoring_checkbox->setCheckState(Qt::CheckState::PartiallyChecked);
    }
  }
  ui_.enable_monitoring_checkbox->blockSignals(false);
}

void TaskWidget::updateEditAreaRestartBySeverityEnabled(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.restart_by_severity_checkbox->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.restart_by_severity_checkbox->setCheckState(Qt::CheckState::Unchecked);
    ui_.restart_by_severity_warning_label->setVisible(false);
  }
  else
  {
    // Set the appropriate values
    if (commonRestartBySeverityEnabled(task_list))
    {
      ui_.restart_by_severity_checkbox->setCheckState((task_list[0]->IsRestartBySeverityEnabled() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked));
    }
    else
    {
      ui_.restart_by_severity_checkbox->setCheckState(Qt::CheckState::PartiallyChecked);
    }
    ui_.restart_by_severity_warning_label->setVisible((ui_.restart_by_severity_checkbox->checkState() != Qt::CheckState::Unchecked));
  }
  ui_.restart_by_severity_checkbox->blockSignals(false);
}

void TaskWidget::updateEditAreaRestartAtSeverity(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.restart_by_severity_combobox->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.restart_by_severity_combobox->setCurrentIndex(severity_model_->getRow(eCAL_Process_eSeverity::proc_sev_failed));
  }
  else
  {
    // Set the appropriate values
    if (commonRestartAtSeverity(task_list))
    {
      ui_.restart_by_severity_combobox->setCurrentIndex(severity_model_->getRow(task_list[0]->GetRestartAtSeverity().severity));
    }
    else
    {
      ui_.restart_by_severity_combobox->setCurrentIndex(-1);
    }
  }
  ui_.restart_by_severity_combobox->blockSignals(false);
}

void TaskWidget::updateEditAreaRestartAtSeverityLevel(std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  ui_.restart_by_severity_level_combobox->blockSignals(true);
  if (task_list.size() == 0)
  {
    ui_.restart_by_severity_level_combobox->setCurrentIndex(severity_level_model_->getRow(eCAL_Process_eSeverity_Level::proc_sev_level1));
  }
  else
  {
    // Set the appropriate values
    if (commonRestartAtSeverityLevel(task_list))
    {
      ui_.restart_by_severity_level_combobox->setCurrentIndex(severity_level_model_->getRow(task_list[0]->GetRestartAtSeverity().severity_level));
    }
    else
    {
      ui_.restart_by_severity_level_combobox->setCurrentIndex(-1);
    }
  }
  ui_.restart_by_severity_level_combobox->blockSignals(false);
}

////////////////////////////////////////////////////////////////////////////////
//// Callbacks for the Edit Controls                                        ////
////////////////////////////////////////////////////////////////////////////////


void TaskWidget::nameEditingFinished()
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    std::string text = ui_.task_name_lineedit->text().toStdString();

    // In batch-edit mode with multiple values, we interpret an empty string as "don't change anything"
    if (!commonName(selected_tasks) && (text == ""))
    {
      return;
    }

    ui_.task_name_lineedit->setPlaceholderText(tr("Task Name"));

    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
    for (auto& task : selected_tasks)
    {
      if (text != task->GetName())
      {
        task->SetName(text);
        modified_tasks.push_back(task);
        task_tree_model_->updateTask(task);
      }
    }

    if (modified_tasks.size() > 0)
    {
      emit tasksModifiedSignal(modified_tasks);
    }
  }
}

void TaskWidget::targetEditingFinished()
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    std::string text = ui_.target_lineedit->text().toStdString();

    // In batch-edit mode with multiple values, we interpret an empty string as "don't change anything"
    if (!commonTarget(selected_tasks) && (text == ""))
    {
      return;
    }

    ui_.target_lineedit->setPlaceholderText(tr("Hostname"));

    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
    for (auto& task : selected_tasks)
    {
      if (text != task->GetTarget())
      {
        task->SetTarget(text);
        modified_tasks.push_back(task);
        task_tree_model_->updateTask(task);
      }
    }

    if (modified_tasks.size() > 0)
    {
      emit tasksModifiedSignal(modified_tasks);
    }

    // update the completer (We cannot update the completer right now, as the
    // completer itself might have called this function. Changing the completers
    // configuration right away causes it to set faulty data to the lineedit.
    QTimer::singleShot(10, [this]() {updateTargetCompleter(); });
  }
}

void TaskWidget::runnerIndexChanged(int index)
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0 && index >= 0)
  {
    auto selected_runner = runner_tree_model_->getRunnerByRow(index);
    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
    for (auto& task : selected_tasks)
    {
      if (selected_runner != task->GetRunner())
      {
        task->SetRunner(selected_runner);
        modified_tasks.push_back(task);
        task_tree_model_->updateTask(task);
      }
    }

    if (modified_tasks.size() > 0)
    {
      updateCommandLineWorkingDirPreviews();
      emit tasksModifiedSignal(modified_tasks);
    }
  }
}

void TaskWidget::showRunnersButtonClicked()
{
  int selected_runner_row = ui_.runner_combobox->currentIndex();
  std::shared_ptr<EcalSysRunner> selected_runner(nullptr);
  if (selected_runner_row >= 0)
  {
    selected_runner = runner_tree_model_->getRunnerByRow(selected_runner_row);
  }
  emit showRunnerWindowSignal(selected_runner);
}

void TaskWidget::algoPathEditingFinished()
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    QString text = ui_.algo_path_lineedit->text();

    // In batch-edit mode with multiple values, we interpret an empty string as "don't change anything"
    if (!commonAlgoPath(selected_tasks) && (text == ""))
    {
      return;
    }

    ui_.algo_path_lineedit->setPlaceholderText(tr("Path"));

    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
    for (auto& task : selected_tasks)
    {
      if (text.toStdString() != task->GetAlgoPath())
      {
        task->SetAlgoPath(text.toStdString());
        // Conveniently also set the name if it hasn't been set, yet
        if (task->GetName().empty())
        {
          // Evaluate the path as it would be on this machine
          std::string parsed_string = EcalParser::Evaluate(text.toStdString(), true);

          // Get the base name (e.g. the name of the .exe file)
          QFileInfo file_info(QString::fromStdString(parsed_string));
          QString algo_name = file_info.baseName();
          task->SetName(algo_name.toStdString());
        }
        task_tree_model_->updateTask(task);
        modified_tasks.push_back(task);
      }
    }

    updateEditAreaName(selected_tasks);
    if (modified_tasks.size() > 0)
    {
      updateCommandLineWorkingDirPreviews();
      emit tasksModifiedSignal(modified_tasks);
    }
  }
}

void TaskWidget::algoPathChooseButtonClicked()
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    // Guessing which directory the user might want to start at
    QString start_path = "";

    // Majority voting for the working dir
    std::map<std::string, int> current_working_dir_votes;
    for (auto& task : selected_tasks)
    {
      std::string task_working_dir = task->GetWorkingDir();
      if (!task_working_dir.empty())
      {
        if (current_working_dir_votes.find(task_working_dir) == current_working_dir_votes.end())
        {
          current_working_dir_votes[task_working_dir] = 0;
        }
        current_working_dir_votes[task_working_dir]++;
      }
    }
    std::string most_popular_working_dir = "";
    int most_popular_working_dir_votes = 0;
    for (auto& vote : current_working_dir_votes)
    {
      if (vote.second > most_popular_working_dir_votes)
      {
        most_popular_working_dir       = vote.first;
        most_popular_working_dir_votes = vote.second;
      }
    }

    // Majority voting for the runner's default algo dir
    std::map<std::string, int> runner_algo_dir_votes;
    for (auto& task : selected_tasks)
    {
      auto runner = task->GetRunner();
      if (runner)
      {
        std::string runner_algo_dir = runner->GetDefaultAlgoDir();
        if (!runner_algo_dir.empty())
        {
          if (runner_algo_dir_votes.find(runner_algo_dir) == runner_algo_dir_votes.end())
          {
            runner_algo_dir_votes[runner_algo_dir] = 0;
          }
          runner_algo_dir_votes[runner_algo_dir]++;
        }
      }
    }
    std::string most_popular_runner_algo_dir = "";
    int most_popular_runner_algo_dir_votes = 0;
    for (auto& vote : runner_algo_dir_votes)
    {
      if (vote.second > most_popular_runner_algo_dir_votes)
      {
        most_popular_runner_algo_dir       = vote.first;
        most_popular_runner_algo_dir_votes = vote.second;
      }
    }

    // Majority voting for the tasks current algo path
    std::map<std::string, int> task_algo_path_votes;
    for (auto& task : selected_tasks)
    {
      std::string task_algo_path = task->GetAlgoPath();
      if (!task_algo_path.empty())
      {
        if (task_algo_path_votes.find(task_algo_path) == task_algo_path_votes.end())
        {
          task_algo_path_votes[task_algo_path] = 0;
        }
        task_algo_path_votes[task_algo_path]++;
      }
    }
    std::string most_popular_task_algo_path = "";
    int most_popular_task_algo_path_votes = 0;
    for (auto& vote : task_algo_path_votes)
    {
      if (vote.second > most_popular_task_algo_path_votes)
      {
        most_popular_task_algo_path = vote.first;
        most_popular_task_algo_path_votes = vote.second;
      }
    }

    if (!most_popular_task_algo_path.empty())
    {
      // Use the current algo as start
      start_path = most_popular_task_algo_path.c_str();
    }
    else if (!most_popular_runner_algo_dir.empty())
    {
      // Use the default algo working dir as start
      start_path = most_popular_runner_algo_dir.c_str();
    }
    else if (!most_popular_working_dir.empty())
    {
      // Use the current working dir as start
      start_path = most_popular_working_dir.c_str();
    }

    // If we still don't have a path, let's check the data stored in the settings
    if (start_path == "")
    {
      QSettings settings;
      settings.beginGroup("taskwidget");
      start_path = settings.value("last_algo_dir").toString();
      settings.endGroup();
    }

    QString algo_path = QFileDialog::getOpenFileName(this, tr("Choose Algo Path"), start_path, tr("All Files (*)"));
    if (!algo_path.isEmpty())
    {
#ifdef WIN32
      algo_path.replace('/', '\\');
#endif // WIN32

      ui_.algo_path_lineedit->setPlaceholderText(tr("Path"));

      std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
      for (auto& task : selected_tasks)
      {
        if (algo_path.toStdString() != task->GetAlgoPath())
        {
          task->SetAlgoPath(algo_path.toStdString());
          // Conveniently also set the name if it hasn't been set, yet
          if (task->GetName().empty())
          {
            QFileInfo file_info(algo_path);
            QString algo_name = file_info.baseName();
            task->SetName(algo_name.toStdString());
          }
          task_tree_model_->updateTask(task);
          modified_tasks.push_back(task);
        }
      }

      updateEditAreaAlgoPath     (selected_tasks);
      updateEditAreaName         (selected_tasks);

      if (modified_tasks.size() > 0)
      {
        updateCommandLineWorkingDirPreviews();
        emit tasksModifiedSignal(modified_tasks);
      }

      QSettings settings;
      settings.beginGroup("taskwidget");
      settings.setValue("last_algo_dir", QFileInfo(algo_path).absoluteDir().absolutePath());
      settings.endGroup();
    }
  }
}

void TaskWidget::workingDirEditingFinished()
{
  auto selected_tasks = getSelectedTasks();

  if (selected_tasks.size() != 0)
  {
    std::string text = ui_.working_dir_lineedit->text().toStdString();

    // In batch-edit mode with multiple values, we interpret an empty string as "don't change anything"
    if (!commonWorkingDir(selected_tasks) && (text == ""))
    {
      return;
    }

    ui_.working_dir_lineedit->setPlaceholderText(tr("Path"));

    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
    for (auto& task : selected_tasks)
    {
      if (text != task->GetWorkingDir())
      {
        task->SetWorkingDir(text);
        task_tree_model_->updateTask(task);
        modified_tasks.push_back(task);
      }
    }

    if (modified_tasks.size() > 0)
    {
      updateCommandLineWorkingDirPreviews();
      emit tasksModifiedSignal(modified_tasks);
    }
  }
}

void TaskWidget::workingDirChooseButtonClicked()
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    // Guessing which directory the user might want to start at
    QString start_directory = "";
    
    // Majority voting for the working dir
    std::map<std::string, int> current_working_dir_votes;
    for (auto& task : selected_tasks)
    {
      std::string task_working_dir = task->GetWorkingDir();
      if (!task_working_dir.empty())
      {
        if (current_working_dir_votes.find(task_working_dir) == current_working_dir_votes.end())
        {
          current_working_dir_votes[task_working_dir] = 0;
        }
        current_working_dir_votes[task_working_dir]++;
      }
    }
    std::string most_popular_working_dir = "";
    int most_popular_working_dir_votes = 0;
    for (auto& vote : current_working_dir_votes)
    {
      if (vote.second > most_popular_working_dir_votes)
      {
        most_popular_working_dir       = vote.first;
        most_popular_working_dir_votes = vote.second;
      }
    }

    // Majority voting for the runner's default algo dir
    std::map<std::string, int> runner_algo_dir_votes;
    for (auto& task : selected_tasks)
    {
      auto runner = task->GetRunner();
      if (runner)
      {
        std::string runner_algo_dir = runner->GetDefaultAlgoDir();
        if (!runner_algo_dir.empty())
        {
          if (runner_algo_dir_votes.find(runner_algo_dir) == runner_algo_dir_votes.end())
          {
            runner_algo_dir_votes[runner_algo_dir] = 0;
          }
          runner_algo_dir_votes[runner_algo_dir]++;
        }
      }
    }
    std::string most_popular_runner_algo_dir = "";
    int most_popular_runner_algo_dir_votes = 0;
    for (auto& vote : runner_algo_dir_votes)
    {
      if (vote.second > most_popular_runner_algo_dir_votes)
      {
        most_popular_runner_algo_dir       = vote.first;
        most_popular_runner_algo_dir_votes = vote.second;
      }
    }

    // Majority voting for the tasks current algo path
    std::map<std::string, int> task_algo_path_votes;
    for (auto& task : selected_tasks)
    {
      std::string task_algo_path = task->GetAlgoPath();
      if (!task_algo_path.empty())
      {
        if (task_algo_path_votes.find(task_algo_path) == task_algo_path_votes.end())
        {
          task_algo_path_votes[task_algo_path] = 0;
        }
        task_algo_path_votes[task_algo_path]++;
      }
    }
    std::string most_popular_task_algo_path = "";
    int most_popular_task_algo_path_votes = 0;
    for (auto& vote : task_algo_path_votes)
    {
      if (vote.second > most_popular_task_algo_path_votes)
      {
        most_popular_task_algo_path       = vote.first;
        most_popular_task_algo_path_votes = vote.second;
      }
    }

    if (!most_popular_working_dir.empty())
    {
      // Use the current working dir as start
      start_directory = most_popular_working_dir.c_str();
    }
    else if (!most_popular_runner_algo_dir.empty())
    {
      // Use the default algo working dir as start
      start_directory = most_popular_runner_algo_dir.c_str();
    }
    else if (!most_popular_task_algo_path.empty())
    {
      // Use the directory of the algo as start
      QFileInfo file_info(most_popular_task_algo_path.c_str());
      start_directory = file_info.absoluteDir().absolutePath();
    }

    // If we still don't have a path, let's check the data stored in the settings
    if (start_directory == "")
    {
      QSettings settings;
      settings.beginGroup("taskwidget");
      start_directory = settings.value("last_working_dir").toString();
      settings.endGroup();
    }

    QString working_directory = QFileDialog::getExistingDirectory(this, tr("Choose Working Directory"), start_directory);
    if (!working_directory.isEmpty())
    {
#ifdef WIN32
      working_directory.replace('/', '\\');
#endif // WIN32

      std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
      for (auto& task : selected_tasks)
      {
        if (working_directory.toStdString() != task->GetWorkingDir())
        {
          task->SetWorkingDir(working_directory.toStdString());
          task_tree_model_->updateTask(task);
          modified_tasks.push_back(task);
        }
      }

      updateEditAreaWorkingDir   (selected_tasks);

      if (modified_tasks.size() > 0)
      {
        updateCommandLineWorkingDirPreviews();
        emit tasksModifiedSignal(modified_tasks);
      }

      QSettings settings;
      settings.beginGroup("taskwidget");
      settings.setValue("last_working_dir", working_directory);
      settings.endGroup();
    }
  }
}

void TaskWidget::cmdArgsEditingFinished()
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    std::string text = ui_.cmd_args_lineedit->text().toStdString();

    // In batch-edit mode with multiple values, we interpret an empty string as "don't change anything"
    if (!commonCommandLine(selected_tasks) && (text == ""))
    {
      return;
    }

    ui_.cmd_args_lineedit->setPlaceholderText("Arguments");

    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
    for (auto& task : selected_tasks)
    {
      if (text != task->GetCommandLineArguments())
      {
        task->SetCommandLineArguments(text);
        modified_tasks.push_back(task);
        task_tree_model_->updateTask(task);
      }
    }

    if (modified_tasks.size() > 0)
    {
      updateCommandLineWorkingDirPreviews();
      emit tasksModifiedSignal(modified_tasks);
    }
  }
}

void TaskWidget::visibilityTextChanged()
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    std::string text = ui_.visibility_combobox->currentText().toStdString();
    eCAL_Process_eStartMode visibility = eCAL_Process_eStartMode::proc_smode_normal;
    if (text == "Normal")
    {
      visibility = eCAL_Process_eStartMode::proc_smode_normal;
    }
    else if (text == "Maximized")
    {
      visibility = eCAL_Process_eStartMode::proc_smode_maximized;
    }
    else if (text == "Minimized")
    {
      visibility = eCAL_Process_eStartMode::proc_smode_minimized;
    }
    else if (text == "Hidden")
    {
      visibility = eCAL_Process_eStartMode::proc_smode_hidden;
    }

    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
    for (auto& task : selected_tasks)
    {
      if (visibility != task->GetVisibility())
      {
        task->SetVisibility(visibility);
        modified_tasks.push_back(task);
        task_tree_model_->updateTask(task);
      }
    }

    if (modified_tasks.size() > 0)
    {
      emit tasksModifiedSignal(modified_tasks);
    }
  }
}

void TaskWidget::launchOrderValueChanged()
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    int value = ui_.launch_order_spinbox->value();
    if (value >= 0)
    {
      std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
      ui_.launch_order_spinbox->setRange(0, ui_.launch_order_spinbox->maximum());
      ui_.launch_order_spinbox->setSpecialValueText("");

      for (auto& task : selected_tasks)
      {
        if ((unsigned int)value != task->GetLaunchOrder())
        {
          task->SetLaunchOrder(value);
          modified_tasks.push_back(task);
          task_tree_model_->updateTask(task);
        }
      }

      if (modified_tasks.size() > 0)
      {
        emit tasksModifiedSignal(modified_tasks);
      }
    }
  }
}

void TaskWidget::waitingTimeValueChanged()
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    int value = ui_.waiting_time_spinbox->value();
    if (value >= 0)
    {
      std::chrono::milliseconds timout_after_start(value);

      std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
      ui_.waiting_time_spinbox->setRange(0, ui_.waiting_time_spinbox->maximum());
      ui_.waiting_time_spinbox->setSpecialValueText("");

      for (auto& task : selected_tasks)
      {
        if (timout_after_start != task->GetTimeoutAfterStart())
        {
          task->SetTimeoutAfterStart(timout_after_start);
          modified_tasks.push_back(task);
          task_tree_model_->updateTask(task);
        }
      }

      if (modified_tasks.size() > 0)
      {
        emit tasksModifiedSignal(modified_tasks);
      }
    }
  }
}

void TaskWidget::enableMonitoringStateChanged(int check_state)
{
  // We change a PartiallyChecked to Checked, as the default value for the monitoring is true.
  ui_.enable_monitoring_checkbox->blockSignals(true);
  bool enabled = ((check_state == Qt::CheckState::PartiallyChecked) || (check_state == Qt::CheckState::Checked));
  ui_.enable_monitoring_checkbox->setChecked(enabled);
  ui_.enable_monitoring_checkbox->setTristate(false);
  ui_.enable_monitoring_checkbox->blockSignals(false);

  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;

    for (auto& task : selected_tasks)
    {
      if (enabled != task->IsMonitoringEnabled())
      {
        task->SetMonitoringEnabled(enabled);
        modified_tasks.push_back(task);
        task_tree_model_->updateTask(task);
      }
    }

    if (modified_tasks.size() > 0)
    {
      emit tasksModifiedSignal(modified_tasks);
    }
  }
  updateMonitoringControlsEnabledState();
}

void TaskWidget::restartBySeverityEnabledStateChanged(int check_state)
{
  // We change a PartiallyChecked to Unchecked, as the default value for the monitoring is false. Defaulting to true here may be disastrous, if some of the processes are running.
  ui_.restart_by_severity_checkbox->blockSignals(true);
  bool enabled = (check_state == Qt::CheckState::Checked);
  ui_.restart_by_severity_checkbox->setChecked(enabled);
  ui_.restart_by_severity_checkbox->setTristate(false);
  ui_.restart_by_severity_checkbox->blockSignals(false);

  // Set the warning label
  ui_.restart_by_severity_warning_label->setVisible(enabled);

  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;

    for (auto& task : selected_tasks)
    {
      if (enabled != task->IsRestartBySeverityEnabled())
      {
        task->SetRestartBySeverityEnabled(enabled);
        modified_tasks.push_back(task);
        task_tree_model_->updateTask(task);
      }
    }

    if (modified_tasks.size() > 0)
    {
      emit tasksModifiedSignal(modified_tasks);
    }
  }
  updateMonitoringControlsEnabledState();
}

void TaskWidget::restartBySeverityIndexChanged(int index)
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    eCAL_Process_eSeverity selected_severity = severity_model_->getSeverity(index);

    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
    for (auto& task : selected_tasks)
    {
      if (selected_severity != task->GetRestartAtSeverity().severity)
      {
        TaskState restart_at_severity = task->GetRestartAtSeverity();
        restart_at_severity.severity = selected_severity;
        task->SetRestartAtSeverity(restart_at_severity);
        modified_tasks.push_back(task);
        task_tree_model_->updateTask(selected_tasks.front());
      }
    }
    if (modified_tasks.size() > 0)
    {
      emit tasksModifiedSignal(modified_tasks);
    }
  }
}

void TaskWidget::restartBySeverityLevelIndexChanged(int index)
{
  auto selected_tasks = getSelectedTasks();
  if (selected_tasks.size() != 0)
  {
    eCAL_Process_eSeverity_Level selected_severity_level = severity_level_model_->getSeverityLevel(index);

    std::vector<std::shared_ptr<EcalSysTask>> modified_tasks;
    for (auto& task : selected_tasks)
    {
      if (selected_severity_level != task->GetRestartAtSeverity().severity_level)
      {
        TaskState restart_at_severity = task->GetRestartAtSeverity();
        restart_at_severity.severity_level = selected_severity_level;
        task->SetRestartAtSeverity(restart_at_severity);
        modified_tasks.push_back(task);
        task_tree_model_->updateTask(selected_tasks.front());
      }
    }
    if (modified_tasks.size() > 0)
    {
      emit tasksModifiedSignal(modified_tasks);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
//// Public Slots for external control signals                              ////
////////////////////////////////////////////////////////////////////////////////

void TaskWidget::tasksAdded(const std::vector<std::shared_ptr<EcalSysTask>>& tasks)
{
  for (auto& task : tasks)
  {
    task_tree_model_->addTask(task);
  }
  updateTargetCompleter();
}

void TaskWidget::tasksRemoved(const std::vector<std::shared_ptr<EcalSysTask>>& tasks)
{
  for (auto& task : tasks)
  {
    task_tree_model_->removeTask(task);
  }
  updateTargetCompleter();
  updateCommandLineWorkingDirPreviews();
}

void TaskWidget::tasksModified(const std::vector<std::shared_ptr<EcalSysTask>>& tasks)
{
  for (auto& task : tasks)
  {
    task_tree_model_->updateTask(task);
  }
  
  updateTargetCompleter();
  updateCommandLineWorkingDirPreviews();

  // Also update the edit-panel by calling the selection changed method
  taskTableSelectionChanged();
}

void TaskWidget::runnersAdded(const std::vector<std::shared_ptr<EcalSysRunner>>& runners)
{
  for (auto& runner : runners)
  {
    runner_tree_model_->addRunner(runner);
  }
}

void TaskWidget::runnersRemoved(const std::vector<std::shared_ptr<EcalSysRunner>>& runners)
{
  // removing a runner may change the current selection. therefore, we have to temporarily disable the signals of the combobox
  ui_.runner_combobox->blockSignals(true);
  for (auto& runner : runners)
  {
    runner_tree_model_->removeRunner(runner);
  }
  ui_.runner_combobox->blockSignals(false);
}

void TaskWidget::runnersModified(const std::vector<std::shared_ptr<EcalSysRunner>>& runners)
{
  for (auto& runner : runners)
  {
    runner_tree_model_->updateRunner(runner);
  }
  updateCommandLineWorkingDirPreviews();
}

void TaskWidget::ecalsysOptionsChanged()
{
  task_tree_model_->updateAll();

}

void TaskWidget::monitorUpdated()
{
  emit monitorUpdatedSignal();
}

void TaskWidget::configChanged()
{
  // Clear the selection in the task table, as otherwise the controls may induce
  // faulty values, e.g. for the runner, when the model reloads
  ui_.task_tree->selectionModel()->clearSelection();

  runner_tree_model_->reload();
  task_tree_model_->reload();
  updateTargetCompleter();
  updateCommandLineWorkingDirPreviews();
  taskTableSelectionChanged();
  ui_.filter_lineedit->setText("");
}

////////////////////////////////////////////////////////////////////////////////
//// Auxiliary                                                              ////
////////////////////////////////////////////////////////////////////////////////

bool TaskWidget::checkTargetsReachable(const std::list<std::shared_ptr<EcalSysTask>>& task_list)
{
  std::set<std::string> target_set;
  for (auto& task : task_list)
  {
    target_set.emplace(task->GetTarget());
  }
  return checkTargetsReachable(target_set);
}

bool TaskWidget::checkTargetsReachable(const std::set<std::string>& target_set)
{
  std::vector<std::string> target_vector;
  target_vector.reserve(target_set.size());

  for (std::string target : target_set)
  {
    target_vector.push_back(target);
  }

  bool any_target_not_reachable = false;
  for (std::string target : target_vector)
  {
    if ((target != eCAL::Process::GetHostName())
      && !Globals::EcalSysInstance()->IseCALSysClientRunningOnHost(target))
    {
      any_target_not_reachable = true;
      break;
    }
  }

  if (any_target_not_reachable)
  {
    TargetsReachableDialog dialog(this, target_vector);
    connect(this, SIGNAL(monitorUpdatedSignal()), &dialog, SLOT(update()));
    dialog.exec();
    return dialog.shallStartTasks();
  }

  return true;
}
