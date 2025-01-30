/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "channel_widget.h"

#include "widgets/models/tree_item_type.h"

#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QList>
#include <QSettings>
#include <QMessageBox>

#include <ecal_play_globals.h>

#include "widgets/models/item_data_roles.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/monitoring.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

ChannelWidget::ChannelWidget(QWidget *parent)
  : QWidget(parent)
  , init_button_state_is_init_(true)
  , channel_model_(nullptr)
  , channel_proxy_model_(nullptr)
  , message_counter_update_timer_(nullptr)
{
  ui_.setupUi(this);

  // Channel Tree
  channel_model_ = new ChannelTreeModel(this);
  channel_proxy_model_ = new QStableSortFilterProxyModel(this);
  channel_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  channel_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  channel_proxy_model_->setFilterKeyColumn((int)ChannelTreeModel::Columns::CHANNEL_NAME);
  channel_proxy_model_->setFilterRole(ItemDataRoles::FilterRole);
  channel_proxy_model_->setSortRole(ItemDataRoles::SortRole);
  channel_proxy_model_->setDynamicSortFilter(false);
  channel_proxy_model_->setSourceModel(channel_model_);

  ui_.channel_tree_view->setModel(channel_proxy_model_);

  // Channel tree Header
  tree_header_ = new QCheckboxHeaderView(ui_.channel_tree_view);
  tree_header_->setStretchLastSection(true);
  tree_header_->setCheckable((int)ChannelTreeModel::Columns::ENABLED);
  tree_header_->setStretchLastSection(false);
  ui_.channel_tree_view->setHeader(tree_header_);

  ui_.channel_tree_view->sortByColumn((int)ChannelTreeModel::Columns::CHANNEL_NAME, Qt::SortOrder::AscendingOrder);

  ui_.channel_tree_view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

  connect(tree_header_,          &QCheckboxHeaderView::clicked,                this, &ChannelWidget::headerClicked);
  connect(channel_model_,        &QAbstractItemModel::dataChanged,             this, &ChannelWidget::treeDataChanged);
  connect(ui_.channel_tree_view, &QTreeView::customContextMenuRequested,       this, &ChannelWidget::treeContextMenu);
  connect(ui_.channel_tree_view, &QAdvancedTreeView::headerContextMenuChanged, this, &ChannelWidget::headerContextMenu);

  // Set initial layout
  autoSizeColumns();

  // Prevent the user from hiding the checkbox and name columns
  ui_.channel_tree_view->setForcedColumns(QVector<int>{
    (int)ChannelTreeModel::Columns::ENABLED,
    (int)ChannelTreeModel::Columns::CHANNEL_NAME,
  });

  // Default visible columns
  QList<int> visible_columns
  {
    (int)ChannelTreeModel::Columns::ENABLED,
    (int)ChannelTreeModel::Columns::CHANNEL_NAME,
    (int)ChannelTreeModel::Columns::EXISTING_FRAMES,
    (int)ChannelTreeModel::Columns::MESSAGE_COUNTER,
  };
  for (int i = 0; i < channel_model_->columnCount(); i++)
  {
    ui_.channel_tree_view->setColumnHidden(i, !visible_columns.contains(i));
  }

  // Set the current index to the channel name for easier keyboard lookup
  connect(ui_.channel_tree_view->selectionModel(), &QItemSelectionModel::currentColumnChanged, 
      [this](const QModelIndex& current)
      {
        if ((ChannelTreeModel::Columns)(current.column()) != ChannelTreeModel::Columns::CHANNEL_NAME)
        {
          QModelIndex new_current_index = channel_proxy_model_->index(current.row(), (int)ChannelTreeModel::Columns::CHANNEL_NAME);
          ui_.channel_tree_view->selectionModel()->setCurrentIndex(new_current_index, QItemSelectionModel::NoUpdate);
        }
      });

  // Filter input
  ui_.filter_lineedit->setClearIcon(QIcon(":/ecalicons/FILTER_CANCEL"));
  connect(ui_.filter_lineedit, &QLineEdit::textChanged, [this](const QString& text) { channel_proxy_model_->setFilterFixedString(text); });

  // connect QEcalPlay -> this
  connect(QEcalPlay::instance(), &QEcalPlay::measurementLoadedSignal,           this, &ChannelWidget::measurementLoaded);
  connect(QEcalPlay::instance(), &QEcalPlay::measurementClosedSignal,           this, &ChannelWidget::measurementClosed);
  connect(QEcalPlay::instance(), &QEcalPlay::publishersInitStateChangedSignal,  this, &ChannelWidget::publishersInitStateChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::channelMappingChangedSignal,       this, &ChannelWidget::channelMappingChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::channelMappingLoadedSignal,        this, &ChannelWidget::channelMappingChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::playStateChangedSignal,            this, [this]() {channel_model_->updateMessageCounters(); });

  // connect this -> QEcalPlay
  connect(ui_.initialize_button, &QPushButton::clicked, QEcalPlay::instance(),
      [this]()
      {
        if (init_button_state_is_init_)
          QEcalPlay::instance()->initializePublishers();
        else
          QEcalPlay::instance()->deInitializePublishers();
      });

  if (QEcalPlay::instance()->isMeasurementLoaded())
  {
    measurementLoaded(QEcalPlay::instance()->measurementPath());
  }
  updateHeader();
  updateSelectionCountLabel();

  // Save layout
  saveInitialLayout();
  restoreLayout();
}

