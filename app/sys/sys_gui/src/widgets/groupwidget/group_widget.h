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

#include <memory>
#include "ecalsys/task/ecal_sys_task.h"
#include "ecalsys/task/task_group.h"

#include <QWidget>
#include <CustomQt/QStableSortFilterProxyModel.h>
#include "ui_group_widget.h"

#include "widgets/treemodels/group_tree_model.h"
#include "widgets/group_edit_widget/group_edit_widget.h"
#include "widgets/treemodels/group_tree_item.h"

class GroupWidget : public QWidget
{
  Q_OBJECT

public:
  GroupWidget(GroupEditWidget* group_edit_widget, QWidget *parent = Q_NULLPTR);
  ~GroupWidget();

  std::set<std::shared_ptr<EcalSysTask>> getAllSelectedTasks();
  std::list<std::shared_ptr<TaskGroup>> getSelectedGroups();

public slots:
  void resetLayout();
  void saveGuiSettings();
  void loadGuiSettings();

  void monitorUpdated();
  void configChanged();
  void ecalsysOptionsChanged();

  void tasksAdded(std::vector<std::shared_ptr<EcalSysTask>> tasks);
  void tasksRemoved(std::vector<std::shared_ptr<EcalSysTask>> tasks);
  void tasksModified(const std::vector<std::shared_ptr<EcalSysTask>>& tasks);

  void groupsModified(const std::vector<std::shared_ptr<TaskGroup>>& groups);

  // Buttonsex
  void addGroup();
  void removeSelectedGroups();
  void duplicateSelectedGroups();
  void startSelectedTasks(const std::string& target_override = "");
  void stopSelectedTasks(bool shutdown_request = true, bool kill_tasks = true);
  void restartSelectedTasks(bool shutdown_request = true, bool kill_tasks = true, const std::string& target_override = "");

  void expandGroups();
  void collapseGroups();

  void startOnHostActionTriggered(QAction* action);
  void restartToHostActionTriggered(QAction* action);

  void showEditControls();
  void hideEditControls();

private slots:
  // Group tree
  void groupTreeSelectionChanged();
  void groupTreeDoubleClicked(const QModelIndex& index);
  void groupTreeContextMenu(const QPoint& pos);
  void filterTextChanged(QString text);

  void updateStartStopButtons();

signals:
  void groupsRemovedSignal(std::vector<std::shared_ptr<TaskGroup>> groups);
  void groupsAddedSignal(std::vector<std::shared_ptr<TaskGroup>> groups);

  void startStopActionTriggeredSignal();

  void selectionChangedSignal();

  void setEditTaskSignal(std::shared_ptr<EcalSysTask> task);


private:
  Ui::GroupWidget ui_;

  QByteArray       initial_group_tree_state_;

  GroupEditWidget* group_edit_widget_;

  GroupTreeModel*              group_tree_model_;
  QStableSortFilterProxyModel* group_sort_filter_proxy_model_;
  bool                         currently_expanded_;

  QList<QModelIndex> getSelectedIndices();
  QList<GroupTreeItem*> getSelectedGroupItems();

  bool checkTargetsReachable(const std::list<std::shared_ptr<EcalSysTask>>& task_list);
  bool checkTargetsReachable(const std::set<std::string>& targets);
};
