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

#include "group_widget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QSettings>

#include "widgets/treemodels/tree_data_roles.h"
#include "widgets/treemodels/tree_item_types.h"
#include "CustomQt/QStableSortFilterProxyModel.h"
#include "widgets/targets_reachable_dialog/targets_reachable_dialog.h"

GroupWidget::GroupWidget(GroupEditWidget* group_edit_widget, QWidget *parent)
  : QWidget(parent)
  , group_edit_widget_(group_edit_widget)
  , currently_expanded_(false)
{
  ui_.setupUi(this);

  // Setup the edit widget
  ui_.edit_area_content_frame_layout->addWidget(group_edit_widget_);

  // Create Models for the group tree
  group_tree_model_              = new GroupTreeModel(this);
  group_tree_model_              ->reload(Globals::EcalSysInstance()->GetGroupList());
  group_sort_filter_proxy_model_ = new QStableSortFilterProxyModel(this);

  group_sort_filter_proxy_model_->setSourceModel(group_tree_model_);
  group_sort_filter_proxy_model_->setSortRole(ItemDataRoles::SortRole);
  group_sort_filter_proxy_model_->setFilterRole(ItemDataRoles::FilterRole);
  group_sort_filter_proxy_model_->setFilterKeyColumn((int)GroupTreeModel::Columns::NAME);
  group_sort_filter_proxy_model_->setDynamicSortFilter(false);
  group_sort_filter_proxy_model_->setRecursiveFilteringEnabled(true);
  group_sort_filter_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  ui_.group_tree                ->setModel(group_sort_filter_proxy_model_);

  // Setup the filter lineedit
  ui_.filter_lineedit->setClearIcon(QIcon(":/ecalicons/FILTER_CANCEL"));

  // Group Tree Signals
  connect(ui_.group_tree->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this,
    SLOT(groupTreeSelectionChanged()));
  connect(ui_.group_tree->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this,
    SIGNAL(selectionChangedSignal()));
  connect(ui_.group_tree,              SIGNAL(doubleClicked(QModelIndex)),                this, SLOT(groupTreeDoubleClicked(QModelIndex)));
  connect(ui_.group_tree,              SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(groupTreeContextMenu(const QPoint&)));

  connect(ui_.filter_lineedit,         SIGNAL(textChanged(QString)),                      this, SLOT(filterTextChanged(QString)));

  connect(ui_.group_tree,              SIGNAL(keySequenceDeletePressed()),                this, SLOT(removeSelectedGroups()));

  // Buttons
  connect(ui_.add_button,              SIGNAL(clicked()),                                 this, SLOT(addGroup()));
  connect(ui_.edit_button,             SIGNAL(clicked()),                                 this, SLOT(showEditControls()));
  connect(ui_.remove_button,           SIGNAL(clicked()),                                 this, SLOT(removeSelectedGroups()));
  connect(ui_.start_selected_button,   SIGNAL(clicked()),                                 this, SLOT(startSelectedTasks()));
  connect(ui_.stop_selected_button,    SIGNAL(clicked()),                                 this, SLOT(stopSelectedTasks()));
  connect(ui_.restart_selected_button, SIGNAL(clicked()),                                 this, SLOT(restartSelectedTasks()));
  connect(ui_.expand_button,           SIGNAL(clicked()),                                 this, SLOT(expandGroups()));
  connect(ui_.collapse_button,         SIGNAL(clicked()),                                 this, SLOT(collapseGroups()));

  connect(ui_.hide_edit_area_button,   SIGNAL(clicked()),                                 this, SLOT(hideEditControls()));
  connect(ui_.show_edit_area_button,   SIGNAL(clicked()),                                 this, SLOT(showEditControls()));

  // "Hidden" stop / restart menus
  ui_.stop_selected_button->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.stop_selected_button, &QPushButton::customContextMenuRequested,
    [this](const QPoint &pos)
    {
      QMenu context_menu(tr("Context menu"), this);
      QAction fast_stop_action(tr("Fast-kill selected"), this);
      context_menu.addAction(&fast_stop_action);
      connect(&fast_stop_action, &QAction::triggered, [this]() {stopSelectedTasks(false, true); });
      context_menu.exec(ui_.stop_selected_button->mapToGlobal(pos));
    }
  );
  ui_.restart_selected_button->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.restart_selected_button, &QPushButton::customContextMenuRequested,
    [this](const QPoint &pos)
    {
      QMenu context_menu(tr("Context menu"), this);
      QAction fast_restart_option(tr("Fast-kill && restart selected"), this);
      context_menu.addAction(&fast_restart_option);
      connect(&fast_restart_option, &QAction::triggered, [this]() {restartSelectedTasks(false, true); });
      context_menu.exec(ui_.restart_selected_button->mapToGlobal(pos));
    }
  );


  // Agressivly move the current index back to the name column in order to get proper keyboard search behaviour
  connect(ui_.group_tree->selectionModel(), &QItemSelectionModel::currentChanged,
    [=](const QModelIndex & current, const QModelIndex & /*previous*/)
    {
      if ((GroupTreeModel::Columns)(current.column()) != GroupTreeModel::Columns::NAME)
      {
        QModelIndex new_current_index = group_sort_filter_proxy_model_->index(current.row(), (int)GroupTreeModel::Columns::NAME);
        ui_.group_tree->selectionModel()->setCurrentIndex(new_current_index, QItemSelectionModel::NoUpdate);
      }
    }
  );

  // Update buttons whenever something is starting or stopping
  connect(this, SIGNAL(startStopActionTriggeredSignal()), this, SLOT(updateStartStopButtons()));

  // hide the edit controls
  hideEditControls();

  // update everything once
  groupTreeSelectionChanged();

  initial_group_tree_state_ = ui_.group_tree->saveState();
  loadGuiSettings();

}

