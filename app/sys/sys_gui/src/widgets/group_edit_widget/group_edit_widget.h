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

#include <memory>
#include "ecalsys/task/task_group.h"

#include <QWidget>
#include "ui_group_edit_widget.h"

#include "widgets/treemodels/group_state_tree_model.h"
#include "widgets/treemodels/task_tree_model.h"
#include "widgets/treemodels/group_state_min_task_state_tree_model.h"
#include "CustomQt/QStableSortFilterProxyModel.h"
#include "severity_picker_delegate.h"

#include "globals.h"

class GroupEditWidget : public QWidget
{
  Q_OBJECT

public:
  GroupEditWidget(QWidget *parent = Q_NULLPTR);
  ~GroupEditWidget();

public slots:
  void resetLayout();

  void setGroup(std::shared_ptr<TaskGroup> group);
  void focusGroupName(bool select_text = false);

  void ecalsysOptionsChanged();
  void monitorUpdated();

  void tasksModified(const std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void tasksAdded   (const std::vector<std::shared_ptr<EcalSysTask>>& task_list);
  void tasksRemoved (const std::vector<std::shared_ptr<EcalSysTask>>& task_list);


  void groupsModified(std::vector<std::shared_ptr<TaskGroup>> group_list);


private slots:
  void groupStateSelectionChanged();
  void availableTasksSelectionChanged();
  void activeTasksSelectionChanged();

  void updateMoveUpDownButtons();
  void updateTasks();

  // Edit Controls
  void groupNameEditingFinished();
  void groupStateAddButtonClicked();
  void groupStateRemoveButtonClicked();
  void groupStateMoveUpButtonClicked();
  void groupStateMoveDownButtonClicked();

  void stateNameEditingFinished();
  void stateColorButtonClicked();
  void stateAddTaskButtonClicked();
  void stateRemoveTaskButtonClicked();

signals:
  void groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>> group_list);

protected:
  void showEvent(QShowEvent* event);

private:
  Ui::GroupEditWidget ui_;

  QByteArray group_state_tree_initial_state_;
  QByteArray available_tasks_tree_initial_state_;
  QByteArray active_tasks_tree_initial_state_;

  QByteArray main_splitter_initial_state_;
  QByteArray state_trees_splitter_initial_state_;
  bool initial_splitter_state_valid_;

  std::shared_ptr<TaskGroup> group_;
  std::shared_ptr<TaskGroup::GroupState> group_state_;

  GroupStateTreeModel*             group_state_tree_model_;
  TaskTreeModel*                   available_tasks_tree_model_;
  QStableSortFilterProxyModel*     available_tasks_proxy_tree_model_;
  GroupStateMinTaskStateTreeModel* active_tasks_tree_model_;
  QStableSortFilterProxyModel*     active_tasks_proxy_tree_model_;

  SeverityPickerDelegate*          severity_picker_delegate_;

  void editState(std::shared_ptr<TaskGroup::GroupState> state);
};
