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

#include <QWidget>

#include <regex>

#include "ui_import_from_cloud_widget.h"

#include "widgets/treemodels/import_from_cloud_task_tree_model.h"
#include "widgets/treemodels/import_from_cloud_runner_tree_model.h"
#include "CustomQt/QStableSortFilterProxyModel.h"
#include "CustomQt/QCheckboxHeaderView.h"

class ImportFromCloudWidget : public QWidget
{
  Q_OBJECT

public:
  ImportFromCloudWidget(QWidget *parent = Q_NULLPTR);
  ~ImportFromCloudWidget();

public slots:
  void resetLayout();

  void clear();
  void reload();
  void setUpdateEnabled(bool enabled);

  void monitorUpdated();
  void tasksChanged(std::vector<std::shared_ptr<EcalSysTask>>);
  void ecalsysOptionsChanged();
  void configChanged();

private slots:
  void taskTreeContextMenu(const QPoint& pos);
  void runnerTreeContextMenu(const QPoint& pos);
  void updateButtons();
  void updateTaskTableHeaderCheckbox();
  void updateImportCheckboxEnabledStates();
  void import();

signals:
  void tasksAddedSignal(std::vector<std::shared_ptr<EcalSysTask>> tasks);
  void runnersAddedSignal(std::vector<std::shared_ptr<EcalSysRunner>> runners);

  void closeSignal();

protected:
  void showEvent(QShowEvent* event);

private:
  Ui::ImportFromCloudWidget ui_;

  QByteArray                      initial_task_tree_state_;
  QByteArray                      initial_runner_tree_state_;
  QByteArray                      initial_splitter_state_;
  bool                            initial_splitter_state_valid_;

  ImportFromCloudTaskTreeModel*   task_tree_model_;
  QStableSortFilterProxyModel*    task_sort_filter_proxy_model_;
  QCheckboxHeaderView*            task_tree_header_;
  ImportFromCloudRunnerTreeModel* runner_tree_model_;
  QStableSortFilterProxyModel*    runner_sort_filter_proxy_model_;
  QCheckboxHeaderView*            runner_tree_header_;

  std::list<std::shared_ptr<EcalSysTask>> task_list_;
  std::list<std::shared_ptr<EcalSysRunner>> runner_list_;

  std::regex                      exclude_tasks_regex_;
  bool                            exclude_tasks_regex_valid_;

  bool                            update_enabled_;

  void autoDetectRunnersFor(const std::list<std::shared_ptr<EcalSysTask>>& task_list, bool force_create = false);

  static void attemptRunnerCommandLineSeparation(
    const std::string& input_command_line,
    bool&              first_arg_is_algo_path,
    bool&              second_arg_is_algo_path,
    std::string&       runner_load_arg);

  std::list<std::shared_ptr<EcalSysTask>> getSelectedTasks();
  std::list<std::shared_ptr<EcalSysRunner>> getSelectedRunners();
  std::list<std::shared_ptr<EcalSysTask>> getCheckedTasks();
  std::list<std::shared_ptr<EcalSysTask>> getCheckableTasks();

  void removeRunnerAssignment(const std::list<std::shared_ptr<EcalSysTask>>& task_list);
  void removeRunnerAssignment(const std::list<std::shared_ptr<EcalSysRunner>>& runner_list);
  void removeRunners(const std::list<std::shared_ptr<EcalSysRunner>>& runner_list);
  void autoAssignRunners(const std::list<std::shared_ptr<EcalSysRunner>>& runner_list);

  void loadExcludeTasksFilter();
};