GroupWidget::~GroupWidget()
{
  saveGuiSettings();
}

void GroupWidget::resetLayout()
{
  ui_.group_tree->restoreState(initial_group_tree_state_);
}

void GroupWidget::saveGuiSettings()
{
  QSettings settings;

  settings.beginGroup("group_widget");
  settings.setValue("tree_state", ui_.group_tree->saveState(Globals::ecalSysVersion()));
  settings.endGroup();
}

void GroupWidget::loadGuiSettings()
{
  QSettings settings;

  settings.beginGroup("group_widget");
  ui_.group_tree->restoreState(settings.value("tree_state").toByteArray(), Globals::ecalSysVersion());
  settings.endGroup();
}

QList<QModelIndex> GroupWidget::getSelectedIndices()
{
  auto selected_proxy_indices = ui_.group_tree->selectionModel()->selectedRows();

  QList<QModelIndex> selected_indices;
  for (auto& proxy_index : selected_proxy_indices)
  {
    selected_indices.push_back(group_sort_filter_proxy_model_->mapToSource(proxy_index));
  }

  return selected_indices;
}

QList<GroupTreeItem*> GroupWidget::getSelectedGroupItems()
{
  auto selected_indices = getSelectedIndices();
  QList<GroupTreeItem*> group_items;
  for (auto& index : selected_indices)
  {
    QAbstractTreeItem* item = group_tree_model_->item(index);
    if (item && (item->type() == (int)TreeItemType::Group))
    {
      group_items.push_back(static_cast<GroupTreeItem*>(item));
    }
  }
  return group_items;
}

void GroupWidget::configChanged()
{
  group_tree_model_->reload();
}

void GroupWidget::ecalsysOptionsChanged()
{
  group_tree_model_->updateAll();
}

void GroupWidget::monitorUpdated()
{
  group_tree_model_->updateAll();
}

void GroupWidget::tasksAdded(std::vector<std::shared_ptr<EcalSysTask>> /*tasks*/)
{
  // The table itself does not need to react to added tasks, as they are not used by any group
}

void GroupWidget::tasksRemoved(std::vector<std::shared_ptr<EcalSysTask>> /*tasks*/)
{
}

void GroupWidget::tasksModified(const std::vector<std::shared_ptr<EcalSysTask>>& tasks)
{
  for (auto& task : tasks) 
  {
    group_tree_model_->updateTask(task);
  }
}

void GroupWidget::groupsModified(const std::vector<std::shared_ptr<TaskGroup>>& groups)
{
  // This is the outside -> insidide slot that will be relaied towards the edit widget
  
  for (auto& group : groups)
  {
    group_tree_model_->updateGroup(group);
  }
}

////////////////////////////////////////////////////////////////////////////////
//// Group Tree                                                             ////
////////////////////////////////////////////////////////////////////////////////

