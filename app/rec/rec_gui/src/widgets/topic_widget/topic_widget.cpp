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

#include "topic_widget.h"

#include "topiclist_dialog.h"

#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>

TopicWidget::TopicWidget(QWidget *parent)
  : QWidget(parent)
  , connect_to_ecal_button_state_is_connect_(true)
{
  ui_.setupUi(this);

  connect(QEcalRec::instance(), &QEcalRec::recordModeChangedSignal,     this, &TopicWidget::recordModeChanged);
  connect(QEcalRec::instance(), &QEcalRec::topicBlacklistChangedSignal, this, &TopicWidget::topicBlacklistChanged);
  connect(QEcalRec::instance(), &QEcalRec::topicWhitelistChangedSignal, this, &TopicWidget::topicWhitelistChanged);
  connect(QEcalRec::instance(), &QEcalRec::monitorUpdatedSignal,        this, &TopicWidget::monitorUpdated);

  connect(QEcalRec::instance(), &QEcalRec::recordModeChangedSignal,     this, &TopicWidget::updateRecordModeEnabledStates);
  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal, this, &TopicWidget::updateRecordModeEnabledStates);

  connect(ui_.record_mode_all_topics_radiobutton, &QRadioButton::toggled, QEcalRec::instance(),
    [this](bool checked) { if (checked) QEcalRec::instance()->setRecordMode(eCAL::rec::RecordMode::All); });
  connect(ui_.record_mode_whitelist_radiobutton, &QRadioButton::toggled, QEcalRec::instance(),
    [this](bool checked) { if (checked) QEcalRec::instance()->setRecordMode(eCAL::rec::RecordMode::Whitelist); });
  connect(ui_.record_mode_blacklist_radiobutton, &QRadioButton::toggled, QEcalRec::instance(),
    [this](bool checked) { if (checked) QEcalRec::instance()->setRecordMode(eCAL::rec::RecordMode::Blacklist); });

  connect(ui_.blacklist_button, &QAbstractButton::clicked, this, &TopicWidget::showBlacklistDialog);
  connect(ui_.whitelist_button, &QAbstractButton::clicked, this, &TopicWidget::showWhitelistDialog);

  // Topic list models
  auto topic_info_map = QEcalRec::instance()->monitorTopicInfo();
  auto blacklist      = QEcalRec::instance()->topicBlacklist();
  auto whitelist      = QEcalRec::instance()->topicWhitelist();

  all_topics_model_ = new TopicListModel(eCAL::rec::RecordMode::All,       this);
  blacklist_model_  = new TopicListModel(eCAL::rec::RecordMode::Blacklist, this);
  whitelist_model_  = new TopicListModel(eCAL::rec::RecordMode::Whitelist, this);

  all_topics_model_->reset(topic_info_map, {},        false);
  blacklist_model_ ->reset(topic_info_map, blacklist, true);
  whitelist_model_ ->reset(topic_info_map, whitelist, true);
  
  topics_hide_disabled_proxy_model_ = new QStableSortFilterProxyModel(this);
  topics_hide_disabled_proxy_model_->setFilterKeyColumn((int)TopicListModel::Columns::RECORDING_ENABLED);
  topics_hide_disabled_proxy_model_->setFilterFixedString("Yes");

  topics_user_filter_proxy_model_ = new QStableSortFilterProxyModel(this);
  topics_user_filter_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  topics_user_filter_proxy_model_->setFilterKeyColumn((int)TopicListModel::Columns::NAME);
  topics_user_filter_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  topics_user_filter_proxy_model_->setSourceModel(topics_hide_disabled_proxy_model_);

  ui_.topic_treeview->setModel(topics_user_filter_proxy_model_);

  // Tree view
  ui_.topic_treeview->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.topic_treeview, &QAbstractItemView::customContextMenuRequested, this, &TopicWidget::treeviewContextMenuRequested);
  connect(ui_.topic_treeview, &QAdvancedTreeView::keySequenceCopyPressed,     this, &TopicWidget::copySelectedTopicNames);

  // Hide disabled checkbox
  connect(ui_.hide_disabled_checkbox, &QCheckBox::toggled, topics_hide_disabled_proxy_model_, [this](bool checked) {topics_hide_disabled_proxy_model_->setFilterFixedString(checked ? "Yes" : ""); });

  // Recorded topics label
  connect(QEcalRec::instance(), &QEcalRec::recordModeChangedSignal,     this, &TopicWidget::updateTopicCounterLabel);
  connect(QEcalRec::instance(), &QEcalRec::topicBlacklistChangedSignal, this, &TopicWidget::updateTopicCounterLabel);
  connect(QEcalRec::instance(), &QEcalRec::topicWhitelistChangedSignal, this, &TopicWidget::updateTopicCounterLabel);
  connect(QEcalRec::instance(), &QEcalRec::monitorUpdatedSignal,        this, &TopicWidget::updateTopicCounterLabel);

  // Filter lineedit
  ui_.filter_lineedit->setClearIcon(QIcon(":/ecalicons/FILTER_CANCEL"));
  connect(ui_.filter_lineedit, &QLineEdit::textChanged, this, [this](const QString& text) {topics_user_filter_proxy_model_->setFilterFixedString(text); });

  // Initial state
  recordModeChanged     (QEcalRec::instance()->recordMode());
  topicWhitelistChanged(QEcalRec::instance()->topicWhitelist());
  topicBlacklistChanged(QEcalRec::instance()->topicBlacklist());

  ui_.topic_treeview->setColumnHidden((int)TopicListModel::Columns::RECORDING_ENABLED, true);
  updateRecordModeEnabledStates();
}