ChannelWidget::~ChannelWidget()
{
  saveLayout();
}

////////////////////////////////////////////////////////////////////////////////
//// Slots for QEcalPlay                                                    ////
////////////////////////////////////////////////////////////////////////////////

void ChannelWidget::measurementLoaded(const QString& /*path*/)
{
  channels_total_size_calculated_ = false;
  channel_model_->reload();
  ui_.filter_lineedit->clear();
  updateHeader();
  updateSelectionCountLabel();

  QEcalPlay::instance()->setChannelMapping(getChannelMapping(), true);

  channel_model_->updateMessageCounters();

  headerContextMenu();
}

void ChannelWidget::measurementClosed()
{
  channel_model_->reload();
  ui_.filter_lineedit->clear();
  updateHeader();
  updateSelectionCountLabel();
  QEcalPlay::instance()->setChannelMapping(getChannelMapping(), true);

  channel_model_->updateMessageCounters();

  channels_total_size_calculated_ = false;
}

void ChannelWidget::publishersInitStateChanged(bool publishers_initialized)
{
  if (publishers_initialized)
  {
    setInitButtonToDeInit();

    tree_header_->setCheckboxEnabled(0, false);
    channel_model_->setEditEnabled(false);
  }
  else
  {
    setInitButtonToInit();

    tree_header_->setCheckboxEnabled(0, true);
    channel_model_->setEditEnabled(true);
  }

  channel_model_->updateMessageCounters();
}

void ChannelWidget::channelMappingChanged(const std::map<std::string, std::string>& channel_mapping)
{
  // Update Tree
  channel_model_->blockSignals(true);
  for (int i = 0; i < channel_model_->rowCount(); i++)
  {
    QAbstractTreeItem* tree_item = channel_model_->root()->child(i);
    if (tree_item->type() == (int)TreeItemType::Channel)
    {
      ChannelTreeItem* channel_item = static_cast<ChannelTreeItem*>(tree_item);
      std::string source_name = channel_item->sourceChannelName().toStdString();
      if (channel_mapping.find(source_name) != channel_mapping.end())
      {
        // The element must be activated
        if (!channel_item->enabled())
        {
          channel_model_->setData(channel_model_->index(channel_item, (int)ChannelTreeModel::Columns::ENABLED), true, Qt::ItemDataRole::CheckStateRole);
        }
        QString target_channel(channel_mapping.at(source_name).c_str());
        if (target_channel != channel_item->targetChannelName())
        {
          channel_model_->setData(channel_model_->index(channel_item, (int)ChannelTreeModel::Columns::CHANNEL_NAME), target_channel, Qt::ItemDataRole::EditRole);
        }
      }
      else
      {
        if (channel_item->enabled())
        {
          channel_item->setEnabled(false);
        }
      }
    }
  }
  channel_model_->blockSignals(false);

  // Update Buttons
  if (channel_mapping.empty() && ui_.initialize_button->isEnabled())
  {
    ui_.initialize_button->setEnabled(false);
  }
  else if (!channel_mapping.empty() && !ui_.initialize_button->isEnabled())
  {
    ui_.initialize_button->setEnabled(true);
  }
}

