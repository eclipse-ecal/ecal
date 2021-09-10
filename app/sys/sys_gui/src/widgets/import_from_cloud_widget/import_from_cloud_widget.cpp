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

#include "import_from_cloud_widget.h"

#include <QMenu>
#include <QAction>

#include <regex>

#include "ecal/ecal.h"

#include "globals.h"

#include "ecalsys_settings.h"
#include "widgets/treemodels/tree_data_roles.h"
#include "widgets/treemodels/tree_item_types.h"
#include "widgets/treemodels/task_tree_item.h"

#include <sys_client_core/task.h>
#include <ecalsys/task/ecal_sys_task_helper.h>

#include <ecal_utils/filesystem.h>
#include <ecal_utils/string.h>
#include <ecal_utils/ecal_utils.h>

ImportFromCloudWidget::ImportFromCloudWidget(QWidget *parent)
  : QWidget(parent)
  , initial_splitter_state_valid_(false)
{
  ui_.setupUi(this);

  // Load the exclude list
  loadExcludeTasksFilter();

  // Model for the task-tree
  task_tree_model_              = new ImportFromCloudTaskTreeModel(this);
  // create the custom header with a checkbox
  task_tree_header_ = new QCheckboxHeaderView(ui_.tasks_tree);
  task_tree_header_->setCheckable((int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED);
  task_tree_header_->setChecked((int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED, true);
  task_tree_header_->setStretchLastSection(true);
  ui_.tasks_tree->setHeader(task_tree_header_);

  task_sort_filter_proxy_model_ = new QStableSortFilterProxyModel(this);
  task_sort_filter_proxy_model_->setSourceModel(task_tree_model_);
  task_sort_filter_proxy_model_->setFilterRole(ItemDataRoles::FilterRole);
  task_sort_filter_proxy_model_->setFilterKeyColumn((int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED);
  task_sort_filter_proxy_model_->setSortRole(ItemDataRoles::SortRole);
  task_sort_filter_proxy_model_->setDynamicSortFilter(false);
  task_sort_filter_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

  updateImportCheckboxEnabledStates();

  ui_.tasks_tree->setModel(task_sort_filter_proxy_model_);

  // only show some colums in the task-tree
  static std::vector<ImportFromCloudTaskTreeModel::Columns> intial_task_colum_selection =
  {
    ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED,
    ImportFromCloudTaskTreeModel::Columns::TASK_NAME,
    ImportFromCloudTaskTreeModel::Columns::CURRENT_PID,
    ImportFromCloudTaskTreeModel::Columns::CURRENT_HOST,
    ImportFromCloudTaskTreeModel::Columns::RUNNER_NAME,
    ImportFromCloudTaskTreeModel::Columns::PATH,
    ImportFromCloudTaskTreeModel::Columns::COMMAND_LINE_ARGS,
    ImportFromCloudTaskTreeModel::Columns::STATE,
    ImportFromCloudTaskTreeModel::Columns::INFO
  };
  for (int col = 0; col < (int)ImportFromCloudTaskTreeModel::Columns::COLUMN_COUNT; col++)
  {
    if (std::find(intial_task_colum_selection.begin(), intial_task_colum_selection.end(), (ImportFromCloudTaskTreeModel::Columns)col) == intial_task_colum_selection.end())
    {
      ui_.tasks_tree->hideColumn(col);
    }
  }
  // Prevent the user from hiding the name and checkbox column
  ui_.tasks_tree->setForcedColumns(QVector<int>{(int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED, (int)ImportFromCloudTaskTreeModel::Columns::TASK_NAME});

  ui_.tasks_tree->resizeColumnToContents((int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED);
  ui_.tasks_tree->sortByColumn((int)ImportFromCloudTaskTreeModel::Columns::TASK_NAME, Qt::SortOrder::AscendingOrder);


  // Model for the runner-tree
  runner_tree_model_  = new ImportFromCloudRunnerTreeModel(this);
  runner_tree_model_->reload(std::list<std::shared_ptr<EcalSysRunner>>(), false);
  runner_tree_header_ = new QCheckboxHeaderView(this);
  runner_tree_header_->setCheckable((int)ImportFromCloudRunnerTreeModel::Columns::IMPORT_CHECKED);
  runner_tree_header_->setCheckboxEnabled((int)ImportFromCloudRunnerTreeModel::Columns::IMPORT_CHECKED, false);
  runner_tree_header_->setChecked((int)ImportFromCloudRunnerTreeModel::Columns::IMPORT_CHECKED, true);
  runner_tree_header_->setStretchLastSection(true);
  ui_.runner_tree->setHeader(runner_tree_header_);

  runner_sort_filter_proxy_model_ = new QStableSortFilterProxyModel(this);
  runner_sort_filter_proxy_model_->setSourceModel(runner_tree_model_);
  runner_sort_filter_proxy_model_->setFilterRole(ItemDataRoles::FilterRole);
  runner_sort_filter_proxy_model_->setFilterKeyColumn((int)ImportFromCloudRunnerTreeModel::Columns::IMPORT_CHECKED);
  runner_sort_filter_proxy_model_->setSortRole(ItemDataRoles::SortRole);
  runner_sort_filter_proxy_model_->setDynamicSortFilter(false);
  runner_sort_filter_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

  ui_.runner_tree->setModel(runner_sort_filter_proxy_model_);

  // Prevent the user from hiding the name and checkbox column
  ui_.runner_tree->setForcedColumns(QVector<int>{(int)ImportFromCloudRunnerTreeModel::Columns::IMPORT_CHECKED, (int)ImportFromCloudRunnerTreeModel::Columns::NAME});
  ui_.runner_tree->resizeColumnToContents((int)ImportFromCloudRunnerTreeModel::Columns::IMPORT_CHECKED);

  connect(ui_.runner_tree, &QAdvancedTreeView::keySequenceDeletePressed, [this]() {removeRunners(getSelectedRunners()); });


  // connect the buttons
  connect(ui_.auto_detect_runners_button, &QPushButton::clicked, [this]() {autoDetectRunnersFor(getCheckedTasks()); });
  connect(ui_.cancel_button,              SIGNAL(clicked()), this, SIGNAL(closeSignal()));
  connect(ui_.import_button,              SIGNAL(clicked()), this, SLOT(import()));
  connect(ui_.show_all_tasks_checkbox,    &QCheckBox::toggled, [this](bool checked) { task_sort_filter_proxy_model_->setFilterRegExp(checked ? "" : "enabled"); });

  // connect the task-tree
  ui_.tasks_tree->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.tasks_tree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(taskTreeContextMenu(const QPoint&)));

  connect(task_tree_model_, &QAbstractItemModel::dataChanged,
    [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
    {
      // We always emit the dataChanged Signal for each entry, so the row of topLeft and bottomRight are always the same.

      if ((topLeft.column() <= (int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED)
        && (bottomRight.column() >= (int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED)
        && (std::find(roles.begin(), roles.end(), Qt::ItemDataRole::CheckStateRole) != roles.end()))
      {
        updateButtons();
        updateTaskTableHeaderCheckbox();
      }
    }
  );
  //connect(task_tree_model_, SIGNAL(checkboxStateChanged(std::shared_ptr<EcalSysTask>, bool)), this, SLOT(updateButtons()));
  //connect(task_tree_model_, SIGNAL(checkboxStateChanged(std::shared_ptr<EcalSysTask>, bool)), this, SLOT(updateTaskTableHeaderCheckbox()));
  connect(task_tree_header_, &QCheckboxHeaderView::clicked, 
      [this](int column, bool checked)
      {
        for (int i = 0; i < task_tree_model_->rowCount(); i++)
        {
          bool checkbox_enabled = (task_tree_model_->flags(task_tree_model_->index(i, column)) & Qt::ItemFlag::ItemIsEnabled);
          if (checkbox_enabled)
          {
            task_tree_model_->setData(task_tree_model_->index(i, column), (checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked), Qt::ItemDataRole::CheckStateRole);
          }
        }
      }
  );

  // connect the runner-tree
  ui_.runner_tree->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.runner_tree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(runnerTreeContextMenu(const QPoint&)));
  connect(runner_tree_model_, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(updateButtons()));
  connect(runner_tree_model_, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(updateButtons()));

  initial_task_tree_state_   = ui_.tasks_tree->saveState();
  initial_runner_tree_state_ = ui_.runner_tree->saveState();
}

ImportFromCloudWidget::~ImportFromCloudWidget()
{
}

void ImportFromCloudWidget::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);

  // We have to actually show the splitter, in order to know its size. Thus, we have to save this value in the showevent.
  if (!initial_splitter_state_valid_)
  {
    initial_splitter_state_ = ui_.splitter->saveState();
    initial_splitter_state_valid_ = true;
  }
}

void ImportFromCloudWidget::resetLayout()
{
  ui_.tasks_tree ->restoreState(initial_task_tree_state_);
  ui_.runner_tree->restoreState(initial_runner_tree_state_);

  if (initial_splitter_state_valid_)
  {
    ui_.splitter->restoreState(initial_splitter_state_);
  }
}

void ImportFromCloudWidget::clear()
{
  task_list_.clear();
  runner_list_.clear();

  task_tree_model_->reload(task_list_);
  runner_tree_model_->reload(runner_list_, false);

  updateImportCheckboxEnabledStates();
  updateButtons();
  updateTaskTableHeaderCheckbox();
}

void ImportFromCloudWidget::reload()
{
  clear();

  // Load the task list
  task_list_ = Globals::EcalSysInstance()->GetTasksFromCloud();

  // "repair" the start at severity states
  for (auto& task : task_list_)
  {
    TaskState restart_at_severity;
    restart_at_severity.severity = eCAL_Process_eSeverity::proc_sev_failed;
    restart_at_severity.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level1;
    task->SetRestartAtSeverity(restart_at_severity);
  }

  task_tree_model_->reload(task_list_);
  runner_tree_model_->reload(runner_list_, false);

  // Update the GUI manually
  updateImportCheckboxEnabledStates();
  updateButtons();
  updateTaskTableHeaderCheckbox();

  // Re-apply the filter
  task_sort_filter_proxy_model_->setFilterRegExp(task_sort_filter_proxy_model_->filterRegExp());
}

void ImportFromCloudWidget::setUpdateEnabled(bool enabled)
{
  update_enabled_ = enabled;
}

void ImportFromCloudWidget::autoDetectRunnersFor(const std::list<std::shared_ptr<EcalSysTask>>& task_list, bool force_create)
{
  std::map<std::string, std::list<std::shared_ptr<EcalSysTask>>> common_process_path_map;

  std::list<std::shared_ptr<EcalSysTask>> filtered_tasks_list;
  for (auto& task : task_list)
  {
    // Only detect runners for tasks that don't already have one!
    if (!task->GetRunner())
    {
      filtered_tasks_list.push_back(task);
    }
  }

  // Group all tasks by their process path. If two Tasks run off the same executable this is a candidate for a runner!
  for (auto& task : filtered_tasks_list)
  {
    common_process_path_map[task->GetAlgoPath()].push_back(task);
  }

  for (auto path_task_mapping : common_process_path_map)
  {
    std::string path = path_task_mapping.first;
    std::list<std::shared_ptr<EcalSysTask>> tasks = path_task_mapping.second;

    // We only auto-create runners for paths that appear multiple times (or for all tasks, if the force_create option has been set)
    if (tasks.size() > 1 || force_create)
    {
      // If no task has any parameter, it is not really necessary to create a
      // a runner. We assume that the task is just a process that the user has
      // started multiple times.
      bool any_process_has_parameters = false;
      for (auto& task : tasks)
      {
        if (task->GetCommandLineArguments() != "")
        {
          any_process_has_parameters = true;
          break;
        }
      }
      if (!any_process_has_parameters && !force_create)
      {
        break;
      }

      // Create a new Runner with that path
      std::shared_ptr<EcalSysRunner> new_runner(new EcalSysRunner());
      new_runner->SetName(EcalUtils::Filesystem::BaseName(path));
      new_runner->SetPath(path);

      // Until now, everything was quite deterministic. Now we have to detect
      // whether the runner has a command-line-argument for loading algos and
      // potentially separate the process's command line into:
      //   1. The runner load command argument
      //   2. The algo path
      //   3. The additional command line arguments of the task
      //
      // We cannot automatically decide that separation with absolute certainty.
      // Thus, we use a heuristic here, that should work in many cases. Our
      // assumptions are:
      //   1. The runner-load-argument:
      //        a) starts with a dash (-)
      //        b) appears on all processes as first argument
      //        c) is always followed by an algo-path (criteria: see below)
      //
      //   2. The algo-path:
      //        a) never starts with a dash
      //        b) may or may not be enclosed in single-quotes (') or
      //           double-quotes (")
      //        c) may be preceeded by a runner-load-argument, but can also just
      //           stand for itself on the first position
      //        d) If a runner-load-argument is present, the algo-path is
      //           separated by a space
      // 
      //   3. The additional command line:
      //        a) is everything after the algo-path, if an algo-path is found
      //        b) defaults to the whole command line, if no algo-path is found 


      bool        reference_first_arg_is_algo_path = false;
      bool        reference_second_arg_is_algo_path = false;
      std::string reference_runner_load_arg = "";
      bool        any_task_disagreeing = false;

      for (auto it = tasks.begin(); it != tasks.end(); ++it)
      {
        std::string complete_command_line = (*it)->GetCommandLineArguments();
        complete_command_line = EcalUtils::String::Trim(complete_command_line);

        if (it == tasks.begin())
        {
          // Parse the first task as reference. We will compare any other task against that one.
          attemptRunnerCommandLineSeparation((*it)->GetCommandLineArguments(), reference_first_arg_is_algo_path, reference_second_arg_is_algo_path, reference_runner_load_arg);
        }
        else
        {
          bool        first_arg_is_algo_path = false;
          bool        second_arg_is_algo_path = false;
          std::string runner_load_arg = "";

          attemptRunnerCommandLineSeparation((*it)->GetCommandLineArguments(), first_arg_is_algo_path, second_arg_is_algo_path, runner_load_arg);

          // Commpare the task against the reference task. If it gave us other results, we will not create a runner.
          if (!(reference_first_arg_is_algo_path == first_arg_is_algo_path
            && reference_second_arg_is_algo_path == second_arg_is_algo_path
            && reference_runner_load_arg == runner_load_arg))
          {
            any_task_disagreeing = true;
            break;
          }
        }
      }

      if (!any_task_disagreeing)
      {
        // If no task is disagreeing with the reference task, we use the reference data.

        if (reference_second_arg_is_algo_path)
        {
          // If the second argument is an algo path, we defintively have an runner_load_arg.
          new_runner->SetLoadCmdArgument(reference_runner_load_arg);
        }

        // Modify all Tasks
        for (auto& task : tasks)
        {
          if (reference_first_arg_is_algo_path)
          {
            // Split into (algo_path + rest)
            auto task_args = EcalUtils::CommandLine::splitCommandLine(task->GetCommandLineArguments(), 2);
            task->SetAlgoPath(task_args[0]);
            task->SetCommandLineArguments(task_args.size() >= 2 ? task_args[1] : "");
          }
          else if (reference_second_arg_is_algo_path)
          {
            // Split into (load_arg + algo_path + rest)
            auto task_args = EcalUtils::CommandLine::splitCommandLine(task->GetCommandLineArguments(), 3);
            task->SetAlgoPath(task_args[1]);
            task->SetCommandLineArguments(task_args.size() >= 3 ? task_args[2] : "");
          }
          else // We don't have an algo path
          {
            // Don't split, everything will be 'rest'
            task->SetAlgoPath("");
          }
        }

      }
      else
      {
        // If any task is disagreeing with the reference task, we have no other choice than interpreting everything as an argument. (i.e. there is no algo path and no load argument)
        for (auto& task : tasks)
        {
          task->SetAlgoPath("");
        }
      }

      // Check if we already have an existing runner with that exact configuration and reuse it if possible
      bool existing_runner_found = false;
      for (auto& existing_runner : runner_list_)
      {
        if (existing_runner->GetPath() == new_runner->GetPath()
          && existing_runner->GetLoadCmdArgument() == new_runner->GetLoadCmdArgument())
        {
          for (auto& task : tasks)
          {
            // Use the existing runner...
            task->SetRunner(existing_runner);
            // ... and update the task in the treeview.
            task_tree_model_->updateTask(task);
          }
          existing_runner_found = true;
          break;
        }
      }

      // Only assign the new runner, if we didn't reuse an existing one
      if (!existing_runner_found)
      {
        // Rename the new runner, if a runner with that exact name already exists
        if (std::find_if(runner_list_.begin(), runner_list_.end(),
          [new_runner](auto& existing_runner) {return (new_runner->GetName() == existing_runner->GetName()); })
          != runner_list_.end())
        {
          int counter = 2;
          std::string new_name;
          do
          {
            new_name = new_runner->GetName() + " (" + std::to_string(counter) + ")";
            counter++;
          } while (std::find_if(runner_list_.begin(), runner_list_.end(),
            [&new_name](auto& existing_runner) {return (new_name == existing_runner->GetName()); })
            != runner_list_.end());

          new_runner->SetName(new_name);
        }

        // Add the new runner
        runner_list_.push_back(new_runner);
        runner_tree_model_->addRunner(new_runner);

        for (auto& task : tasks)
        {
          // Use the new runner...
          task->SetRunner(new_runner);
          // ... and update the task in the treeview.
          task_tree_model_->updateTask(task);
        }
      }
    }
  }
}

void ImportFromCloudWidget::attemptRunnerCommandLineSeparation(
  const std::string& input_command_line,
  bool&              first_arg_is_algo_path,
  bool&              second_arg_is_algo_path,
  std::string&       runner_load_arg)
{
  // Let's start by splitting the command line of the task.
  // We can have up to 3 parts here (load_arg + algo_path + rest)
  auto arguments = EcalUtils::CommandLine::splitCommandLine(input_command_line, 3);

  if (arguments.size() == 0)
  {
    // We don't have any arguments!
  }
  else if (arguments.size() == 1)
  {
    // Only one argument means that we cannot have a load arguments, as that must be followed by the algo path. We can however have an algo path as first argument.
    if (arguments[0].at(0) != '-')
    {
      first_arg_is_algo_path = true;
    }
  }
  else // (arguments.size() >= 2)
  {
    if ((arguments[0].at(0) == '-') && (arguments[1].at(0) != '-'))
    {
      // When having 2 or 3 parts, we may have a load argument at the first place
      second_arg_is_algo_path = true;
      runner_load_arg = arguments[0];
    }
    else if (arguments[0].at(0) != '-')
    {
      // Or we have the algo path at the first place
      first_arg_is_algo_path = true;
    }
  }
}

void ImportFromCloudWidget::removeRunnerAssignment(const std::list<std::shared_ptr<EcalSysTask>>& task_list)
{
  for (const auto& task : task_list)
  {
    // Convert into a primitive sys_client task
    eCAL::sys_client::Task temp_task = eCAL::sys::task_helpers::ToSysClientTask_NoLock(task);

    // Merge the runner into the task, just as we would start the task.
    // The only differences are:
    //   1. We haven't evaluated the eCAL Parser functions
    //   2. We have no idea of the targets OS
    // 
    // This may lead to some errors, e.g. if an eCAL Parser function is used
    // that prevents the detection whether the algo path is relative or
    // absolut. The user will have to deal with that.

    eCAL::sys_client::MergeRunnerIntoTask(temp_task, EcalUtils::Filesystem::OsStyle::Combined);

    // Copy the values back to the internal task
    task->SetAlgoPath(temp_task.path);
    task->SetCommandLineArguments(temp_task.arguments);
    task->SetWorkingDir(temp_task.working_dir);

    // Remove the runner
    task->SetRunner(std::shared_ptr<EcalSysRunner>());
  }
}

void ImportFromCloudWidget::removeRunnerAssignment(const std::list<std::shared_ptr<EcalSysRunner>>& runner_list)
{
  std::list<std::shared_ptr<EcalSysTask>> affected_tasks;
  for (auto& task : task_list_)
  {
    if (std::find(runner_list.begin(), runner_list.end(), task->GetRunner()) != runner_list.end())
    {
      affected_tasks.push_back(task);
    }
  }
  removeRunnerAssignment(affected_tasks);
}

void ImportFromCloudWidget::removeRunners(const std::list<std::shared_ptr<EcalSysRunner>>& runner_list)
{
  removeRunnerAssignment(runner_list);
  for (auto& runner : runner_list)
  {
    runner_list_.remove(runner);
    runner_tree_model_->removeRunner(runner);
  }
}

void ImportFromCloudWidget::autoAssignRunners(const std::list<std::shared_ptr<EcalSysRunner>>& runner_list)
{
  for (auto& runner : runner_list)
  {
    for (auto& task : task_list_)
    {
      if (!task->GetRunner() && (task->GetAlgoPath() == runner->GetPath()))
      {
        if (runner->GetLoadCmdArgument() != "")
        {
          // We need to look for 2-3 arguments (load_arg + algo_path + ?rest?)
          auto task_args = EcalUtils::CommandLine::splitCommandLine(task->GetCommandLineArguments(), 3);
          if ((task_args.size() >= 2)
            && (task_args[0] == runner->GetLoadCmdArgument()) // The first argument must be the load argument
            && (task_args[1].at(0) != '-'))                   // The second argument must not start with a dash, as this is an algo path
          {
            task->SetRunner(runner);
            task->SetAlgoPath(task_args[1]);
            if (task_args.size() >= 3)
            {
              task->SetCommandLineArguments(task_args[2]);
            }
            else
            {
              task->SetCommandLineArguments("");
            }
            task_tree_model_->updateTask(task);
          }
        }
        else
        {
          // We can have 0-2 arguments (?algo_path? + ?rest?).
          // But in any case, the runner matches (due to the same process path).
          // It's now just a case of checking whether we have an algo path.
          auto task_args = EcalUtils::CommandLine::splitCommandLine(task->GetCommandLineArguments(), 2);
          if (task_args.size() >= 1)
          {
            if (task_args[0].at(0) != '-')
            {
              // We have an algo path
              task->SetAlgoPath(task_args[0]);
              if (task_args.size() >= 2)
              {
                task->SetCommandLineArguments(task_args[1]);
              }
              else
              {
                task->SetCommandLineArguments("");
              }
            }
            else
            {
              // We don't have an algo path and thus leave the command line as it is
              task->SetAlgoPath("");
            }
          }
          else
          {
            // We don't have any arguments ant thus leave the command line as it is
            task->SetAlgoPath("");
          }
          task->SetRunner(runner);
          task_tree_model_->updateTask(task);
        }
      }
    }
  }
}

std::list<std::shared_ptr<EcalSysTask>> ImportFromCloudWidget::getSelectedTasks()
{
  auto selected_proxy_indices = ui_.tasks_tree->selectionModel()->selectedRows();
  std::list <std::shared_ptr<EcalSysTask>> selected_tasks;
  for (auto& proxy_index : selected_proxy_indices)
  {
    QAbstractTreeItem* tree_item = task_tree_model_->item(task_sort_filter_proxy_model_->mapToSource(proxy_index));
    if (tree_item && tree_item->type() == (int)TreeItemType::Task)
    {
      selected_tasks.push_back(static_cast<TaskTreeItem*>(tree_item)->getTask());
    }
  }
  return selected_tasks;
}

std::list<std::shared_ptr<EcalSysRunner>> ImportFromCloudWidget::getSelectedRunners()
{
  auto selected_proxy_indices = ui_.runner_tree->selectionModel()->selectedRows();
  std::list <std::shared_ptr<EcalSysRunner>> selected_runners;
  for (auto& proxy_index : selected_proxy_indices)
  {
    QAbstractTreeItem* tree_item = runner_tree_model_->item(runner_sort_filter_proxy_model_->mapToSource(proxy_index));
    if (tree_item && tree_item->type() == (int)TreeItemType::Runner)
    {
      selected_runners.push_back(static_cast<RunnerTreeItem*>(tree_item)->getRunner());
    }
  }
  return selected_runners;
}

std::list<std::shared_ptr<EcalSysTask>> ImportFromCloudWidget::getCheckedTasks()
{
  std::list<std::shared_ptr<EcalSysTask>> task_list;

  for (int i = 0; i < task_tree_model_->rowCount(); i++)
  {
    QAbstractTreeItem* tree_item = task_tree_model_->item(task_tree_model_->index(i, 0));
    if (tree_item
      && (tree_item->type() == (int)TreeItemType::Task)
      && (static_cast<TaskTreeItem*>(tree_item)->isImportChecked()))
    {
      task_list.push_back(static_cast<TaskTreeItem*>(tree_item)->getTask());
    }
  }
  return task_list;
}

std::list<std::shared_ptr<EcalSysTask>> ImportFromCloudWidget::getCheckableTasks()
{
  std::list<std::shared_ptr<EcalSysTask>> task_list;

  for (int i = 0; i < task_tree_model_->rowCount(); i++)
  {
    QAbstractTreeItem* tree_item = task_tree_model_->item(task_tree_model_->index(i, 0));
    if (tree_item
      && (tree_item->type() == (int)TreeItemType::Task)
      && (static_cast<TaskTreeItem*>(tree_item)->isImportCheckboxEnabled()))
    {
      task_list.push_back(static_cast<TaskTreeItem*>(tree_item)->getTask());
    }
  }
  return task_list;
}

void ImportFromCloudWidget::taskTreeContextMenu(const QPoint& pos)
{
  auto selected_tasks = getSelectedTasks();

  if (selected_tasks.size() > 0)
  {
    // Create lists of tasks with and without runners;
    std::list<std::shared_ptr<EcalSysTask>> tasks_with_runners;
    std::list<std::shared_ptr<EcalSysTask>> tasks_without_runners;
    for (auto& task : selected_tasks)
    {
      if (task->GetRunner())
      {
        tasks_with_runners.push_back(task);
      }
      else
      {
        tasks_without_runners.push_back(task);
      }
    }
  
    QMenu context_menu(tr("Context menu"), this);

    if (tasks_without_runners.size() > 0)
    {
      QAction* autodetect_runners_action = new QAction((tasks_without_runners.size() == 1 ? tr("Create runner") : tr("Detect runners for selection")), &context_menu);
      connect(autodetect_runners_action, &QAction::triggered, [this, tasks_without_runners]() {autoDetectRunnersFor(tasks_without_runners, tasks_without_runners.size() == 1); });
      context_menu.addAction(autodetect_runners_action);
    }
    if (tasks_with_runners.size() > 0)
    {
      QAction* unassign_runner_action = new QAction((tasks_with_runners.size() == 1 ? tr("Remove runner assignment") : tr("Remove runner assignments")), &context_menu);
      connect(unassign_runner_action, &QAction::triggered, [this, tasks_with_runners]() {removeRunnerAssignment(tasks_with_runners); });
      context_menu.addAction(unassign_runner_action);
    }

    context_menu.exec(ui_.tasks_tree->viewport()->mapToGlobal(pos));
  }
}

void ImportFromCloudWidget::runnerTreeContextMenu(const QPoint& pos)
{
  auto selected_runners = getSelectedRunners();

  if (selected_runners.size() > 0)
  {
    QMenu context_menu(tr("Context menu"), this);

    QAction* auto_assign_action = new QAction(tr("Auto-assign to possible tasks"), &context_menu);
    connect(auto_assign_action, &QAction::triggered, [this, selected_runners]() { autoAssignRunners(selected_runners); });
    context_menu.addAction(auto_assign_action);

    QAction* unassign_from_all_tasks_action = new QAction(tr("Remove assignment from all Tasks"), &context_menu);
    connect(unassign_from_all_tasks_action, &QAction::triggered, [this, selected_runners]() { removeRunnerAssignment(selected_runners); });
    context_menu.addAction(unassign_from_all_tasks_action);

    QAction* remove_runner_action = new QAction((selected_runners.size() == 1 ? tr("Remove runner") : tr("Remove runners")), &context_menu);
    connect(remove_runner_action, &QAction::triggered, [this, selected_runners]() {removeRunners(selected_runners); });
    context_menu.addAction(remove_runner_action);

    context_menu.exec(ui_.runner_tree->viewport()->mapToGlobal(pos));
  }

}

void ImportFromCloudWidget::monitorUpdated()
{
  // Most of the time, the import from cloud window is hidden. Therefore, we don't need to update it.
  if (!update_enabled_)
    return;

  // Update Tasks that are already existing
  Globals::EcalSysInstance()->UpdateTaskStates(task_list_);
  task_tree_model_->updateAll();

  // Check if there are new Tasks
  std::list<std::shared_ptr<EcalSysTask>> all_tasks_from_cloud = Globals::EcalSysInstance()->GetTasksFromCloud();
  std::list<std::shared_ptr<EcalSysTask>> new_tasks_from_cloud;

  for (auto& task_from_cloud : all_tasks_from_cloud)
  {
    if (std::find_if(task_list_.begin(), task_list_.end(),
        [task_from_cloud](const std::shared_ptr<EcalSysTask>& task_from_task_list)
        {
          return (task_from_cloud->GetHostStartedOn() == task_from_task_list->GetHostStartedOn())
            && (task_from_cloud->GetPids().front() == task_from_task_list->GetPids().front());
        })
      == task_list_.end())
    {
      // Someone started a new Task! We've got to add it!
      task_list_.push_back(task_from_cloud);
      task_tree_model_->addTask(task_from_cloud);
    }
  }

  updateImportCheckboxEnabledStates();
  updateTaskTableHeaderCheckbox();
  updateButtons();
  task_sort_filter_proxy_model_->setFilterRegExp(task_sort_filter_proxy_model_->filterRegExp());
}

void ImportFromCloudWidget::tasksChanged(std::vector<std::shared_ptr<EcalSysTask>>)
{
  updateImportCheckboxEnabledStates();
  updateTaskTableHeaderCheckbox();
  updateButtons();
  task_sort_filter_proxy_model_->setFilterRegExp(task_sort_filter_proxy_model_->filterRegExp());
}

void ImportFromCloudWidget::ecalsysOptionsChanged()
{
  task_tree_model_->updateAll();
}

void ImportFromCloudWidget::configChanged()
{
  updateImportCheckboxEnabledStates();
  updateTaskTableHeaderCheckbox();
  updateButtons();
  task_sort_filter_proxy_model_->setFilterRegExp(task_sort_filter_proxy_model_->filterRegExp());
}

void ImportFromCloudWidget::updateButtons()
{
  // Text of import buttons
  size_t number_of_tasks = getCheckedTasks().size();
  int number_of_runners = runner_tree_model_->rowCount(); // We force the user to always import all runners, as we otherwise might end up with inconsistent Tasks using a runner that wasn't imported
  ui_.import_button->setText("Import (" + QString::number(number_of_tasks) + " Tasks, " + QString::number(number_of_runners) + " Runners)");

  // Only enable the import button if there is anything to import
  ui_.import_button->setEnabled((number_of_tasks > 0) || (number_of_runners > 0));

  // enable / disable auto-detect runners button
  ui_.auto_detect_runners_button->setEnabled(number_of_tasks != 0);
}

void ImportFromCloudWidget::updateTaskTableHeaderCheckbox()
{
  size_t number_of_checkable_tasks = getCheckableTasks().size();
  size_t number_of_checked_tasks = getCheckedTasks().size();

  if ((number_of_checkable_tasks == 0) && (number_of_checked_tasks == 0))
  {} // DO nothing
  else if (number_of_checkable_tasks == number_of_checked_tasks)
    task_tree_header_->setCheckState((int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED, Qt::CheckState::Checked);
  else if (number_of_checked_tasks == 0)
    task_tree_header_->setCheckState((int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED, Qt::CheckState::Unchecked);
  else
    task_tree_header_->setCheckState((int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED, Qt::CheckState::PartiallyChecked);
}

void ImportFromCloudWidget::updateImportCheckboxEnabledStates()
{
  auto already_existing_tasks = Globals::EcalSysInstance()->GetTaskList();

  for (int i = 0; i < task_tree_model_->rowCount(); i++)
  {
    QModelIndex task_tree_item_checkbox_index = task_tree_model_->index(i, (int)ImportFromCloudTaskTreeModel::Columns::IMPORT_CHECKED);

    QAbstractTreeItem* tree_item = task_tree_model_->item(task_tree_item_checkbox_index);
    if (tree_item && tree_item->type() == (int)TreeItemType::Task)
    {
      auto task = static_cast<TaskTreeItem*>(tree_item)->getTask();

      if (exclude_tasks_regex_valid_ && std::regex_search(task->GetName(), exclude_tasks_regex_))
      {
        // disable tasks that are in the exclude list
        task_tree_model_->setData(task_tree_item_checkbox_index, Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole);
        static_cast<TaskTreeItem*>(tree_item)->setImportCheckboxEnabled(false);
        static_cast<TaskTreeItem*>(tree_item)->setImportDisabledReason(tr("Import forbidden by the eCAL settings"));
      }
      else
      {
        // disable tasks that are already in the task list
        auto existing_duplicate_task = std::find_if(already_existing_tasks.begin(), already_existing_tasks.end(), 
            [task](const std::shared_ptr<EcalSysTask>& existing_task)
            {
              std::string existing_task_host = existing_task->GetHostStartedOn();
              std::vector<int> existing_task_pids = existing_task->GetPids();

              return (task->GetHostStartedOn() == existing_task->GetHostStartedOn())
                && (std::find(existing_task_pids.begin(), existing_task_pids.end(), task->GetPids().front()) != existing_task_pids.end());
            }
        );
        if (existing_duplicate_task != already_existing_tasks.end())
        {
          task_tree_model_->setData(task_tree_item_checkbox_index, Qt::CheckState::Unchecked, Qt::ItemDataRole::CheckStateRole);
          static_cast<TaskTreeItem*>(tree_item)->setImportCheckboxEnabled(false);
          static_cast<TaskTreeItem*>(tree_item)->setImportDisabledReason(tr("Already existing in the task list"));
        }
        else
        {
          static_cast<TaskTreeItem*>(tree_item)->setImportCheckboxEnabled(true);
          static_cast<TaskTreeItem*>(tree_item)->setImportDisabledReason(tr(""));
        }
      }
      task_tree_model_->updateItem(tree_item);
    }
  }
}

void ImportFromCloudWidget::import()
{
  auto checked_tasks = getCheckedTasks();

  // Import Tasks
  std::vector<std::shared_ptr<EcalSysTask>> added_tasks_vector;
  added_tasks_vector.reserve(checked_tasks.size());
  for (auto& task : checked_tasks)
  {
    // Remove quotes
    std::string task_path = task->GetAlgoPath();
    if (task_path.size() > 2
      && ((task_path.at(0) == '\"' && task_path.at(task_path.size() - 1) == '\"')
      || (task_path.at(0) == '\'' && task_path.at(task_path.size() - 1) == '\'')))
    {
      task->SetAlgoPath(task_path.substr(1, task_path.size() - 2));
    }

    // Reset the modified property
    task->ResetConfigModifiedSinceStart();

    // Add Task
    added_tasks_vector.push_back(task);
    Globals::EcalSysInstance()->AddTask(task);
  }
  emit tasksAddedSignal(added_tasks_vector);

  // Import Runners
  std::vector<std::shared_ptr<EcalSysRunner>> added_runners_vector;
  added_runners_vector.reserve(runner_list_.size());
  for (auto& runner : runner_list_)
  {
    added_runners_vector.push_back(runner);
    Globals::EcalSysInstance()->AddRunner(runner);
  }
  emit runnersAddedSignal(added_runners_vector);
  emit closeSignal();
}

void ImportFromCloudWidget::loadExcludeTasksFilter()
{
  std::string default_cfg_file_path = eCAL::Util::GeteCALActiveIniFile();
  QFile default_cfg_file(default_cfg_file_path.c_str());
  if (default_cfg_file.exists())
  {
    EcalsysSettings settings;
    settings.Create(default_cfg_file_path);

    std::regex reg(settings.AppsFilterDenied(), std::regex::icase);
    exclude_tasks_regex_valid_ = !settings.AppsFilterDenied().empty();
    exclude_tasks_regex_ = reg;
  }
}