TopicWidget::~TopicWidget()
{}

void TopicWidget::recordModeChanged(eCAL::rec::RecordMode record_mode)
{
  switch (record_mode)
  {
  case eCAL::rec::RecordMode::All:
    if (!ui_.record_mode_all_topics_radiobutton->isChecked())
    {
      ui_.record_mode_all_topics_radiobutton->blockSignals(true);
      ui_.record_mode_all_topics_radiobutton->setChecked(true);
      ui_.record_mode_all_topics_radiobutton->blockSignals(false);
    }

    if (topics_hide_disabled_proxy_model_->sourceModel() != all_topics_model_)
    {
      topics_hide_disabled_proxy_model_->setSourceModel(all_topics_model_);
      ui_.filter_lineedit->clear();
    }


    break;

  case eCAL::rec::RecordMode::Blacklist:
    if (!ui_.record_mode_blacklist_radiobutton->isChecked())
    {
      ui_.record_mode_blacklist_radiobutton->blockSignals(true);
      ui_.record_mode_blacklist_radiobutton->setChecked(true);
      ui_.record_mode_blacklist_radiobutton->blockSignals(false);
    }

    if (topics_hide_disabled_proxy_model_->sourceModel() != blacklist_model_)
    {
      topics_hide_disabled_proxy_model_->setSourceModel(blacklist_model_);
      ui_.filter_lineedit->clear();
    }

    break;

  case eCAL::rec::RecordMode::Whitelist:
    if (!ui_.record_mode_whitelist_radiobutton->isChecked())
    {
      ui_.record_mode_whitelist_radiobutton->blockSignals(true);
      ui_.record_mode_whitelist_radiobutton->setChecked(true);
      ui_.record_mode_whitelist_radiobutton->blockSignals(false);
    }

    if (topics_hide_disabled_proxy_model_->sourceModel() != whitelist_model_)
    {
      topics_hide_disabled_proxy_model_->setSourceModel(whitelist_model_);
      ui_.filter_lineedit->clear();
    }

    break;

  default:
    break;
  }
}

void TopicWidget::updateRecordModeEnabledStates()
{
  if (QEcalRec::instance()->recordersRecording())
  {
    // While recording we cannot change anything
    ui_.record_mode_all_topics_radiobutton->setEnabled(false);
    ui_.record_mode_blacklist_radiobutton->setEnabled(false);
    ui_.record_mode_whitelist_radiobutton->setEnabled(false);

    ui_.blacklist_button->setEnabled(false);
    ui_.whitelist_button->setEnabled(false);

    ui_.hide_disabled_checkbox->setEnabled(false);
  }
  else
  {
    ui_.record_mode_all_topics_radiobutton->setEnabled(true);
    ui_.record_mode_blacklist_radiobutton->setEnabled(true);
    ui_.record_mode_whitelist_radiobutton->setEnabled(true);

    auto current_record_mode = QEcalRec::instance()->recordMode();
    switch (current_record_mode)
    {
    case eCAL::rec::RecordMode::All:
      ui_.blacklist_button->setEnabled(false);
      ui_.whitelist_button->setEnabled(false);
      ui_.hide_disabled_checkbox->setEnabled(false);
      break;

    case eCAL::rec::RecordMode::Blacklist:
      ui_.blacklist_button->setEnabled(true);
      ui_.whitelist_button->setEnabled(false);
      ui_.hide_disabled_checkbox->setEnabled(true);
      break;

    case eCAL::rec::RecordMode::Whitelist:
      ui_.blacklist_button->setEnabled(false);
      ui_.whitelist_button->setEnabled(true);
      ui_.hide_disabled_checkbox->setEnabled(true);
      break;

    default:
      break;
    }
  }
}

