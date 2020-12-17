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

#include "group_edit_widget.h"

#include <algorithm>
#include <list>

#include <QColorDialog>
#include <QTimer>
#include <QSettings>
#include <QMenu>
#include <QAction>

#include "widgets/treemodels/tree_item_types.h"
#include "widgets/treemodels/tree_data_roles.h"

GroupEditWidget::GroupEditWidget(QWidget *parent)
  : QWidget(parent)
  , initial_splitter_state_valid_(false)
{
  ui_.setupUi(this);

  // Create all Models and Proxy Models
  group_state_tree_model_           = new GroupStateTreeModel(this);
  available_tasks_tree_model_       = new TaskTreeModel(this);
  available_tasks_proxy_tree_model_ = new QStableSortFilterProxyModel(this);
  active_tasks_tree_model_          = new GroupStateMinTaskStateTreeModel(this);
  active_tasks_proxy_tree_model_    = new QStableSortFilterProxyModel(this);

  // Set the source for the proxy models
  available_tasks_proxy_tree_model_ ->setSourceModel(available_tasks_tree_model_);
  active_tasks_proxy_tree_model_    ->setSourceModel(active_tasks_tree_model_);

  // Set Sort and Filter Roles
  available_tasks_proxy_tree_model_ ->setSortRole(ItemDataRoles::SortRole);
  active_tasks_proxy_tree_model_    ->setSortRole(ItemDataRoles::SortRole);
  available_tasks_proxy_tree_model_ ->setFilterRole(ItemDataRoles::FilterRole);
  active_tasks_proxy_tree_model_    ->setFilterRole(ItemDataRoles::FilterRole);

  // Case sensitivity
  available_tasks_proxy_tree_model_ ->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  active_tasks_proxy_tree_model_    ->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);

  // Assign Models to TreeViews
  ui_.group_states_tree             ->setModel(group_state_tree_model_);
  ui_.state_available_tasks_tree    ->setModel(available_tasks_proxy_tree_model_);
  ui_.state_active_tasks_tree       ->setModel(active_tasks_proxy_tree_model_);

  // Set forced columns
  ui_.group_states_tree             ->setForcedColumns(QVector<int>{(int)GroupStateTreeModel::Columns::NAME});
  ui_.state_available_tasks_tree    ->setForcedColumns(QVector<int>{(int)TaskTreeModel::Columns::TASK_NAME});
  ui_.state_active_tasks_tree       ->setForcedColumns(QVector<int>{(int)GroupStateMinTaskStateTreeModel::Columns::TASK_NAME});


  // Hide unnecessary Columns
  for (int i = 0; i < (int)TaskTreeModel::Columns::COLUMN_COUNT; i++)
  {
    if (! (i == (int)TaskTreeModel::Columns::TASK_NAME))
    {
      ui_.state_available_tasks_tree->setColumnHidden(i, true);
    }
  }
  for (int i = 0; i < (int)GroupStateMinTaskStateTreeModel::Columns::COLUMN_COUNT; i++)
  {
    if (!((i == (int)GroupStateMinTaskStateTreeModel::Columns::TASK_NAME)
          || (i == (int)GroupStateMinTaskStateTreeModel::Columns::SEVERITY)
          || (i == (int)GroupStateMinTaskStateTreeModel::Columns::SEVERITY_LEVEL)))
    {
      ui_.state_active_tasks_tree->setColumnHidden(i, true);
    }
  }

  // Rename the Task-name column
  available_tasks_tree_model_ ->setHeaderLabel(TaskTreeModel::Columns::TASK_NAME, tr("Available tasks"));

  // Fill available tasks with data (or rather fill it with nothing)
  available_tasks_tree_model_->reload(std::list<std::shared_ptr<EcalSysTask>>());

  // Set the delegation for editing the severity right from the table
  severity_picker_delegate_ = new SeverityPickerDelegate(this);
  ui_.state_active_tasks_tree->setItemDelegate(severity_picker_delegate_);

  // connect the tree models
  connect(ui_.group_states_tree->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this,
    SLOT(groupStateSelectionChanged()));
  connect(ui_.state_available_tasks_tree->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this,
    SLOT(availableTasksSelectionChanged()));
  connect(ui_.state_active_tasks_tree->selectionModel(),
    SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
    this,
    SLOT(activeTasksSelectionChanged()));

  connect(ui_.group_states_tree,          SIGNAL(keySequenceDeletePressed()),         this, SLOT(groupStateRemoveButtonClicked()));
  connect(ui_.state_active_tasks_tree,    SIGNAL(keySequenceDeletePressed()),         this, SLOT(stateRemoveTaskButtonClicked()));

  connect(ui_.state_available_tasks_tree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(stateAddTaskButtonClicked()));
  connect(ui_.state_active_tasks_tree,    SIGNAL(doubleClicked(QModelIndex)), this, SLOT(stateRemoveTaskButtonClicked()));

  // Agressivly move the current index back to the name column in order to get proper keyboard search behaviour
  connect(ui_.state_available_tasks_tree->selectionModel(), &QItemSelectionModel::currentChanged,
    [=](const QModelIndex & current, const QModelIndex & /*previous*/)
    {
      if ((TaskTreeModel::Columns)(current.column()) != TaskTreeModel::Columns::TASK_NAME)
      {
        QModelIndex new_current_index = available_tasks_proxy_tree_model_->index(current.row(), (int)TaskTreeModel::Columns::TASK_NAME);
        ui_.state_available_tasks_tree->selectionModel()->setCurrentIndex(new_current_index, QItemSelectionModel::NoUpdate);
      }
    }
  );
  connect(ui_.state_active_tasks_tree->selectionModel(), &QItemSelectionModel::currentChanged,
    [=](const QModelIndex & current, const QModelIndex & /*previous*/)
    {
      if (((GroupStateMinTaskStateTreeModel::Columns)(current.column()) != GroupStateMinTaskStateTreeModel::Columns::TASK_NAME)
        && ((GroupStateMinTaskStateTreeModel::Columns)(current.column()) != GroupStateMinTaskStateTreeModel::Columns::SEVERITY)
        && ((GroupStateMinTaskStateTreeModel::Columns)(current.column()) != GroupStateMinTaskStateTreeModel::Columns::SEVERITY_LEVEL))
      {
        QModelIndex new_current_index = active_tasks_proxy_tree_model_->index(current.row(), (int)GroupStateMinTaskStateTreeModel::Columns::TASK_NAME);
        ui_.state_active_tasks_tree->selectionModel()->setCurrentIndex(new_current_index, QItemSelectionModel::NoUpdate);
      }
    }
  );

  // Create context menus
  ui_.group_states_tree         ->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  ui_.state_available_tasks_tree->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  ui_.state_active_tasks_tree   ->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

  connect(ui_.group_states_tree, &QTreeView::customContextMenuRequested,
    [this](const QPoint &pos)
    {
      if (group_)
      {
        QMenu context_menu(this);

        QAction add_action      (tr("Add State"), &context_menu);
        QAction remove_action   (tr("Remove"), &context_menu);
        QAction move_up_action  (tr("Move up"), &context_menu);
        QAction move_down_action(tr("Move down"), &context_menu);

        // Sort selected indices so we begin with the upper row
        auto selected_indices = ui_.group_states_tree->selectionModel()->selectedRows();
        if (selected_indices.size() > 0)
        {
          std::sort(selected_indices.begin(), selected_indices.end(), [](const QModelIndex& i1, const QModelIndex& i2) { return i1.row() < i2.row(); });
          move_up_action.setEnabled(selected_indices.first().row() != 0);
          move_down_action.setEnabled(selected_indices.last().row() != (group_state_tree_model_->rowCount() - 1));
        }
        else
        {
          remove_action.setEnabled(false);
          move_up_action.setEnabled(false);
          move_down_action.setEnabled(false);
        }

        connect(&add_action,       &QAction::triggered, [this]() {groupStateAddButtonClicked(); });
        connect(&remove_action,    &QAction::triggered, [this]() {groupStateRemoveButtonClicked(); });
        connect(&move_up_action,   &QAction::triggered, [this]() {groupStateMoveUpButtonClicked(); });
        connect(&move_down_action, &QAction::triggered, [this]() {groupStateMoveDownButtonClicked(); });

        context_menu.addAction(&add_action);
        context_menu.addAction(&remove_action);
        context_menu.addSeparator();
        context_menu.addAction(&move_up_action);
        context_menu.addAction(&move_down_action);

        context_menu.exec(ui_.group_states_tree->viewport()->mapToGlobal(pos));
      }
    }
  );

  connect(ui_.state_available_tasks_tree, &QTreeView::customContextMenuRequested,
    [this](const QPoint &pos)
    {
      if (group_ && group_state_)
      {
        QMenu context_menu(this);

        auto selected_indices = ui_.state_available_tasks_tree->selectionModel()->selectedRows();
        QAction add_action((selected_indices.size() > 1 ? tr("Add these tasks") : tr("Add this task")), &context_menu);
        add_action.setEnabled(selected_indices.size() > 0);
        connect(&add_action, &QAction::triggered, [this]() {stateAddTaskButtonClicked(); });
        context_menu.addAction(&add_action);
        context_menu.exec(ui_.state_available_tasks_tree->viewport()->mapToGlobal(pos));
      }
    }
  );

  connect(ui_.state_active_tasks_tree, &QTreeView::customContextMenuRequested,
    [this](const QPoint &pos)
    {
      if (group_ && group_state_)
      {
        QMenu context_menu(this);

        auto selected_indices = ui_.state_active_tasks_tree->selectionModel()->selectedRows();
        QAction remove_action((selected_indices.size() > 1 ? tr("Remove these tasks") : tr("Remove this task")), &context_menu);
        remove_action.setEnabled(selected_indices.size() > 0);
        connect(&remove_action, &QAction::triggered, [this]() {stateRemoveTaskButtonClicked(); });
        context_menu.addAction(&remove_action);
        context_menu.exec(ui_.state_active_tasks_tree->viewport()->mapToGlobal(pos));
      }
    }
  );

  // Connecte the edit controls
  connect(ui_.group_name_lineedit,       SIGNAL(editingFinished()), this, SLOT(groupNameEditingFinished()));
  connect(ui_.group_add_state_button,    SIGNAL(clicked()),         this, SLOT(groupStateAddButtonClicked()));
  connect(ui_.group_remove_state_button, SIGNAL(clicked()),         this, SLOT(groupStateRemoveButtonClicked()));
  connect(ui_.group_state_up_button,     SIGNAL(clicked()),         this, SLOT(groupStateMoveUpButtonClicked()));
  connect(ui_.group_state_down_button,   SIGNAL(clicked()),         this, SLOT(groupStateMoveDownButtonClicked()));

  connect(ui_.state_name_lineedit,       SIGNAL(editingFinished()), this, SLOT(stateNameEditingFinished()));
  connect(ui_.state_color_button,        SIGNAL(clicked()),         this, SLOT(stateColorButtonClicked()));
  connect(ui_.state_add_task_button,     SIGNAL(clicked()),         this, SLOT(stateAddTaskButtonClicked()));
  connect(ui_.state_remove_task_button,  SIGNAL(clicked()),         this, SLOT(stateRemoveTaskButtonClicked()));

  setGroup(std::shared_ptr<TaskGroup>(nullptr));

  // Update the GUI manually once
  groupStateSelectionChanged();

  group_state_tree_initial_state_     = ui_.group_states_tree         ->saveState();
  available_tasks_tree_initial_state_ = ui_.state_available_tasks_tree->saveState();
  active_tasks_tree_initial_state_    = ui_.state_active_tasks_tree   ->saveState();
}

