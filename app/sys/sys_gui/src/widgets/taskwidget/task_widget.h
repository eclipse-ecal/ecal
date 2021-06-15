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

#pragma once

#include <QFrame>
#include <QWidget>
#include <QSortFilterProxyModel>
#include <QPoint>
#include <QCompleter>
#include <QStringListModel>
#include "CustomQt/QCustomFileSystemModel.h"

#include "ui_task_widget.h"

#include "widgets/treemodels/task_tree_model.h"
#include "widgets/treemodels/task_tree_item.h"

#include "widgets/severity_model/severity_model.h"
#include "widgets/severity_model/severity_level_model.h"

#include "widgets/treemodels/runner_tree_model.h"

class TaskWidget : public QFrame
{
  Q_OBJECT

public:
  TaskWidget(QWidget *parent = Q_NULLPTR);
  ~TaskWidget();

  std::vector<std::shared_ptr<EcalSysTask>> getSelectedTasks();

public slots:
  void configChanged        ();
  void ecalsysOptionsChanged();
  void monitorUpdated       ();

  void tasksAdded   (const std::vector<std::shared_ptr<EcalSysTask>>& tasks);
  void tasksRemoved (const std::vector<std::shared_ptr<EcalSysTask>>& tasks);
  void tasksModified(const std::vector<std::shared_ptr<EcalSysTask>>& tasks);

  void runnersAdded   (const std::vector<std::shared_ptr<EcalSysRunner>>& runners);
  void runnersRemoved (const std::vector<std::shared_ptr<EcalSysRunner>>& runners);
  void runnersModified(const std::vector<std::shared_ptr<EcalSysRunner>>& runners);

  void resetLayout();

  void updateStartStopButtons();

  void setEditTask(std::shared_ptr<EcalSysTask> task);

  // Button Row Slots
  void addTask                   ();
  void duplicateSelectedTasks    ();
  void removeSelectedTasks       ();
  void startSelectedTasks        (const std::string& target_override = "");
  void stopSelectedTasks         (bool shutdown_request = true, bool kill_tasks = true);
  void restartSelectedTasks      (bool shutdown_request = true, bool kill_tasks = true, const std::string& target_override = "");
  void startAllTasks             (const std::string& target_override = "");
  void stopAllTasks              (bool shutdown_request = true, bool kill_tasks = true);
  void restartAllTasks           (bool shutdown_request = true, bool kill_tasks = true, const std::string& target_override = "");
  void updateFromCloud           ();

  // Edit Area Slots
  void toggleEditControlsVisibility();
  void setEditControlsVisibility   (bool visible = true);
  void deactivateEditArea          ();
  void activateEditArea            (std::vector<std::shared_ptr<EcalSysTask>> task_list);

signals:
  void monitorUpdatedSignal();      // relay-signal for the targets-reachable-dialog

  void tasksAddedSignal(std::vector<std::shared_ptr<EcalSysTask>> tasks);
  void tasksRemovedSignal(std::vector<std::shared_ptr<EcalSysTask>> tasks);
  void tasksModifiedSignal(std::vector<std::shared_ptr<EcalSysTask>> tasks);
  
  void groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>> groups);

  void startStopActionTriggeredSignal();

  void selectionChangedSignal();

  void importFromCloudButtonClickedSignal();

  void showRunnerWindowSignal(std::shared_ptr<EcalSysRunner> selected_runner);

private:
  Ui::TaskWidget          ui_;

  QByteArray              initial_task_tree_state_;

  TaskTreeModel*          task_tree_model_;
  QSortFilterProxyModel*  task_tree_sort_filter_proxy_model_;
  RunnerTreeModel*        runner_tree_model_;

  SeverityModel*          severity_model_;
  SeverityLevelModel*     severity_level_model_;

  QCompleter*             target_completer_;
  QStringListModel*       target_completer_model_;

  void autoSizeColumns();

  QList<QModelIndex> getSelectedIndices();

  // Update of the indidividual edit-control-inputs for batch editing tasks
  bool commonName                       (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonTarget                     (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonRunner                     (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonAlgoPath                   (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonWorkingDir                 (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonCommandLine                (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonVisibility                 (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonLaunchOrder                (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonWaitingTime                (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonMonitoringEnabled          (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonRestartBySeverityEnabled   (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonRestartAtSeverity          (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  bool commonRestartAtSeverityLevel     (std::vector<std::shared_ptr<EcalSysTask>>& task_list);

  void updateEditAreaName                       (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaTarget                     (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaRunner                     (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaAlgoPath                   (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaWorkingDir                 (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaCommandLine                (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaVisibility                 (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaLaunchOrder                (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaWaitingTime                (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaMonitoringEnabled          (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaRestartBySeverityEnabled   (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaRestartAtSeverity          (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void updateEditAreaRestartAtSeverityLevel     (std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  
  void updateMonitoringControlsEnabledState();


  bool checkTargetsReachable(const std::list<std::shared_ptr<EcalSysTask>>& task_list);
  bool checkTargetsReachable(const std::set<std::string>& targets);

private slots:
  // Task Table Slots
  void taskTableSelectionChanged(/*const QItemSelection & selected, const QItemSelection & deselected*/);
  void taskTableCurrentChanged(const QModelIndex & current, const QModelIndex & previous);
  void filterTextChanged(QString text);
  void updateTaskTableMonitorInformation();
  void taskTableContextMenu(const QPoint &pos);

  void focusTaskNameLineedit(bool select_text = false);

  void updateTargetCompleter();
  void updateCommandLineWorkingDirPreviews();

  // Edit Area Content Slots
  void nameEditingFinished();
  void targetEditingFinished();
  void runnerIndexChanged(int index);
  void algoPathEditingFinished();
  void showRunnersButtonClicked();
  void algoPathChooseButtonClicked();
  void workingDirEditingFinished();
  void workingDirChooseButtonClicked();
  void cmdArgsEditingFinished();
  void visibilityTextChanged();
  void launchOrderValueChanged();
  void waitingTimeValueChanged();
  void enableMonitoringStateChanged(int check_state);
  void restartBySeverityEnabledStateChanged(int check_state);
  void restartBySeverityIndexChanged(int index);
  void restartBySeverityLevelIndexChanged(int index);
};
