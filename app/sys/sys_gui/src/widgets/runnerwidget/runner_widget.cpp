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

#include "runner_widget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>

#include "widgets/treemodels/tree_item_types.h"

#include "globals.h"

RunnerWidget::RunnerWidget(QWidget *parent)
  : QWidget(parent)
  , initial_splitter_state_valid_(false)
{
  ui_.setupUi(this);

  runner_tree_model_ = new RunnerTreeModel(this);
  runner_tree_model_->reload(Globals::EcalSysInstance()->GetRunnerList(), true);
  ui_.runner_tree->setModel(runner_tree_model_);

  // Hide all columns except the name column
  for (int col = 0; col < (int)RunnerTreeModel::Columns::COLUMN_COUNT; col++)
  {
    if (!(col == (int)RunnerTreeModel::Columns::NAME))
    {
      ui_.runner_tree->hideColumn(col);
    }
  }

  connect(ui_.runner_tree->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this,
    SLOT(runnerTableSelectionChanged()));
  connect(runner_tree_model_, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(runnerTreeRowsInserted(const QModelIndex&, int, int)));

  connect(ui_.runner_tree,              SIGNAL(keySequenceDeletePressed()), this, SLOT(removeButtonClicked()));

  // Buttons
  connect(ui_.add_button,                SIGNAL(clicked()),         this, SLOT(addButtonClicked()));
  connect(ui_.remove_button,             SIGNAL(clicked()),         this, SLOT(removeButtonClicked()));

  // Edit Controls
  connect(ui_.name_lineedit,             SIGNAL(editingFinished()), this, SLOT(nameEditingFinished()));
  connect(ui_.path_lineedit,             SIGNAL(editingFinished()), this, SLOT(pathEditingFinished()));
  connect(ui_.path_button,               SIGNAL(clicked()),         this, SLOT(pathButtonClicked()));
  connect(ui_.default_algo_dir_lineedit, SIGNAL(editingFinished()), this, SLOT(defaultAlgoDirEditingFinished()));
  connect(ui_.default_algo_dir_button,   SIGNAL(clicked()),         this, SLOT(defaultAlgoDirButtonClicked()));
  connect(ui_.command_line_lineedit,     SIGNAL(editingFinished()), this, SLOT(commandLineEditingFinished()));

  // Singals that something has changed
  connect(this, &RunnerWidget::runnersAddedSignal,
    [this](std::vector<std::shared_ptr<EcalSysRunner>> runner_vector)
      {
        for (auto& runner : runner_vector)
        {
          runner_tree_model_->addRunner(runner);
        }
      }
  );
  connect(this, &RunnerWidget::runnersRemovedSignal,
    [this](std::vector<std::shared_ptr<EcalSysRunner>> runner_vector)
      {
        for (auto& runner : runner_vector)
        {
          runner_tree_model_->removeRunner(runner);
        }
      }
  );
  connect(this, &RunnerWidget::runnersModifiedSignal,
    [this](std::vector<std::shared_ptr<EcalSysRunner>> runner_vector)
      {
        for (auto& runner : runner_vector)
        {
          runner_tree_model_->updateRunner(runner);
        }
      }
  );

  runnerTableSelectionChanged();

  initial_splitter_state_ = ui_.splitter->saveState();
}

RunnerWidget::~RunnerWidget()
{}

void RunnerWidget::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);

  // We have to actually show the splitter, in order to know its size. Thus, we have to save this value in the showevent.
  if (!initial_splitter_state_valid_)
  {
    initial_splitter_state_       = ui_.splitter->saveState();
    initial_splitter_state_valid_ = true;
  }
}

void RunnerWidget::resetLayout()
{
  if (initial_splitter_state_valid_)
  {
    ui_.splitter->restoreState(initial_splitter_state_);
  }
}

void RunnerWidget::configChanged()
{
  runner_tree_model_->reload();
}