void GroupWidget::groupTreeSelectionChanged()
{
  auto selected_group_items = getSelectedGroupItems();

  if (selected_group_items.empty())
  {
    // Set the edit-widget
    group_edit_widget_->setGroup(std::shared_ptr<TaskGroup>(nullptr));

    // Update the buttons
    ui_.remove_button->setEnabled(false);
    ui_.edit_button->setEnabled(false);
    updateStartStopButtons();
  }
  else
  {
    // Set the edit widget
    if (selected_group_items.size() == 1)
    {
      group_edit_widget_->setGroup(selected_group_items.first()->getGroup());
    }
    else
    {
      group_edit_widget_->setGroup(std::shared_ptr<TaskGroup>(nullptr));
    }

    // Update the buttons
    ui_.remove_button->setEnabled(true);
    ui_.edit_button->setEnabled(true);
    updateStartStopButtons();
  }
}

void GroupWidget::groupTreeDoubleClicked(const QModelIndex& index)
{
  QAbstractTreeItem* tree_item = group_tree_model_->item(group_sort_filter_proxy_model_->mapToSource(index));
  if (tree_item) {
    if (tree_item->type() == (int)TreeItemType::Group)
    {
      showEditControls();
    }
    else if (tree_item->type() == (int)TreeItemType::Task)
    {
      TaskTreeItem* task_tree_item = static_cast<TaskTreeItem*> (tree_item);
      emit setEditTaskSignal(task_tree_item->getTask());
    }
  }
}

void GroupWidget::groupTreeContextMenu(const QPoint& pos)
{
  QMenu context_menu(tr("Context menu"), this);

  QAction start_action         (tr("Start"),           this);
  QAction stop_action          (tr("Stop"),            this);
  QAction restart_action       (tr("Restart"),         this);
  QMenu   start_on_host_menu   (tr("Start on host"),   this);
  QMenu   restart_to_host_menu (tr("Restart on host"), this);
  QAction fast_kill_action     (tr("Fast-kill"),       this);
  QAction fast_restart_action  (tr("Fast-kill && restart"), this);
  QAction add_action           (tr("Add Group"),       this);
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

  auto selected_group_items = getSelectedGroupItems();
  auto selected_tasks = getAllSelectedTasks();

  if (selected_tasks.size() == 0)
  {
    // Deactivate start / stop / restart options, if no task is selected
    start_action        .setEnabled(false);
    stop_action         .setEnabled(false);
    restart_action      .setEnabled(false);
    fast_kill_action    .setEnabled(false);
    fast_restart_action .setEnabled(false);
    start_on_host_menu  .setEnabled(false);
    restart_to_host_menu.setEnabled(false);
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
        restart_to_host_menu.setEnabled(false);
        break;
      }
    }
  }

  if (selected_group_items.size() != 1)
  {
    edit_action.setEnabled(false);
  }

  if (selected_group_items.size() == 0)
  {
    duplicate_action.setEnabled(false);
    remove_action.setEnabled   (false);
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
    restart_to_host_menu.addAction(dummy_action2);
  }
  else {
    for (std::string host : all_hosts)
    {
      QAction* start_on_host_action   = new QAction(host.c_str(), &context_menu);
      QAction* restart_to_host_action = new QAction(host.c_str(), &context_menu);
      start_on_host_menu  .addAction(start_on_host_action);
      restart_to_host_menu.addAction(restart_to_host_action);
    }
  }

  context_menu.addAction   (&start_action);
  context_menu.addAction   (&stop_action);
  context_menu.addAction   (&restart_action);
  context_menu.addSeparator();
  context_menu.addMenu     (&start_on_host_menu);
  context_menu.addMenu     (&restart_to_host_menu);
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
  connect(&start_on_host_menu,   SIGNAL(triggered(QAction*)), this, SLOT(startOnHostActionTriggered(QAction*)));
  connect(&restart_to_host_menu, SIGNAL(triggered(QAction*)), this, SLOT(restartToHostActionTriggered(QAction*)));
  connect(&add_action ,          SIGNAL(triggered()),         this, SLOT(addGroup()));
  connect(&edit_action,          SIGNAL(triggered()),         this, SLOT(showEditControls()));
  connect(&duplicate_action,     SIGNAL(triggered()),         this, SLOT(duplicateSelectedGroups()));
  connect(&remove_action,        SIGNAL(triggered()),         this, SLOT(removeSelectedGroups()));

  connect(&fast_kill_action,    &QAction::triggered, [this]() {stopSelectedTasks(false, true); });
  connect(&fast_restart_action, &QAction::triggered, [this]() {restartSelectedTasks(false, true); });

  context_menu.exec(ui_.group_tree->viewport()->mapToGlobal(pos));
}