void TopicWidget::topicBlacklistChanged(const std::set<std::string>& blacklist)
{
  ui_.blacklist_button->setText(QString::number(blacklist.size()) + tr(" topics blacklisted"));

  blacklist_model_->updateListedTopics(blacklist, true);
  blacklist_model_->clean(true, true);
}

void TopicWidget::topicWhitelistChanged(const std::set<std::string>& whitelist)
{
  ui_.whitelist_button->setText(QString::number(whitelist.size()) + tr(" topics"));

  whitelist_model_->updateListedTopics(whitelist, true);
  whitelist_model_->clean(true, true);
}

void TopicWidget::monitorUpdated(const std::map<std::string, eCAL::rec::TopicInfo> topic_info_map)
{
  all_topics_model_->updateVisibleTopics(topic_info_map, true);
  all_topics_model_->clean(true, false);

  blacklist_model_->updateVisibleTopics(topic_info_map, true);
  blacklist_model_->clean(true, true);

  whitelist_model_->updateVisibleTopics(topic_info_map, true);
  whitelist_model_->clean(true, true);
}


void TopicWidget::showBlacklistDialog()
{
  TopicListDialog topic_list_dialog(eCAL::rec::RecordMode::Blacklist, QEcalRec::instance()->topicBlacklist(), this);
  int result = topic_list_dialog.exec();
  if (result == TopicListDialog::DialogCode::Accepted)
  {
    auto blacklist = topic_list_dialog.getTopicList();
    QEcalRec::instance()->setTopicBlacklist(blacklist);
  }
}

void TopicWidget::showWhitelistDialog()
{
  TopicListDialog topic_list_dialog(eCAL::rec::RecordMode::Whitelist, QEcalRec::instance()->topicWhitelist(), this);
  int result = topic_list_dialog.exec();
  if (result == TopicListDialog::DialogCode::Accepted)
  {
    auto whitelist = topic_list_dialog.getTopicList();
    QEcalRec::instance()->setTopicWhitelist(whitelist);
  }
}

void TopicWidget::updateTopicCounterLabel()
{
  int all_topics = activeModel()->rowCount();
  int recorded_topics = 0;

  if (activeModel() == all_topics_model_)
  {
    recorded_topics = activeModel()->rowCount();
  }
  else if (activeModel() == blacklist_model_)
  {
    recorded_topics = all_topics - activeModel()->listedTopicsCount();
  }
  else if (activeModel() == whitelist_model_)
  {
    recorded_topics = activeModel()->listedTopicsCount();
  }

  ui_.status_label->setText(tr("Recording ") + QString::number(recorded_topics) + " / " + QString::number(all_topics));
}

void TopicWidget::treeviewContextMenuRequested(const QPoint& pos)
{
  QMenu context_menu;
  eCAL::rec::RecordMode mode = QEcalRec::instance()->recordMode();


  QModelIndexList selected_indexes = selectedSourceIndexes();
  const int selected_index_count = selected_indexes.size();

  if (mode == eCAL::rec::RecordMode::All)
  {
    QAction* dummy_action = new QAction(&context_menu);
    dummy_action->setText(tr("Switch mode to blacklist / whitelist topics"));
    dummy_action->setEnabled(false);
    context_menu.addAction(dummy_action);
  }
  else
  {
    QAction* add_to_list_action      = new QAction(&context_menu);
    QAction* remove_from_list_action = new QAction(&context_menu);

    const int selected_and_listed_index_count   = countListedIndexes(selected_indexes);
    const int selected_and_unlisted_index_count = selected_index_count - selected_and_listed_index_count;

    if (mode == eCAL::rec::RecordMode::Blacklist)
    {
      add_to_list_action     ->setText(tr("Add to blacklist"));
      remove_from_list_action->setText(tr("Remove from blacklist"));

      connect(add_to_list_action,      &QAction::triggered, this, &TopicWidget::addSelectedTopicsToBlacklist);
      connect(remove_from_list_action, &QAction::triggered, this, &TopicWidget::removeSelectedTopicsFromBlacklist);
    }
    else if (mode == eCAL::rec::RecordMode::Whitelist)
    {
      add_to_list_action     ->setText(tr("Add to whitelist"));
      remove_from_list_action->setText(tr("Remove from whitelist"));

      connect(add_to_list_action,      &QAction::triggered, this, &TopicWidget::addSelectedTopicsToWhitelist);
      connect(remove_from_list_action, &QAction::triggered, this, &TopicWidget::removeSelectedTopicsFromWhitelist);
    }

    if (QEcalRec::instance()->recordersRecording())
    {
      add_to_list_action->setEnabled(false);
      remove_from_list_action->setEnabled(false);
    }
    else
    {
      if (selected_and_unlisted_index_count <= 0)
        add_to_list_action->setEnabled(false);

      if (selected_and_listed_index_count <= 0)
        remove_from_list_action->setEnabled(false);
    }

    context_menu.addAction(add_to_list_action);
    context_menu.addAction(remove_from_list_action);
  }

  QAction* copy_action = new QAction(&context_menu);
  copy_action->setText(tr("Copy"));
  connect(copy_action, &QAction::triggered, this, &TopicWidget::copySelectedTopicNames);
  copy_action->setEnabled(selected_index_count > 0);

  context_menu.addSeparator();
  context_menu.addAction(copy_action);

  context_menu.exec(ui_.topic_treeview->viewport()->mapToGlobal(pos));
}