void RunnerWidget::setEditRunner(std::shared_ptr<EcalSysRunner> runner)
{
  QModelIndex runner_index = runner_tree_model_->index(runner, (int)RunnerTreeModel::Columns::NAME);
  ui_.runner_tree->selectionModel()->select(runner_index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
  ui_.runner_tree->scrollTo(runner_index);
  ui_.runner_tree->setCurrentIndex(runner_index);
  ui_.runner_tree->setFocus();
}

void RunnerWidget::runnersAdded(const std::vector<std::shared_ptr<EcalSysRunner>>& runners)
{
  for (auto& runner : runners)
  {
    runner_tree_model_->addRunner(runner);
  }
}

void RunnerWidget::runnersRemoved(const std::vector<std::shared_ptr<EcalSysRunner>>& runners)
{
  for (auto& runner : runners)
  {
    runner_tree_model_->removeRunner(runner);
  }
}

void RunnerWidget::runnersModified(const std::vector<std::shared_ptr<EcalSysRunner>>& runners)
{
  for (auto& runner : runners)
  {
    runner_tree_model_->updateRunner(runner);
  }
}

void RunnerWidget::runnerTableSelectionChanged()
{
  ui_.name_lineedit            ->blockSignals(true);
  ui_.path_lineedit            ->blockSignals(true);
  ui_.default_algo_dir_lineedit->blockSignals(true);
  ui_.command_line_lineedit    ->blockSignals(true);

  auto selected_runners = getSelectedRunners();
  if (selected_runners.size() == 0 || !selected_runners[0])
  {
    ui_.remove_button            ->setEnabled(false);
    ui_.edit_controls_widget     ->setEnabled(false);
    ui_.name_lineedit            ->setText(QString(""));
    ui_.path_lineedit            ->setText(QString(""));
    ui_.default_algo_dir_lineedit->setText(QString(""));
    ui_.command_line_lineedit    ->setText(QString(""));
  }
  else
  {
    ui_.remove_button            ->setEnabled(true);
    ui_.edit_controls_widget     ->setEnabled(true);
    ui_.name_lineedit            ->setText(QString(selected_runners[0]->GetName()           .c_str()));
    ui_.path_lineedit            ->setText(QString(selected_runners[0]->GetPath()           .c_str()));
    ui_.default_algo_dir_lineedit->setText(QString(selected_runners[0]->GetDefaultAlgoDir() .c_str()));
    ui_.command_line_lineedit    ->setText(QString(selected_runners[0]->GetLoadCmdArgument().c_str()));
  }

  ui_.name_lineedit            ->blockSignals(false);
  ui_.path_lineedit            ->blockSignals(false);
  ui_.default_algo_dir_lineedit->blockSignals(false);
  ui_.command_line_lineedit    ->blockSignals(false);
}

void RunnerWidget::runnerTreeRowsInserted(const QModelIndex& parent, int start, int /*end*/)
{
  QModelIndex index = runner_tree_model_->index(start, (int)RunnerTreeModel::Columns::NAME, parent);
  ui_.runner_tree->scrollTo(index);
}

QList<QModelIndex> RunnerWidget::getSelectedIndices()
{
  return ui_.runner_tree->selectionModel()->selectedRows();
}

std::vector<std::shared_ptr<EcalSysRunner>> RunnerWidget::getSelectedRunners()
{
  auto selected_indices = getSelectedIndices();
  std::vector<std::shared_ptr<EcalSysRunner>> selected_runners;

  for (auto& index : selected_indices)
  {
    QAbstractTreeItem* item = runner_tree_model_->item(index);
    if (item && item->type() == (int)TreeItemType::Runner)
    {
      selected_runners.push_back(static_cast<RunnerTreeItem*>(item)->getRunner());
    }
  }
  return selected_runners;
}

void RunnerWidget::addButtonClicked()
{
  std::shared_ptr<EcalSysRunner> new_runner(new EcalSysRunner);
  Globals::EcalSysInstance()->AddRunner(new_runner);
  emit runnersAddedSignal(std::vector<std::shared_ptr<EcalSysRunner>>{new_runner});

  QModelIndex added_index = runner_tree_model_->index(new_runner, (int)RunnerTreeModel::Columns::NAME);
  ui_.runner_tree->selectionModel()->select(added_index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
  ui_.runner_tree->scrollTo(added_index);
  ui_.name_lineedit->setFocus();
}

void RunnerWidget::removeButtonClicked()
{
  auto selected_runners = getSelectedRunners();

  // Don't remove the "no runner" element
  auto no_runner_it = std::remove_if(selected_runners.begin(), selected_runners.end(), [](std::shared_ptr<EcalSysRunner> runner) {return !(runner); });
  if (no_runner_it != selected_runners.end())
  {
    selected_runners.erase(no_runner_it);
  }

  if (selected_runners.size() != 0)
  {
    // Check if the Runner is in use
    auto task_list = Globals::EcalSysInstance()->GetTaskList();
    QStringList tasks_that_use_the_runner;
    for (auto& task : task_list)
    {
      if (task->GetRunner() == selected_runners[0])
      {
        tasks_that_use_the_runner.push_back(QString(task->GetName().c_str()));
      }
    }

    bool expand_before_removal = false;
    if (!tasks_that_use_the_runner.isEmpty())
    {
      QMessageBox warning_message_box(this);
      warning_message_box.setIcon(QMessageBox::Icon::Warning);
      warning_message_box.setWindowTitle("Runner in use");
      warning_message_box.setText("The Runner \"" + QString(selected_runners[0]->GetName().c_str()) + "\" is currently used by the following tasks:<br>"
        + tasks_that_use_the_runner.join(", ")
        + "<br><br>Choose:"
        + "<br><b>Expand & Remove</b>: To copy the runner-data to the task before removal"
        + "<br><b>Remove</b>: To destructively remove the runner without preserving the data");

      QPushButton *expand_and_remove_button = warning_message_box.addButton(tr("Expand && Remove"), QMessageBox::ActionRole);
      /*QPushButton *remove_button          =*/ warning_message_box.addButton(tr("Remove"), QMessageBox::ActionRole);
      QPushButton *cancel_button            = warning_message_box.addButton(QMessageBox::Cancel);
      
      warning_message_box.exec();

      if (warning_message_box.clickedButton() == cancel_button)
      {
        return;
      }
      else
      {
        expand_before_removal = (warning_message_box.clickedButton() == expand_and_remove_button);
      }
    }

    auto affected_tasks = Globals::EcalSysInstance()->RemoveRunner(selected_runners[0], expand_before_removal);
    emit runnersRemovedSignal(std::vector<std::shared_ptr<EcalSysRunner>>{selected_runners[0]});

    // Notify about changed tasks
    if (affected_tasks.size() > 0)
    {
      std::vector<std::shared_ptr<EcalSysTask>> affected_tasks_vector;
      affected_tasks_vector.reserve(affected_tasks.size());
      for (auto& task : affected_tasks)
      {
        affected_tasks_vector.push_back(task);
      }
      emit tasksModifiedSignal(affected_tasks_vector);
    }
  }
}

void RunnerWidget::nameEditingFinished()
{
  auto selected_runners = getSelectedRunners();
  if (selected_runners.size() != 0)
  {
    QString text = ui_.name_lineedit->text();
    if (text.toStdString() != selected_runners.front()->GetName())
    {
      selected_runners[0]->SetName(text.toStdString());
      emit runnersModifiedSignal(std::vector<std::shared_ptr<EcalSysRunner>>{selected_runners[0]});
    }
  }
}

void RunnerWidget::pathEditingFinished()
{
  auto selected_runners = getSelectedRunners();
  if (selected_runners.size() != 0)
  {
    QString text = ui_.path_lineedit->text();
    if (text.toStdString() != selected_runners.front()->GetPath())
    {
      selected_runners[0]->SetPath(text.toStdString());
      // Conveniently also set the name if it hasn't been set, yet
      if (selected_runners[0]->GetName().empty())
      {
        QFileInfo file_info(text);
        QString algo_name = file_info.baseName();
        selected_runners[0]->SetName(algo_name.toStdString());
        
        ui_.name_lineedit->blockSignals(true);
        ui_.name_lineedit->setText(algo_name);
        ui_.name_lineedit->blockSignals(false);
      }
      emit runnersModifiedSignal(std::vector<std::shared_ptr<EcalSysRunner>>{selected_runners[0]});
    }
  }
}

void RunnerWidget::pathButtonClicked()
{
  auto selected_runners = getSelectedRunners();
  if (selected_runners.size() != 0)
  {
    QString start_path = "";
    QString current_path = QString(selected_runners[0]->GetPath().c_str());
    QString current_default_algo_dir = QString(selected_runners[0]->GetDefaultAlgoDir().c_str());
    if (!current_path.isEmpty())
    {
      start_path = current_path;
    }
    else if (!current_default_algo_dir.isEmpty())
    {
      start_path = current_default_algo_dir;
    }


    if (start_path == "")
    {
      QSettings settings;
      settings.beginGroup("runnerwidget");
      start_path = settings.value("last_runner_dir").toString();
      settings.endGroup();
    }
    
    QString new_path = QFileDialog::getOpenFileName(this, tr("Choose Runner Path"), start_path, tr("All Files (*)"));
    if (!new_path.isEmpty())
    {
#ifdef WIN32
      new_path.replace('/', '\\');
#endif // WIN32

      if (new_path.toStdString() != selected_runners.front()->GetPath())
      {
        ui_.path_lineedit->blockSignals(true);
        selected_runners[0]->SetPath(new_path.toStdString());
        ui_.path_lineedit->setText(new_path);
        ui_.path_lineedit->blockSignals(false);

        // Conveniently also set the name if it hasn't been set, yet
        if (selected_runners[0]->GetName().empty())
        {
          QFileInfo file_info(new_path);
          QString algo_name = file_info.baseName();
          selected_runners[0]->SetName(algo_name.toStdString());

          ui_.name_lineedit->blockSignals(true);
          ui_.name_lineedit->setText(algo_name);
          ui_.name_lineedit->blockSignals(false);
        }

        emit runnersModifiedSignal(std::vector<std::shared_ptr<EcalSysRunner>>{selected_runners[0]});

        QSettings settings;
        settings.beginGroup("runnerwidget");
        settings.setValue("last_runner_dir", QFileInfo(new_path).absoluteDir().absolutePath());
        settings.endGroup();
      }
    }
  }
}

void RunnerWidget::defaultAlgoDirEditingFinished()
{
  auto selected_runners = getSelectedRunners();
  if (selected_runners.size() != 0)
  {
    QString text = ui_.default_algo_dir_lineedit->text();
    if (text.toStdString() != selected_runners.front()->GetDefaultAlgoDir())
    {
      selected_runners[0]->SetDefaultAlgoDir(text.toStdString());
      emit runnersModifiedSignal(std::vector<std::shared_ptr<EcalSysRunner>>{selected_runners[0]});
    }
  }
}

void RunnerWidget::defaultAlgoDirButtonClicked()
{
  auto selected_runners = getSelectedRunners();
  if (selected_runners.size() != 0)
  {
    QString start_path = "";
    QString current_path = QString(selected_runners[0]->GetPath().c_str());
    QString current_default_algo_dir = QString(selected_runners[0]->GetDefaultAlgoDir().c_str());
    if (!current_default_algo_dir.isEmpty())
    {
      start_path = current_default_algo_dir;
    }
    else if (!current_path.isEmpty())
    {
      start_path = current_path;
    }

    if (start_path == "")
    {
      QSettings settings;
      settings.beginGroup("runnerwidget");
      start_path = settings.value("last_default_algo_dir").toString();
      settings.endGroup();
    }

    QString new_default_algo_dir = QFileDialog::getExistingDirectory(this, tr("Choose Default Algo Directory"), start_path);
    if (!new_default_algo_dir.isEmpty())
    {
#ifdef WIN32
      new_default_algo_dir.replace('/', '\\');
#endif // WIN32

      if (new_default_algo_dir.toStdString() != selected_runners.front()->GetDefaultAlgoDir())
      {
        ui_.default_algo_dir_lineedit->blockSignals(true);
        selected_runners[0]->SetDefaultAlgoDir(new_default_algo_dir.toStdString());
        ui_.default_algo_dir_lineedit->setText(new_default_algo_dir);
        ui_.default_algo_dir_lineedit->blockSignals(false);
        emit runnersModifiedSignal(std::vector<std::shared_ptr<EcalSysRunner>>{selected_runners[0]});
      }

      QSettings settings;
      settings.beginGroup("runnerwidget");
      settings.setValue("last_default_algo_dir", new_default_algo_dir);
      settings.endGroup();
    }
  }
}

void RunnerWidget::commandLineEditingFinished()
{
  auto selected_runners = getSelectedRunners();
  if (selected_runners.size() != 0)
  {
    QString text = ui_.command_line_lineedit->text();
    if (text.toStdString() != selected_runners.front()->GetLoadCmdArgument())
    {
      selected_runners[0]->SetLoadCmdArgument(text.toStdString());
      emit runnersModifiedSignal(std::vector<std::shared_ptr<EcalSysRunner>>{selected_runners[0]});
    }
  }
}