void GroupWidget::filterTextChanged(QString text)
{
  group_sort_filter_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  group_sort_filter_proxy_model_->setFilterRegExp(text);
}

////////////////////////////////////////////////////////////////////////////////
//// Buttons                                                                ////
////////////////////////////////////////////////////////////////////////////////

void GroupWidget::addGroup()
{
  // clear the filter
  ui_.filter_lineedit->setText("");

  // Create a new group
  std::shared_ptr<TaskGroup> new_group(new TaskGroup());
  Globals::EcalSysInstance()->AddTaskGroup(new_group);
  group_tree_model_->addGroup(new_group);

  // Show the edit controls
  showEditControls();
  QModelIndex proxy_index = group_sort_filter_proxy_model_->mapFromSource(group_tree_model_->index(new_group, (int)GroupTreeModel::Columns::NAME));
  ui_.group_tree->scrollTo(proxy_index);
  ui_.group_tree->selectionModel()->select(proxy_index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
  group_edit_widget_->focusGroupName();

  emit groupsAddedSignal(std::vector<std::shared_ptr<TaskGroup>> { new_group });
}

void GroupWidget::removeSelectedGroups()
{
  auto selected_proxy_indices = ui_.group_tree->selectionModel()->selectedRows();
  
  QList<QAbstractTreeItem*> selected_group_tree_items;
  std::vector<std::shared_ptr<TaskGroup>> selected_groups_vector;

  for (auto& proxy_index : selected_proxy_indices)
  {
    QModelIndex source_index = group_sort_filter_proxy_model_->mapToSource(proxy_index);
    QAbstractTreeItem* tree_item = group_tree_model_->item(source_index);
    if (tree_item && tree_item->type() == (int)TreeItemType::Group)
    {
      GroupTreeItem* group_tree_item = static_cast<GroupTreeItem*>(tree_item);

      // Keep a list of removed Elements
      selected_group_tree_items.push_back(group_tree_item);
      selected_groups_vector   .push_back(group_tree_item->getGroup());
    }
  }

  // Remove items from the tree
  group_tree_model_->removeItems(selected_group_tree_items);

  // Remove Groups from the actual model
  for (auto& group : selected_groups_vector)
  {
    Globals::EcalSysInstance()->RemoveTaskGroup(group);
  }

  emit groupsRemovedSignal(selected_groups_vector);
}

void GroupWidget::duplicateSelectedGroups()
{
  auto selected_group_items = getSelectedGroupItems();

  if (selected_group_items.size() > 0)
  {
    std::vector<std::shared_ptr<TaskGroup>> new_groups;
    new_groups.reserve(selected_group_items.size());

    // Clone all groups
    for (auto& group_item : selected_group_items)
    {
      std::shared_ptr<TaskGroup> new_group(new TaskGroup());
      new_group->SetName(group_item->getGroup()->GetName());

      std::list<std::shared_ptr<TaskGroup::GroupState>>new_group_state_list;

      for (auto& group_state : group_item->getGroup()->GetGroupStateList())
      {
        std::shared_ptr<TaskGroup::GroupState> new_group_state(new TaskGroup::GroupState());
        new_group_state->SetName(group_state->GetName());
        new_group_state->SetColor(group_state->GetColor());

        std::list<std::pair<std::shared_ptr<EcalSysTask>, TaskState>> new_minimal_task_state_list;

        for (auto& minimal_task_state : group_state->GetMinimalStatesList())
        {
          std::pair<std::shared_ptr<EcalSysTask>, TaskState> new_minimal_task_state;
          new_minimal_task_state.first = minimal_task_state.first;
          new_minimal_task_state.second.severity = minimal_task_state.second.severity;
          new_minimal_task_state.second.severity_level = minimal_task_state.second.severity_level;

          new_minimal_task_state_list.push_back(new_minimal_task_state);
        }
        new_group_state->SetMinimalStatesList(new_minimal_task_state_list);
        new_group_state_list.push_back(new_group_state);
      }
      new_group->SetGroupStateList(new_group_state_list);
      new_groups.push_back(new_group);
    }

    // Add the groups to the model and the treeview
    for (auto& group : new_groups)
    {
      group_tree_model_->addGroup(group);
      Globals::EcalSysInstance()->AddTaskGroup(group);
    }

    // Select the new groups
    ui_.group_tree->selectionModel()->clearSelection();
    for (auto& group : new_groups)
    {
      QModelIndex source_index = group_tree_model_->index(group);
      QModelIndex proxy_index = group_sort_filter_proxy_model_->mapFromSource(source_index);
      ui_.group_tree->selectionModel()->select(proxy_index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    // Scroll to the new groups
    QModelIndex first_index = group_sort_filter_proxy_model_->mapFromSource(group_tree_model_->index(new_groups[0], (int)GroupTreeModel::Columns::NAME));
    ui_.group_tree->scrollTo(first_index, QAbstractItemView::ScrollHint::PositionAtTop);

    // Move the textcursor to the name field, if only one group was cloned
    if (selected_group_items.size() == 1)
    {
      showEditControls();
      group_edit_widget_->focusGroupName(true);
    }

    emit groupsAddedSignal(new_groups);
  }
}

void GroupWidget::startSelectedTasks(const std::string& target_override)
{
  auto tasks_to_start_set = getAllSelectedTasks();

  // Convert the set into a list that is required for the ecalsys api
  std::list<std::shared_ptr<EcalSysTask>> tasks_to_start_list;
  for (auto& task : tasks_to_start_set)
  {
    tasks_to_start_list.push_back(task);
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
      if (!checkTargetsReachable(tasks_to_start_list))
      {
        return;
      }
    }
  }

  Globals::EcalSysInstance()->StartTaskList(tasks_to_start_list, target_override);
  emit startStopActionTriggeredSignal();
}

void GroupWidget::startOnHostActionTriggered(QAction* action)
{
  auto selected_tasks_set = getAllSelectedTasks();
  std::list<std::shared_ptr<EcalSysTask>> selected_tasks_list;
  for (auto& task : selected_tasks_set)
  {
    selected_tasks_list.push_back(task);
  }

  auto options = Globals::EcalSysInstance()->GetOptions();
  if (options.check_target_reachability)
  {
    std::set<std::string> target_set;
    target_set.emplace(action->text().toStdString());
    if (!checkTargetsReachable(target_set))
    {
      return;
    }
  }

  Globals::EcalSysInstance()->StartTaskList(selected_tasks_list, action->text().toStdString());
  emit startStopActionTriggeredSignal();
}

void GroupWidget::restartToHostActionTriggered(QAction* action)
{
  auto selected_tasks_set = getAllSelectedTasks();
  std::list<std::shared_ptr<EcalSysTask>> selected_tasks_list;
  for (auto& task : selected_tasks_set)
  {
    selected_tasks_list.push_back(task);
  }

  auto options = Globals::EcalSysInstance()->GetOptions();
  if (options.check_target_reachability)
  {
    std::set<std::string> target_set;
    target_set.emplace(action->text().toStdString());
    if (!checkTargetsReachable(target_set))
    {
      return;
    }
  }

  Globals::EcalSysInstance()->RestartTaskList(selected_tasks_list, true, true, action->text().toStdString());
  emit startStopActionTriggeredSignal();
}

void GroupWidget::stopSelectedTasks(bool shutdown_request, bool kill_tasks)
{
  auto tasks_to_stop_set = getAllSelectedTasks();

  // Convert the set into a list that is require for the ecalsys api
  std::list<std::shared_ptr<EcalSysTask>> tasks_to_stop_list;
  for (auto& task : tasks_to_stop_set)
  {
    tasks_to_stop_list.push_back(task);
  }

  Globals::EcalSysInstance()->StopTaskList(tasks_to_stop_list, shutdown_request, kill_tasks);
  emit startStopActionTriggeredSignal();
}

void GroupWidget::restartSelectedTasks(bool shutdown_request, bool kill_tasks, const std::string& target_override)
{
  auto tasks_to_restart_set = getAllSelectedTasks();

  // Convert the set into a list that is required for the ecalsys api
  std::list<std::shared_ptr<EcalSysTask>> tasks_to_restart_list;
  for (auto& task : tasks_to_restart_set)
  {
    tasks_to_restart_list.push_back(task);
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
      if (!checkTargetsReachable(tasks_to_restart_list))
      {
        return;
      }
    }
  }

  Globals::EcalSysInstance()->RestartTaskList(tasks_to_restart_list, shutdown_request, kill_tasks, target_override);
  emit startStopActionTriggeredSignal();
}

void GroupWidget::collapseGroups()
{
  ui_.group_tree->collapseAll();
}

void GroupWidget::expandGroups()
{
  ui_.group_tree->expandAll();
}

std::set<std::shared_ptr<EcalSysTask>> GroupWidget::getAllSelectedTasks()
{
  std::set<std::shared_ptr<EcalSysTask>> all_selected_tasks;

  auto selected_proxy_indices = ui_.group_tree->selectionModel()->selectedRows();
  for (QModelIndex& proxy_index : selected_proxy_indices)
  {
    QModelIndex source_index = group_sort_filter_proxy_model_->mapToSource(proxy_index);
    QAbstractTreeItem* tree_item = group_tree_model_->item(source_index);
    if (tree_item)
    {
      if (tree_item->type() == (int)TreeItemType::Group)
      {
        // If a group is selected we add all contained tasks
        GroupTreeItem* group_tree_item = static_cast<GroupTreeItem*>(tree_item);
        for (auto& task : group_tree_item->getGroup()->GetAllTasks())
        {
          all_selected_tasks.emplace(task);
        }
      }
      else if (tree_item->type() == (int)TreeItemType::Task)
      {
        // If a task is selected we add only that task
        TaskTreeItem* task_tree_item = static_cast<TaskTreeItem*>(tree_item);
        all_selected_tasks.emplace(task_tree_item->getTask());
      }
    }
  }
  return all_selected_tasks;
}

std::list<std::shared_ptr<TaskGroup>> GroupWidget::getSelectedGroups()
{
  auto selected_group_items = getSelectedGroupItems();
  std::list<std::shared_ptr<TaskGroup>> selected_groups;
  for (auto& group_item : selected_group_items)
  {
    selected_groups.push_back(group_item->getGroup());
  }
  return selected_groups;
}

void GroupWidget::updateStartStopButtons()
{
  auto selected_tasks = getAllSelectedTasks();

  if (selected_tasks.size() == 0)
  {
    ui_.start_selected_button->setEnabled(false);
    ui_.stop_selected_button->setEnabled(false);
    ui_.restart_selected_button->setEnabled(false);
  }
  else
  {
    bool anything_starting_or_stopping = false;
    for (auto& task : selected_tasks)
    {
      if (Globals::EcalSysInstance()->IsTaskActionRunning(task))
      {
        anything_starting_or_stopping = true;
        break;
      }
    }
    ui_.start_selected_button->setEnabled(!anything_starting_or_stopping);
    ui_.stop_selected_button->setEnabled(!anything_starting_or_stopping);
    ui_.restart_selected_button->setEnabled(!anything_starting_or_stopping);
  }
}

////////////////////////////////////////////////////////////////////////////////
//// Edit Controls                                                          ////
////////////////////////////////////////////////////////////////////////////////


void GroupWidget::showEditControls()
{
  // As we have a splitter *above* the Hide-button, but don't want the user to
  // resize the button, we use the two-button workaround here, where always
  // one button is hidden. The Show-button is above the splitter, the
  // Hide-button beneath it.
  ui_.edit_area_widget->setVisible(true);
  ui_.show_edit_area_button->setVisible(false);
}

void GroupWidget::hideEditControls()
{
  ui_.edit_area_widget->setVisible(false);
  ui_.show_edit_area_button->setVisible(true);
}

////////////////////////////////////////////////////////////////////////////////
//// Auxiliary                                                              ////
////////////////////////////////////////////////////////////////////////////////

bool GroupWidget::checkTargetsReachable(const std::list<std::shared_ptr<EcalSysTask>>& task_list)
{
  std::set<std::string> target_set;
  for (auto& task : task_list)
  {
    target_set.emplace(task->GetTarget());
  }
  return checkTargetsReachable(target_set);
}

bool GroupWidget::checkTargetsReachable(const std::set<std::string>& target_set)
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
