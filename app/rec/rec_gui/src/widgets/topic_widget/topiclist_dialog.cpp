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

#include "topiclist_dialog.h"

#include <QSize>
#include <QInputDialog>
#include <QMenu>
#include <QAction>

#include <models/item_data_roles.h>

#include <set>

TopicListDialog::TopicListDialog(eCAL::rec::RecordMode mode, std::set<std::string> initial_topic_list, QWidget *parent)
  : QDialog(parent)
  , mode_(mode)
{
  ui_.setupUi(this);

  connect(ui_.show_hide_visible_topics_button, &QAbstractButton::clicked, this, &TopicListDialog::toggleVisibleTopicsWidgetHidden);

  // Set Maxi size for the button row and show / hide button, so they will not get resized when moving the splitter
  int default_button_height = QPushButton().sizeHint().height();
  auto layout_margin = ui_.button_widget_layout->contentsMargins();
  ui_.button_widget->setMaximumSize(QSize(0xFFFFFF, default_button_height + layout_margin.bottom() + layout_margin.top()));
  ui_.show_hide_visible_topics_button->setMaximumHeight(default_button_height);

  // Models
  selected_topics_model_ = new QStandardItemModel(this);
  selected_topics_model_->setColumnCount(1);
  for (const auto& topic : initial_topic_list)
  {
    addTopicToList(topic.c_str());
  }

  selected_topics_proxy_model_ = new QStableSortFilterProxyModel(this);
  selected_topics_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  selected_topics_proxy_model_->setFilterKeyColumn      (0);
  selected_topics_proxy_model_->setSortCaseSensitivity  (Qt::CaseSensitivity::CaseInsensitive);
  selected_topics_proxy_model_->setDynamicSortFilter    (false);
  selected_topics_proxy_model_->setSortRole             (ItemDataRoles::SortRole);
  selected_topics_proxy_model_->setFilterRole           (ItemDataRoles::FilterRole);
  selected_topics_proxy_model_->setSourceModel          (selected_topics_model_);

  ui_.topics_treeview->setModel(selected_topics_proxy_model_);

  visible_topics_model_ = new TopicListModel(mode, this);
  connect(QEcalRec::instance(), &QEcalRec::monitorUpdatedSignal, this, &TopicListDialog::monitorUpdated);
  visible_topics_model_->reset(QEcalRec::instance()->topicInfo(), initial_topic_list, false);

  visible_topics_proxy_model_ = new QMulticolumnSortFilterProxyModel(this);
  visible_topics_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  visible_topics_proxy_model_->setFilterKeyColumn      ((int)TopicListModel::Columns::NAME);
  visible_topics_proxy_model_->setSortCaseSensitivity  (Qt::CaseSensitivity::CaseInsensitive);
  visible_topics_proxy_model_->setSortRole             (ItemDataRoles::SortRole);
  visible_topics_proxy_model_->setFilterRole           (ItemDataRoles::FilterRole);
  visible_topics_proxy_model_->setSourceModel          (visible_topics_model_);

  if (QEcalRec::instance()->showDisabledElementsAtEnd())
    visible_topics_proxy_model_->setAlwaysSortedColumn((int)TopicListModel::Columns::RECORDING_ENABLED, Qt::SortOrder::DescendingOrder);

  ui_.visible_topics_treeview->setModel(visible_topics_proxy_model_);
  
  connect(QEcalRec::instance(), &QEcalRec::showDisabledElementsAtEndChanged, this
          , [this](bool show_at_end)
            {
              if (show_at_end)
                visible_topics_proxy_model_->setAlwaysSortedColumn((int)TopicListModel::Columns::RECORDING_ENABLED, Qt::SortOrder::DescendingOrder);
              else
                visible_topics_proxy_model_->setAlwaysSortedColumn(-1);
            });

  // Alternating row colors
  ui_.topics_treeview         ->setAlternatingRowColors(QEcalRec::instance()->alternatingRowColorsEnabled());
  ui_.visible_topics_treeview->setAlternatingRowColors(QEcalRec::instance()->alternatingRowColorsEnabled());
  connect(QEcalRec::instance(), &QEcalRec::alternatingRowColorsEnabledChanged, ui_.topics_treeview,         &QTreeView::setAlternatingRowColors);
  connect(QEcalRec::instance(), &QEcalRec::alternatingRowColorsEnabledChanged, ui_.visible_topics_treeview, &QTreeView::setAlternatingRowColors);

  connect(selected_topics_model_, &QAbstractItemModel::dataChanged,  this, &TopicListDialog::updateListedTopicsInVisibleTopics);
  connect(selected_topics_model_, &QAbstractItemModel::modelReset,   this, &TopicListDialog::updateListedTopicsInVisibleTopics);
  connect(selected_topics_model_, &QAbstractItemModel::rowsInserted, this, &TopicListDialog::updateListedTopicsInVisibleTopics);
  connect(selected_topics_model_, &QAbstractItemModel::rowsRemoved,  this, &TopicListDialog::updateListedTopicsInVisibleTopics);

  // Tree Views
  ui_.topics_treeview        ->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  ui_.visible_topics_treeview->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

  ui_.visible_topics_treeview->hideColumn((int)TopicListModel::Columns::RECORDING_ENABLED);

  connect(ui_.topics_treeview,         &QAbstractItemView::customContextMenuRequested, this, &TopicListDialog::listedTopicsContextMenu);
  connect(ui_.visible_topics_treeview, &QAbstractItemView::customContextMenuRequested, this, &TopicListDialog::visibleTopicsContextMenu);

  connect(ui_.topics_treeview, &QAdvancedTreeView::keySequenceDeletePressed, this, &TopicListDialog::removeSelectedTopics);

  ui_.topics_treeview        ->sortByColumn((int)TopicListModel::Columns::NAME, Qt::SortOrder::AscendingOrder);
  ui_.visible_topics_treeview->sortByColumn((int)TopicListModel::Columns::NAME, Qt::SortOrder::AscendingOrder);

  // Filter lineedits
  ui_.topic_filter_lineedit         ->setClearIcon(QIcon(":/ecalicons/FILTER_CANCEL"));
  ui_.visible_topics_filter_lineedit->setClearIcon(QIcon(":/ecalicons/FILTER_CANCEL"));

  connect(ui_.visible_topics_filter_lineedit, &QLineEdit::textChanged, visible_topics_proxy_model_,  [this](const QString& text) { visible_topics_proxy_model_ ->setFilterFixedString(text); });
  connect(ui_.topic_filter_lineedit,          &QLineEdit::textChanged, selected_topics_proxy_model_, [this](const QString& text) { selected_topics_proxy_model_->setFilterFixedString(text); });

  // Buttons
  connect(ui_.new_button,      &QAbstractButton::clicked, this, &TopicListDialog::addNewTopic);
  connect(ui_.add_list_button, &QAbstractButton::clicked, this, &TopicListDialog::openTopicInputDialog);
  connect(ui_.remove_button,   &QAbstractButton::clicked, this, &TopicListDialog::removeSelectedTopics);
  connect(ui_.clear_button,    &QAbstractButton::clicked, this, &TopicListDialog::removeAllTopics);

  connect(ui_.add_selected_visible_topics_button, &QAbstractButton::clicked, this, &TopicListDialog::addSelectedVisibleTopicsToList);

  connect(ui_.topics_treeview->selectionModel(),         &QItemSelectionModel::selectionChanged, this, &TopicListDialog::updateListedTopicsButtons);
  connect(selected_topics_model_,                        &QAbstractItemModel::rowsInserted,      this, &TopicListDialog::updateListedTopicsButtons);
  connect(selected_topics_model_,                        &QAbstractItemModel::modelReset,        this, &TopicListDialog::updateListedTopicsButtons);
  connect(selected_topics_model_,                        &QAbstractItemModel::rowsRemoved,       this, &TopicListDialog::updateListedTopicsButtons);

  connect(ui_.visible_topics_treeview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TopicListDialog::updateVisibleTopicsButtons);
  connect(visible_topics_model_,                         &QAbstractItemModel::rowsInserted,      this, &TopicListDialog::updateVisibleTopicsButtons);
  connect(visible_topics_model_,                         &QAbstractItemModel::modelReset,        this, &TopicListDialog::updateVisibleTopicsButtons);
  connect(visible_topics_model_,                         &QAbstractItemModel::rowsRemoved,       this, &TopicListDialog::updateVisibleTopicsButtons);

  // Mode
  if (mode_ == eCAL::rec::RecordMode::Blacklist)
  {
    ui_.header_label->setText(tr("Blacklist"));
    setWindowTitle(tr("Blacklist"));
    ui_.add_selected_visible_topics_button->setText(tr("Add to blacklist"));
    selected_topics_model_->setHeaderData(0, Qt::Orientation::Horizontal, "Blacklisted topics", Qt::ItemDataRole::DisplayRole);
  }
  else if (mode_ == eCAL::rec::RecordMode::Whitelist)
  {
    ui_.header_label->setText(tr("Whitelist"));
    setWindowTitle(tr("Whitelist"));
    ui_.add_selected_visible_topics_button->setText(tr("Add to whitelist"));
    selected_topics_model_->setHeaderData(0, Qt::Orientation::Horizontal, "Whitelisted topics", Qt::ItemDataRole::DisplayRole);
  }

  // Initial layout
  setVisibleTopicsHidden(true);
  updateListedTopicsButtons();
  updateVisibleTopicsButtons();
}