inline TopicListModel* TopicWidget::activeModel() const
{
  return static_cast<TopicListModel*>(topics_hide_disabled_proxy_model_->sourceModel());
}

QModelIndex TopicWidget::mapToSource(const QModelIndex& proxy_index) const
{
  return topics_hide_disabled_proxy_model_->mapToSource(topics_user_filter_proxy_model_->mapToSource(proxy_index));
}

QModelIndex TopicWidget::mapFromSource(const QModelIndex& source_index) const
{
  return topics_user_filter_proxy_model_->mapFromSource(topics_hide_disabled_proxy_model_->mapFromSource(source_index));
}

QModelIndexList TopicWidget::selectedSourceIndexes(int column) const
{
  QModelIndexList selected_source_indexes;

  auto selected_proxy_indexes = ui_.topic_treeview->selectionModel()->selectedRows(column);
  for (const auto& index : selected_proxy_indexes)
  {
    const QModelIndex source_index = mapToSource(index);
    if (source_index.isValid())
    {
      selected_source_indexes.push_back(std::move(source_index));
    }
  }

  return selected_source_indexes;
}

int TopicWidget::countListedIndexes(const QModelIndexList& index_list) const
{
  int counter = 0;

  for (const auto& index : index_list)
  {
    if (activeModel()->isListed(index.row()))
      counter++;
  }
  return counter;
}

void TopicWidget::addSelectedTopicsToBlacklist()
{
  std::set<std::string> blacklist = QEcalRec::instance()->topicBlacklist();
  QModelIndexList selected_indexes = selectedSourceIndexes((int)TopicListModel::Columns::NAME);
  for (const auto& index : selected_indexes)
  {
    blacklist.emplace(activeModel()->data(index, Qt::ItemDataRole::DisplayRole).toString().toStdString());
  }
  QEcalRec::instance()->setTopicBlacklist(blacklist);
}

void TopicWidget::addSelectedTopicsToWhitelist()
{
  std::set<std::string> whitelist = QEcalRec::instance()->topicWhitelist();
  QModelIndexList selected_indexes = selectedSourceIndexes((int)TopicListModel::Columns::NAME);
  for (const auto& index : selected_indexes)
  {
    whitelist.emplace(activeModel()->data(index, Qt::ItemDataRole::DisplayRole).toString().toStdString());
  }
  QEcalRec::instance()->setTopicWhitelist(whitelist);
}

void TopicWidget::removeSelectedTopicsFromBlacklist()
{
  std::set<std::string> blacklist = QEcalRec::instance()->topicBlacklist();
  QModelIndexList selected_indexes = selectedSourceIndexes((int)TopicListModel::Columns::NAME);
  for (const auto& index : selected_indexes)
  {
    blacklist.erase(activeModel()->data(index, Qt::ItemDataRole::DisplayRole).toString().toStdString());
  }
  QEcalRec::instance()->setTopicBlacklist(blacklist);
}

void TopicWidget::removeSelectedTopicsFromWhitelist()
{
  std::set<std::string> whitelist = QEcalRec::instance()->topicWhitelist();
  QModelIndexList selected_indexes = selectedSourceIndexes((int)TopicListModel::Columns::NAME);
  for (const auto& index : selected_indexes)
  {
    whitelist.erase(activeModel()->data(index, Qt::ItemDataRole::DisplayRole).toString().toStdString());
  }
  QEcalRec::instance()->setTopicWhitelist(whitelist);
}

void TopicWidget::copySelectedTopicNames()
{
  QModelIndexList selected_indexes = selectedSourceIndexes((int)TopicListModel::Columns::NAME);
  QString clipboard_string;

  auto index_it = selected_indexes.begin();
  while (index_it != selected_indexes.end())
  {
    if (index_it != selected_indexes.begin())
      clipboard_string += "\n";
    clipboard_string += activeModel()->data(*index_it, Qt::ItemDataRole::DisplayRole).toString();
    index_it++;
  }

  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(clipboard_string);
}