GroupEditWidget::~GroupEditWidget()
{
}

void GroupEditWidget::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);

  // We have to actually show the splitter, in order to know its size. Thus, we have to save this value in the showevent.
  if (!initial_splitter_state_valid_)
  {  
    main_splitter_initial_state_        = ui_.main_splitter       ->saveState();
    state_trees_splitter_initial_state_ = ui_.state_trees_splitter->saveState();
    initial_splitter_state_valid_ = true;
  }
}

void GroupEditWidget::resetLayout()
{
  ui_.group_states_tree         ->restoreState(group_state_tree_initial_state_);
  ui_.state_available_tasks_tree->restoreState(available_tasks_tree_initial_state_);
  ui_.state_active_tasks_tree   ->restoreState(active_tasks_tree_initial_state_);

  if (initial_splitter_state_valid_)
  {
    ui_.main_splitter       ->restoreState(main_splitter_initial_state_);
    ui_.state_trees_splitter->restoreState(state_trees_splitter_initial_state_);
  }
}

void GroupEditWidget::setGroup(std::shared_ptr<TaskGroup> group)
{
  group_ = group;

  ui_.group_name_lineedit->blockSignals(true);

  if (group_)
  {
    ui_.main_splitter->setEnabled(true);
    ui_.group_name_lineedit->setText(group_->GetName().c_str());
  }
  else
  {
    ui_.main_splitter->setEnabled(false);
    ui_.group_name_lineedit->setText("");
  }

  group_state_tree_model_->setGroup(group_);
  if (group_state_tree_model_->columnCount() > 0)
  {
    QModelIndex first_index = group_state_tree_model_->index(0, 0);
    ui_.group_states_tree->selectionModel()->select(first_index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
  }

  ui_.group_name_lineedit->blockSignals(false);
}

void GroupEditWidget::focusGroupName(bool select_text)
{
  ui_.group_name_lineedit->setFocus();
  if (select_text)
  {
    ui_.group_name_lineedit->selectAll();
  }
}

void GroupEditWidget::groupStateSelectionChanged()
{
  auto selected_rows = ui_.group_states_tree->selectionModel()->selectedRows();
  if (selected_rows.size() == 0)
  {
    ui_.group_remove_state_button->setEnabled(false);
    ui_.state_edit_controls_widget->setEnabled(false);
    editState(std::shared_ptr<TaskGroup::GroupState>(nullptr));
  }
  else if (selected_rows.size() == 1)
  {
    ui_.group_remove_state_button->setEnabled(true);
    ui_.state_edit_controls_widget->setEnabled(true);

    // Set the edit controls to the selected state
    QAbstractTreeItem* tree_item = group_state_tree_model_->item(selected_rows.front());
    if (tree_item && tree_item->type() == (int)TreeItemType::GroupState)
    {
      editState(static_cast<GroupStateTreeItem*>(tree_item)->getGroupState());
    }
    else
    {
      editState(std::shared_ptr<TaskGroup::GroupState>(nullptr));
    }
  }
  else
  {
    ui_.group_remove_state_button->setEnabled(true);
    ui_.state_edit_controls_widget->setEnabled(false);
    editState(std::shared_ptr<TaskGroup::GroupState>(nullptr));
  }
  updateMoveUpDownButtons();
}

void GroupEditWidget::availableTasksSelectionChanged()
{
  auto selected_proxy_rows = ui_.state_available_tasks_tree->selectionModel()->selectedRows();

  // Activate / deactivate the add-task-button
  ui_.state_add_task_button->setEnabled(!selected_proxy_rows.isEmpty());

  // Clear the other selection, just to not confuse the user with too many buttons
  if (!selected_proxy_rows.isEmpty())
  {
    ui_.state_active_tasks_tree->clearSelection();
  }
}

void GroupEditWidget::activeTasksSelectionChanged()
{
  auto selected_proxy_rows = ui_.state_active_tasks_tree->selectionModel()->selectedRows();

  // Activate / deactivate the remove-task-button
  ui_.state_remove_task_button->setEnabled(!selected_proxy_rows.isEmpty());

  // Clear the other selection, just to not confuse the user with too many buttons
  if (!selected_proxy_rows.isEmpty())
  {
    ui_.state_available_tasks_tree->clearSelection();
  }
}

void GroupEditWidget::updateMoveUpDownButtons()
{
  auto selected_indices = ui_.group_states_tree->selectionModel()->selectedRows();
  std::sort(selected_indices.begin(), selected_indices.end(), [](const QModelIndex& i1, const QModelIndex& i2) { return i1.row() < i2.row(); });

  //deactivate both buttons if no indices are selected
  if (selected_indices.size() == 0)
  {
    ui_.group_state_up_button->setEnabled(false);
    ui_.group_state_down_button->setEnabled(false);
  }
  else
  {
    //deactivate / activate up/down buttons based on whether the first / last row is selected
    ui_.group_state_up_button->setEnabled(!(selected_indices.first().row() == 0));
    ui_.group_state_down_button->setEnabled(!(selected_indices.last().row() == group_state_tree_model_->rowCount() - 1));
  }
}

void GroupEditWidget::updateTasks()
{
  available_tasks_tree_model_->updateAll();
  active_tasks_tree_model_->updateAll();
}

void GroupEditWidget::editState(std::shared_ptr<TaskGroup::GroupState> state)
{
  group_state_ = state;

  ui_.state_name_lineedit->blockSignals(true);

  // Set State-name
  ui_.state_name_lineedit->setText(state? state->GetName().c_str() : "");

  // Set the color of the color-button
  if (state)
  {
    auto color = state->GetColor();
    const QString COLOR_STYLE("QPushButton { background-color : %1; }");
    ui_.state_color_button->setStyleSheet(COLOR_STYLE.arg(QColor(color.red, color.green, color.blue).name()));
  }
  else
  {
    const QString COLOR_STYLE("QPushButton { background-color : %1; }");
    ui_.state_color_button->setStyleSheet(COLOR_STYLE.arg(QColor(220, 220, 220).name()));
  }

  // Populate the active tasks lists
  active_tasks_tree_model_->setGroupState(state);
  active_tasks_proxy_tree_model_->sort((int)GroupStateMinTaskStateTreeModel::Columns::TASK_NAME);

  // Populate the available tasks list
  if (state)
  {
    auto task_list = Globals::EcalSysInstance()->GetTaskList();
    task_list.remove_if(
      [&state](std::shared_ptr<EcalSysTask> task)
      {
        auto min_state_list = state->GetMinimalStatesList();
        for (auto min_state : min_state_list)
        {
          if (min_state.first == task)
            return true;
        }
        return false;
      }
    );
    available_tasks_tree_model_->reload(task_list);
    available_tasks_proxy_tree_model_->sort((int)TaskTreeModel::Columns::TASK_NAME);

  }
  else
  {
    available_tasks_tree_model_->reload(std::list<std::shared_ptr<EcalSysTask>>());
  }

  ui_.state_name_lineedit->blockSignals(false);

  // Activate / deactivate the add/remove tasks button
  availableTasksSelectionChanged();
  activeTasksSelectionChanged();
}


////////////////////////////////////////////////////////////////////////////////
//// Edit Control Callbacks                                                 ////
////////////////////////////////////////////////////////////////////////////////

void GroupEditWidget::groupNameEditingFinished()
{
  if (group_)
  {
    QString text = ui_.group_name_lineedit->text();
    if (text.toStdString() != group_->GetName())
    {
      group_->SetName(text.toStdString());
      emit groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>{group_});
    }
  }
}