TopicListDialog::~TopicListDialog()
{}

std::set<std::string> TopicListDialog::getTopicList()
{
  std::set<std::string> topic_list;
  for (int i = 0; i < selected_topics_model_->rowCount(); i++)
  {
    QModelIndex index = selected_topics_model_->index(i, (int)TopicListModel::Columns::NAME);
    topic_list.emplace(selected_topics_model_->data(index, Qt::ItemDataRole::DisplayRole).toString().toStdString());
  }
  return topic_list;
}

void TopicListDialog::toggleVisibleTopicsWidgetHidden()
{
  setVisibleTopicsHidden(ui_.visible_topics_widget->isVisible());
}

void TopicListDialog::setVisibleTopicsHidden(bool hidden)
{
  ui_.visible_topics_widget->setHidden(hidden);

  if (hidden)
  {
    //                                                          ▲ Visible topics ▲
    ui_.show_hide_visible_topics_button->setText(tr("\342\226\262 Visible topics \342\226\262"));
  }
  else
  {
    //                                                          ▼ Hide topics ▼
    ui_.show_hide_visible_topics_button->setText(tr("\342\226\274 Hide topics \342\226\274"));
  }
}

void TopicListDialog::updateListedTopicsButtons()
{
  ui_.remove_button->setEnabled(ui_.topics_treeview->selectionModel()->selectedRows().size() > 0);
  ui_.clear_button ->setEnabled(selected_topics_model_->rowCount() > 0);
}