////////////////////////////////////////////////////////////////////////////////
//// Internal                                                               ////
////////////////////////////////////////////////////////////////////////////////

void ChannelWidget::headerClicked(int column, bool checked)
{

  if (column == (int)ChannelTreeModel::Columns::ENABLED)
  {
    channel_model_->setAllChecked(checked);
  }
}

void ChannelWidget::updateHeader()
{
  int checked_item_count = getCheckedChannels().size();

  if (checked_item_count == 0)
  {
    tree_header_->setCheckState((int)ChannelTreeModel::Columns::ENABLED, Qt::CheckState::Unchecked);
  }
  else if (checked_item_count == channel_model_->rowCount())
  {
    tree_header_->setCheckState((int)ChannelTreeModel::Columns::ENABLED, Qt::CheckState::Checked);
  }
  else
  {
    tree_header_->setCheckState((int)ChannelTreeModel::Columns::ENABLED, Qt::CheckState::PartiallyChecked);
  }
}

void ChannelWidget::updateSelectionCountLabel()
{
  int checked_item_count = getCheckedChannels().size();
  ui_.selected_count_label->setText(tr("Selected ") + QString::number(checked_item_count) + " / " + QString::number(channel_model_->rowCount()));
}

void ChannelWidget::treeDataChanged(const QModelIndex &top_left, const QModelIndex &bottom_right, const QVector<int> &roles)
{
  if (roles.empty() || roles.contains(Qt::ItemDataRole::CheckStateRole))
  {
    updateHeader();
    updateSelectionCountLabel();
  }

  if (!((top_left.column() == (int)ChannelTreeModel::Columns::MESSAGE_COUNTER)
    && (bottom_right.column() == (int)ChannelTreeModel::Columns::MESSAGE_COUNTER)))
  {
    QEcalPlay::instance()->setChannelMapping(getChannelMapping());
  }
}

void ChannelWidget::headerContextMenu()
{
  if (channels_total_size_calculated_)
    return;

  if (QEcalPlay::instance()->isMeasurementLoaded())
  {
    if (!ui_.channel_tree_view->isColumnHidden((int)ChannelTreeModel::Columns::TOTAL_CHANNEL_SIZE))
    {
      QEcalPlay::instance()->calculateChannelsCumulativeEstimatedSize();
      channels_total_size_calculated_ = true;
      channel_model_->reload();
    }
  }
}