void GroupEditWidget::groupStateAddButtonClicked()
{
  if (group_)
  {
    std::shared_ptr<TaskGroup::GroupState> new_state(new TaskGroup::GroupState());
    auto state_list = group_->GetGroupStateList();
    state_list.push_back(new_state);
    group_->SetGroupStateList(state_list);

    GroupStateTreeItem* new_state_tree_item = new GroupStateTreeItem(new_state);
    group_state_tree_model_->insertItem(new_state_tree_item);

    // Scroll to the new item and select it!
    QModelIndex new_state_index = group_state_tree_model_->index(new_state_tree_item, (int)GroupStateTreeModel::Columns::NAME);
    ui_.group_states_tree->scrollTo(new_state_index);
    ui_.group_states_tree->selectionModel()->select(new_state_index, QItemSelectionModel::SelectionFlag::ClearAndSelect);

    // Set the focus to the name-field
    ui_.state_name_lineedit->setFocus(Qt::FocusReason::OtherFocusReason);

    emit groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>{group_});
  }
}

void GroupEditWidget::groupStateRemoveButtonClicked()
{
  if (group_)
  {
    auto group_state_list = group_->GetGroupStateList();

    auto selected_indices = ui_.group_states_tree->selectionModel()->selectedRows();

    for (auto& selected_index : selected_indices)
    {
      QAbstractTreeItem* selected_item = group_state_tree_model_->item(selected_index);
      if (selected_item && selected_item->type() == (int)TreeItemType::GroupState)
      {
        GroupStateTreeItem* group_state_item = static_cast<GroupStateTreeItem*>(selected_item);
        auto group_state = group_state_item->getGroupState();
        group_state_list.remove(group_state);
      }
    }

    group_->SetGroupStateList(group_state_list);
    group_state_tree_model_->removeItems(selected_indices);

    emit groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>{group_});
  }
}