void TopicListDialog::updateVisibleTopicsButtons()
{
  ui_.add_selected_visible_topics_button->setEnabled(ui_.visible_topics_treeview->selectionModel()->selectedRows().size() > 0);
}

void TopicListDialog::monitorUpdated(const std::map<std::string, eCAL::rec_server::TopicInfo>& topic_info_map)
{
  visible_topics_model_->updateVisibleTopics(topic_info_map, true);
  visible_topics_model_->clean(true, false);
}

void TopicListDialog::updateListedTopicsInVisibleTopics()
{
  std::set<std::string> listed_topics = getTopicList();
  visible_topics_model_->updateListedTopics(listed_topics, false);
  visible_topics_model_->clean(true, false);
}

QModelIndex TopicListDialog::addTopicToList(const QString& topic_name)
{
  QStandardItem* topic_item = new QStandardItem(topic_name);
  selected_topics_model_->insertRow(selected_topics_model_->rowCount(), topic_item);

  return selected_topics_model_->index(selected_topics_model_->rowCount() - 1, 0);
}

void TopicListDialog::addNewTopic()
{
  // Clear Filter, so the new topic will not disappear
  ui_.topic_filter_lineedit->clear();

  // Create a new topic and edit it
  auto index = addTopicToList("Topic");

  auto proxy_index = selected_topics_proxy_model_->mapFromSource(index);

  ui_.topics_treeview->scrollTo(proxy_index);
  ui_.topics_treeview->selectionModel()->select(proxy_index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
  ui_.topics_treeview->edit(proxy_index);
}

void TopicListDialog::editSelection()
{
  QModelIndexList selected_proxy_indexes = ui_.topics_treeview->selectionModel()->selectedRows();
  if (!selected_proxy_indexes.empty())
  {
    ui_.topics_treeview->scrollTo(selected_proxy_indexes.first());
    ui_.topics_treeview->edit(selected_proxy_indexes.first());
  }
}

void TopicListDialog::removeSelectedTopics()
{
  QModelIndexList selected_proxy_indexes = ui_.topics_treeview->selectionModel()->selectedRows();
  std::set<int> selected_source_rows;
  for (const auto& proxy_index : selected_proxy_indexes)
  {
    selected_source_rows.emplace(selected_topics_proxy_model_->mapToSource(proxy_index).row());
  }

  // Iterate from back to front, as removing a row would otherwise shift the indices
  auto row_it = selected_source_rows.rbegin();
  while (row_it != selected_source_rows.rend())
  {
    selected_topics_model_->removeRow(*row_it);
    ++row_it;
  }
}

void TopicListDialog::removeAllTopics()
{
  selected_topics_model_->removeRows(0, selected_topics_model_->rowCount());
}

void TopicListDialog::openTopicInputDialog()
{
  QString topics = QInputDialog::getMultiLineText(this, "Input topcis", "Enter / Paste topics, one entry per line");
  for (const QString& topic : topics.split("\n"))
  {
    if (!topic.isEmpty())
    {
      addTopicToList(topic);
    }
  }
}

void TopicListDialog::listedTopicsContextMenu(const QPoint& pos)
{
  QMenu context_menu;

  QAction* new_topic_action     = new QAction("New Topic", &context_menu);
  QAction* edit_topic_action    = new QAction("Edit", &context_menu);
  QAction* remove_topics_action = new QAction("Remove", &context_menu);
  QAction* clear_all_action     = new QAction("Clear all", &context_menu);

  new_topic_action    ->setIcon(QIcon(":/ecalicons/ADD"));
  edit_topic_action   ->setIcon(QIcon(":/ecalicons/EDIT"));
  remove_topics_action->setIcon(QIcon(":/ecalicons/REMOVE"));
  clear_all_action    ->setIcon(QIcon(":/ecalicons/CLEAR"));

  connect(new_topic_action,     &QAction::triggered, this, &TopicListDialog::addNewTopic);
  connect(edit_topic_action,    &QAction::triggered, this, &TopicListDialog::editSelection);
  connect(remove_topics_action, &QAction::triggered, this, &TopicListDialog::removeSelectedTopics);
  connect(clear_all_action,     &QAction::triggered, this, &TopicListDialog::removeAllTopics);

  remove_topics_action->setEnabled(ui_.topics_treeview->selectionModel()->selectedRows().size() > 0);
  edit_topic_action   ->setEnabled(ui_.topics_treeview->selectionModel()->selectedRows().size() == 1);
  clear_all_action    ->setEnabled(ui_.topics_treeview->model()->rowCount() > 0);

  context_menu.addAction(new_topic_action);
  context_menu.addAction(edit_topic_action);
  context_menu.addAction(remove_topics_action);
  context_menu.addSeparator();
  context_menu.addAction(clear_all_action);

  context_menu.exec(ui_.topics_treeview->viewport()->mapToGlobal(pos));
}

void TopicListDialog::visibleTopicsContextMenu(const QPoint& pos)
{
  QMenu context_menu;

  QAction* add_to_list_action = new QAction(&context_menu);
  if (mode_ == eCAL::rec::RecordMode::Whitelist)
  {
    add_to_list_action->setText(tr("Add to whitelist"));
  }
  else if (mode_ == eCAL::rec::RecordMode::Blacklist)
  {
    add_to_list_action->setText(tr("Add to blacklist"));
  }

  add_to_list_action->setIcon(QIcon(":/ecalicons/ADD"));

  connect(add_to_list_action, &QAction::triggered, this, &TopicListDialog::addSelectedVisibleTopicsToList);

  add_to_list_action->setEnabled(ui_.visible_topics_treeview->selectionModel()->selectedRows().size() > 0);

  context_menu.addAction(add_to_list_action);

  context_menu.exec(ui_.visible_topics_treeview->viewport()->mapToGlobal(pos));
}

void TopicListDialog::addSelectedVisibleTopicsToList()
{
  std::set<QString> topics;
  QModelIndexList selected_proxy_indexes = ui_.visible_topics_treeview->selectionModel()->selectedRows((int)TopicListModel::Columns::NAME);

  for (const auto& proxy_index : selected_proxy_indexes)
  {
    topics.emplace(visible_topics_proxy_model_->data(proxy_index, Qt::ItemDataRole::DisplayRole).toString());
  }

  for (const QString& topic : topics)
  {
    addTopicToList(topic);
  }
}