void ChannelWidget::treeContextMenu(const QPoint &pos)
{
  QList<ChannelTreeItem*> selected_channels = getSelectedChannels();

  QMenu context_menu;

  QAction* edit_action              = new QAction(tr("Edit"),                          &context_menu);

  QAction* select_action            = new QAction(tr("Select"),                        &context_menu);
  QAction* deselect_action          = new QAction(tr("Deselect"),                      &context_menu);
  QAction* revert_renaming_action   = new QAction(tr("Revert renaming"),               &context_menu);

  QAction* select_all_action        = new QAction(tr("Select all"),                    &context_menu);
  QAction* deselect_all_action      = new QAction(tr("Deselect all"),                  &context_menu);
  QAction* invert_selection_action  = new QAction(tr("Invert Selection"),              &context_menu);

  QAction* deselect_external_action = new QAction(tr("Deselect externally published"), &context_menu);

  context_menu.addAction(edit_action);
  context_menu.addSeparator();
  context_menu.addAction(select_action);
  context_menu.addAction(deselect_action);
  context_menu.addAction(revert_renaming_action);
  context_menu.addSeparator();
  context_menu.addAction(select_all_action);
  context_menu.addAction(deselect_all_action);
  context_menu.addSeparator();
  context_menu.addAction(invert_selection_action);
  context_menu.addAction(deselect_external_action);

  if ((selected_channels.size() != 1) || QEcalPlay::instance()->isInitialized())
  {
    edit_action->setEnabled(false);
  }

  if ((channel_model_->rowCount() == 0) || QEcalPlay::instance()->isInitialized())
  {
    select_action          ->setEnabled(false);
    deselect_action        ->setEnabled(false);
    revert_renaming_action ->setEnabled(false);
    select_all_action      ->setEnabled(false);
    deselect_all_action    ->setEnabled(false);
    invert_selection_action->setEnabled(false);
    deselect_external_action->setEnabled(false);
  }
  else
  {
    if (selected_channels.size() == 0)
    {
      select_action          ->setEnabled(false);
      deselect_action        ->setEnabled(false);
      revert_renaming_action ->setEnabled(false);
    }
  }

  connect(select_action,           &QAction::triggered, this, [this]() {setSelectedChecked(true); });
  connect(deselect_action,         &QAction::triggered, this, [this]() {setSelectedChecked(false); });
  connect(revert_renaming_action,  &QAction::triggered, this, [this]() {revertRenamingSelected(); });
  connect(select_all_action,       &QAction::triggered, this, [this]() {channel_model_->setAllChecked(true); });
  connect(deselect_all_action,     &QAction::triggered, this, [this]() {channel_model_->setAllChecked(false); });
  connect(invert_selection_action, &QAction::triggered, this, [this]() {channel_model_->invertCheckedState(); });
  connect(deselect_external_action, &QAction::triggered, this, &ChannelWidget::uncheckPublished);

  connect(edit_action, &QAction::triggered, this,
      [this]()
      {
        auto selected_rows = ui_.channel_tree_view->selectionModel()->selectedRows((int)ChannelTreeModel::Columns::CHANNEL_NAME);
        if (!selected_rows.empty())
        {
          auto selected_proxy_index = selected_rows.first();
          ui_.channel_tree_view->edit(selected_proxy_index);
        }
      });

  context_menu.exec(ui_.channel_tree_view->viewport()->mapToGlobal(pos));
}