void GroupEditWidget::groupStateMoveUpButtonClicked() 
{
  if (group_)
  {
    auto selected_indices = ui_.group_states_tree->selectionModel()->selectedRows();
    auto group_state_list = group_->GetGroupStateList();

    // Sort selected indices so we begin with the upper row
    std::sort(selected_indices.begin(), selected_indices.end(), [](const QModelIndex& i1, const QModelIndex& i2) { return i1.row() < i2.row(); });
    
    // If there are no indices, or the first one already is on the top, we are not able to move them
    if (!(selected_indices.size() == 0 || selected_indices.first().row() == 0))
    {
      for (auto& selected_index : selected_indices)
      {
        // Modify the treemodel
        group_state_tree_model_->moveItem(selected_index.row(), selected_index.row() - 1);

        // Modify the actual model
        auto source_iterator = group_state_list.begin();
        auto target_iterator = source_iterator;
        std::advance(source_iterator, selected_index.row());
        std::advance(target_iterator, selected_index.row() - 1);
        group_state_list.splice(target_iterator, group_state_list, source_iterator);
      }
      group_->SetGroupStateList(group_state_list);
    }
    updateMoveUpDownButtons();

    emit groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>{group_});
  }
}

void GroupEditWidget::groupStateMoveDownButtonClicked()
{
  if (group_)
  {
    auto selected_indices = ui_.group_states_tree->selectionModel()->selectedRows();
    auto group_state_list = group_->GetGroupStateList();

    // Sort selected indices, so we begin with the bottom row
    std::sort(selected_indices.begin(), selected_indices.end(), [](const QModelIndex& i1, const QModelIndex& i2) { return i1.row() > i2.row(); });

    // If there are no indices, or the first one already is on the bottom, we are not able to move them
    if (!(selected_indices.size() == 0 || selected_indices.first().row() == (group_state_tree_model_->rowCount() - 1)))
    {
      for (auto& selected_index : selected_indices)
      {
        // Modify the treemodel
        group_state_tree_model_->moveItem(selected_index.row(), selected_index.row() + 1);

        // Modify the actual model
        auto source_iterator = group_state_list.begin();
        auto target_iterator = source_iterator;
        std::advance(source_iterator, selected_index.row());
        std::advance(target_iterator, selected_index.row() + 2);
        group_state_list.splice(target_iterator, group_state_list, source_iterator);
      }
      group_->SetGroupStateList(group_state_list);
    }
    updateMoveUpDownButtons();

    emit groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>{group_});
  }
}

