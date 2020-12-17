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
#include "ui_runner_widget.h"

#include <QCompleter>
#include "CustomQt/QCustomFileSystemModel.h"

#include "widgets/treemodels/runner_tree_model.h"

class RunnerWidget : public QWidget
{
  Q_OBJECT

public:
  RunnerWidget(QWidget *parent = Q_NULLPTR);
  ~RunnerWidget();

protected:
  void showEvent(QShowEvent* event);

private:
  Ui::RunnerWidget       ui_;

  QByteArray             initial_splitter_state_;
  bool                   initial_splitter_state_valid_;   // required, as this widget is not shown when created, and thus the splitter does not have any size, yet

  RunnerTreeModel*       runner_tree_model_;

  QList<QModelIndex> getSelectedIndices();

  std::vector<std::shared_ptr<EcalSysRunner>> getSelectedRunners();

public slots:
  void runnersAdded   (const std::vector<std::shared_ptr<EcalSysRunner>>& runners);
  void runnersRemoved (const std::vector<std::shared_ptr<EcalSysRunner>>& runners);
  void runnersModified(const std::vector<std::shared_ptr<EcalSysRunner>>& runners);

  void resetLayout();

  void configChanged();

  void setEditRunner(std::shared_ptr<EcalSysRunner> runner);

private slots:
  void runnerTableSelectionChanged();

  void runnerTreeRowsInserted(const QModelIndex& parent, int start, int /*end*/);

  // Buttons
  void addButtonClicked();
  void removeButtonClicked();

  // Edit controls
  void nameEditingFinished();
  void pathEditingFinished();
  void pathButtonClicked();
  void defaultAlgoDirEditingFinished();
  void defaultAlgoDirButtonClicked();
  void commandLineEditingFinished();

signals:
  void runnersAddedSignal   (std::vector<std::shared_ptr<EcalSysRunner>> runners);
  void runnersRemovedSignal (std::vector<std::shared_ptr<EcalSysRunner>> runners);
  void runnersModifiedSignal(std::vector<std::shared_ptr<EcalSysRunner>> runners);

  void tasksModifiedSignal  (std::vector<std::shared_ptr<EcalSysTask>> tasks);
};