void ChannelWidget::setSelectedChecked(bool checked)
{
  auto selected_channels = getSelectedChannels();
  for (auto& channel : selected_channels)
  {
    channel_model_->setData(channel_model_->index(channel, (int)ChannelTreeModel::Columns::ENABLED), checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
  }
}

void ChannelWidget::revertRenamingSelected()
{
  auto selected_channels = getSelectedChannels();
  for (auto& channel : selected_channels)
  {
    if (channel->sourceChannelName() != channel->targetChannelName())
    {
      channel_model_->setData(channel_model_->index(channel, (int)ChannelTreeModel::Columns::CHANNEL_NAME), channel->sourceChannelName(), Qt::EditRole);
    }
  }
}

void ChannelWidget::uncheckPublished()
{
  std::set<std::string> published_topic_names_;

  // Get the eCAL Monitoring information
  std::string monitoring_string;
  eCAL::pb::Monitoring monitoring_pb;

  if (eCAL::Monitoring::GetMonitoring(monitoring_string) && !monitoring_string.empty() && monitoring_pb.ParseFromString(monitoring_string))
  {
    // Fill the list of published topics
    for (const auto& topic : monitoring_pb.topics())
    {
      // Only respect published topics not published by eCAL Play itself
      if ((topic.direction() == "publisher") && !((topic.unit_name() == "eCALPlay") || (topic.unit_name() == "eCALPlayGUI")))
      {
        published_topic_names_.emplace(topic.topic_name());
      }
    }

    int unchecked_channels = 0;
    int already_unchecked_channels = 0;

    // Uncheck all names that are in that list
    for (int i = 0; i < channel_model_->rowCount(); i++)
    {
      QModelIndex channel_name_index = channel_model_->index(i, (int)ChannelTreeModel::Columns::CHANNEL_NAME);
      QAbstractTreeItem* tree_item = channel_model_->item(channel_name_index);
      if (tree_item && (tree_item->type() == (int)TreeItemType::Channel))
      {
        ChannelTreeItem* channel_item = static_cast<ChannelTreeItem*>(tree_item);
        std::string channel_name = channel_item->targetChannelName().toStdString();

        if (published_topic_names_.find(channel_name) != published_topic_names_.end())
        {
          QModelIndex enables_index = channel_model_->index(i, (int)ChannelTreeModel::Columns::ENABLED);
          
          if (channel_model_->data(enables_index, Qt::CheckStateRole) == Qt::CheckState::Unchecked)
          {
            already_unchecked_channels++;
          }
          else if (channel_model_->setData(enables_index, Qt::Unchecked, Qt::CheckStateRole))
          {
            unchecked_channels++;
          }
        }
      }
    }

    if ((unchecked_channels == 0) && (already_unchecked_channels == 0))
    {
      QMessageBox warning_message(
        QMessageBox::Icon::Warning
        , tr("Warning")
        , tr("The current measurement does not match any externally published channels.")
        , QMessageBox::Button::Ok
        , this);

      warning_message.exec();
    }
    else if (already_unchecked_channels == 0)
    {
      QMessageBox info_message(
        QMessageBox::Icon::Information
        , tr("Success")
        , tr("Deselected ") + QString::number(unchecked_channels) + tr(" channel") + (unchecked_channels > 1 ? "s." : ".")
        , QMessageBox::Button::Ok
        , this);

      info_message.exec();
    }
    else if (unchecked_channels == 0)
    {
      QMessageBox info_message(
        QMessageBox::Icon::Information
        , tr("Info")
        , QString::number(already_unchecked_channels) + tr(" channel") + (already_unchecked_channels > 1 ? "s where" : " was") + " already unselected."
        , QMessageBox::Button::Ok
        , this);

      info_message.exec();
    }
    else
    {
      QMessageBox info_message(
        QMessageBox::Icon::Information
        , tr("Success")
        , tr("Deselected ") + QString::number(unchecked_channels) + tr(" channel") + (unchecked_channels > 1 ? "s." : ".") + "\n" + QString::number(already_unchecked_channels) + tr(" channel") + (already_unchecked_channels > 1 ? "s where" : " was") + " already unselected."
        , QMessageBox::Button::Ok
        , this);

      info_message.exec();
    }
  }
  else
  {
    QMessageBox error_message(
      QMessageBox::Icon::Critical
      , tr("Error")
      , tr("Error getting eCAL Monitoring information.")
      , QMessageBox::Button::Ok
      , this);

    error_message.exec();
  }
}

QList<ChannelTreeItem*> ChannelWidget::getSelectedChannels() const
{
  QList<ChannelTreeItem*> selected_channels;

  auto selected_proxy_indexes = ui_.channel_tree_view->selectionModel()->selectedRows();
  for (auto& proxy_index : selected_proxy_indexes)
  {
    auto source_index = channel_proxy_model_->mapToSource(proxy_index);
    if (source_index.isValid())
    {
      QAbstractTreeItem* tree_item = channel_model_->item(source_index);
      if (tree_item && (tree_item->type() == (int)TreeItemType::Channel))
      {
        selected_channels.push_back(static_cast<ChannelTreeItem*>(tree_item));
      }
    }
  }
  return selected_channels;
}

QList<ChannelTreeItem*> ChannelWidget::getCheckedChannels() const
{
  QList<ChannelTreeItem*> channel_list;

  for (int i = 0; i < channel_model_->rowCount(); i++)
  {
    QModelIndex index = channel_model_->index(i, (int)ChannelTreeModel::Columns::ENABLED);
    QAbstractTreeItem* tree_item = channel_model_->item(index);
    bool is_checked = (channel_model_->data(index, Qt::ItemDataRole::CheckStateRole).toInt() == Qt::CheckState::Checked);

    if (is_checked && tree_item && (tree_item->type() == (int)TreeItemType::Channel))
    {
      channel_list.push_back(static_cast<ChannelTreeItem*>(tree_item));
    }

  }
  return channel_list;
}

std::map<std::string, std::string> ChannelWidget::getChannelMapping() const
{
  std::map<std::string, std::string> channel_map;

  auto checked_channels = getCheckedChannels();
  for (ChannelTreeItem* channel_item : checked_channels)
  {
    std::string source_name = channel_item->data(ChannelTreeItem::Columns::SOURCE_CHANNEL_NAME, Qt::ItemDataRole::DisplayRole).toString().toStdString();
    std::string target_name = channel_item->data(ChannelTreeItem::Columns::TARGET_CHANNEL_NAME, Qt::ItemDataRole::DisplayRole).toString().toStdString();
    channel_map[source_name] = target_name;
  }
  return channel_map;
}

void ChannelWidget::setInitButtonToInit()
{
  if (!init_button_state_is_init_)
  {
    ui_.initialize_button->setIcon(QPixmap(":/ecalicons/CHECKMARK"));
    ui_.initialize_button->setText(tr("Connect to eCAL"));
    ui_.initialize_button->setToolTip(tr("Create publishers for all enabled channels"));
    init_button_state_is_init_ = true;
  }
}

void ChannelWidget::setInitButtonToDeInit()
{
  if (init_button_state_is_init_)
  {
    ui_.initialize_button->setIcon(QPixmap(":/ecalicons/CROSS"));
    ui_.initialize_button->setText(tr("Disconnect from eCAL"));
    ui_.initialize_button->setToolTip(tr("Remove all publishers"));
    init_button_state_is_init_ = false;
  }
}

void ChannelWidget::autoSizeColumns()
{
  ChannelTreeItem* dummy_item = new ChannelTreeItem("CameraSensorMapFusionCAF___", "proto", "Dummy_type", 99999999, 99999999.0, 99999999.0, 99999999, 99999999, 99999999);
  channel_model_->insertItem(dummy_item);

  for (int i = 0; i < channel_model_->columnCount(); i++)
  {
    ui_.channel_tree_view->resizeColumnToContents(i);
  }
  channel_model_->removeItem(dummy_item);
}

////////////////////////////////////////////////////////////////////////////////
//// Save Layout                                                            ////
////////////////////////////////////////////////////////////////////////////////

void ChannelWidget::saveLayout()
{
  QSettings settings;
  settings.beginGroup("channels");
  settings.setValue("tree_state", ui_.channel_tree_view->saveState(EcalPlayGlobals::Version()));
  settings.endGroup();
}

void ChannelWidget::restoreLayout()
{
  QSettings settings;
  settings.beginGroup("channels");
  QVariant tree_state_variant = settings.value("tree_state");
  if (tree_state_variant.isValid())
  {
    ui_.channel_tree_view->restoreState(tree_state_variant.toByteArray(), EcalPlayGlobals::Version());
  }
  settings.endGroup();
}

void ChannelWidget::saveInitialLayout()
{
  initial_tree_state_ = ui_.channel_tree_view->saveState();
}

void ChannelWidget::resetLayout()
{
  ui_.channel_tree_view->restoreState(initial_tree_state_);
  ui_.filter_lineedit->clear();
}