void GroupEditWidget::stateNameEditingFinished()
{
  if (group_state_)
  {
    QString text = ui_.state_name_lineedit->text();
    if (text.toStdString() != group_state_->GetName())
    {
      group_state_->SetName(text.toStdString());
      group_state_tree_model_->updateGroupState(group_state_);
      emit groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>{group_});
    }
  }
}

void GroupEditWidget::stateColorButtonClicked()
{
  auto original_color = group_state_->GetColor();
  QColor color(original_color.red, original_color.green, original_color.blue);
  
  color = QColorDialog::getColor(color, this);

  if (color.isValid()) {
    TaskGroup::GroupState::Color state_color(color.red(), color.green(), color.blue());
    if (state_color != group_state_->GetColor())
    {
      group_state_->SetColor(state_color);
      const QString COLOR_STYLE("QPushButton { background-color : %1; }");
      ui_.state_color_button->setStyleSheet(COLOR_STYLE.arg(color.name()));
      emit groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>{group_});
    }
  }
}

void GroupEditWidget::stateAddTaskButtonClicked()
{
  if (group_ && group_state_)
  {
    // Map Proxy Model to actual model
    auto selected_proxy_indices = ui_.state_available_tasks_tree->selectionModel()->selectedRows();
    QModelIndexList selected_indices;
    for (auto& proxy_index : selected_proxy_indices)
    {
      selected_indices.push_back(available_tasks_proxy_tree_model_->mapToSource(proxy_index));
    }

    // Add the selected tasks to the group_state_ and the active_tasks_tree_model_
    for (auto& index : selected_indices)
    {
      QAbstractTreeItem* tree_item = available_tasks_tree_model_->item(index);
      if (tree_item && (tree_item->type() == (int)TreeItemType::Task))
      {
        TaskTreeItem* task_tree_item = static_cast<TaskTreeItem*>(tree_item);
        TaskState minimal_task_state;
        minimal_task_state.severity = eCAL_Process_eSeverity::proc_sev_healthy;
        minimal_task_state.severity_level = eCAL_Process_eSeverity_Level::proc_sev_level1;
        std::pair<std::shared_ptr<EcalSysTask>, TaskState> min_state(task_tree_item->getTask(), minimal_task_state);

        auto minimal_states_list = group_state_->GetMinimalStatesList();
        minimal_states_list.push_back(min_state);
        group_state_->SetMinimalStatesList(minimal_states_list);
        active_tasks_tree_model_->addMinTaskState(min_state);
      }
    }

    // Remove the selected tasks from the available tasks
    available_tasks_tree_model_->removeItems(selected_indices);

    emit groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>{group_});
  }
}

void GroupEditWidget::stateRemoveTaskButtonClicked()
{
  if (group_ && group_state_)
  {
    // Map Proxy Model to actual model
    auto selected_proxy_indices = ui_.state_active_tasks_tree->selectionModel()->selectedRows();
    QModelIndexList selected_indices;
    for (auto& proxy_index : selected_proxy_indices)
    {
      selected_indices.push_back(active_tasks_proxy_tree_model_->mapToSource(proxy_index));
    }

    // Remove the selected tasks from the group_state_ and add them to the available_tasks_tree_model_
    for (auto& index : selected_indices)
    {
      QAbstractTreeItem* tree_item = active_tasks_tree_model_->item(index);
      if (tree_item && (tree_item->type() == (int)TreeItemType::GroupStateMinTaskState))
      {
        GroupStateMinTaskStateTreeItem* min_state_tree_item = static_cast<GroupStateMinTaskStateTreeItem*>(tree_item);
        auto min_state = min_state_tree_item->getMinTaskState();

        auto minimal_states_list = group_state_->GetMinimalStatesList();
        minimal_states_list.remove(min_state);
        group_state_->SetMinimalStatesList(minimal_states_list);

        available_tasks_tree_model_->addTask(min_state.first);
      }
    }

    // Remove the selected tasks from the active tasks
    active_tasks_tree_model_->removeItems(selected_indices);

    emit groupsModifiedSignal(std::vector<std::shared_ptr<TaskGroup>>{group_});
  }
}

////////////////////////////////////////////////////////////////////////////////
//// External control slots                                                 ////
////////////////////////////////////////////////////////////////////////////////

void GroupEditWidget::ecalsysOptionsChanged()
{
  updateTasks();
}

void GroupEditWidget::monitorUpdated()
{
  updateTasks();
}

void GroupEditWidget::tasksModified(const std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  for (auto& task : task_list)
  {
    // The task might not even be in the available tasks list, but in that case, the update simply does nothing.
    available_tasks_tree_model_->updateTask(task);
    active_tasks_tree_model_->updateTask(task);
  }
}

void GroupEditWidget::tasksAdded(const std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  // First check if we are currently editing anything
  if (group_state_)
  {
    for (auto& task : task_list)
    {
      // The task cannot be used in any group, as it has just been added. Thus we add it to the list of available tasks without further checks
      available_tasks_tree_model_->addTask(task);
    }
  }
}

void GroupEditWidget::tasksRemoved(const std::vector<std::shared_ptr<EcalSysTask>>& task_list)
{
  // First check if we are currently editing anything
  if (group_state_)
  {
    for (auto& task : task_list)
    {
      // We only remove it from the list of available tasks. Whoever removed that task will also send the groupsModified Signal for all Groups that it had to remove the task from.
      available_tasks_tree_model_->removeTask(task);
    }
  }
}

void GroupEditWidget::groupsModified(std::vector<std::shared_ptr<TaskGroup>> group_list)
{
  // Check if the current group is in the list of modified groups
  if (group_ && (std::find(group_list.begin(), group_list.end(), group_) != group_list.end()))
  {
    setGroup(group_);
    groupStateSelectionChanged();
  }
}