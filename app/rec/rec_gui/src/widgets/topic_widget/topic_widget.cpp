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

#include <models/item_data_roles.h>

#include "topiclist_dialog.h"

#include <rec_client_core/ecal_rec_defs.h>

#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>
#include <QSettings>

TopicWidget::TopicWidget(QWidget *parent)
  : QWidget(parent)
  , connect_to_ecal_button_state_is_connect_(true)
  , first_show_event_(true)
{
  ui_.setupUi(this);

  connect(QEcalRec::instance(), &QEcalRec::recordModeChangedSignal,     this, &TopicWidget::recordModeChanged);
  connect(QEcalRec::instance(), &QEcalRec::topicBlacklistChangedSignal, this, &TopicWidget::topicBlacklistChanged);
  connect(QEcalRec::instance(), &QEcalRec::topicWhitelistChangedSignal, this, &TopicWidget::topicWhitelistChanged);
  connect(QEcalRec::instance(), &QEcalRec::monitorUpdatedSignal,        this, &TopicWidget::monitorUpdated);

  connect(QEcalRec::instance(), &QEcalRec::recordModeChangedSignal,     this, &TopicWidget::updateRecordModeEnabledStates);
  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal, this, &TopicWidget::updateRecordModeEnabledStates);

  connect(ui_.record_mode_all_topics_radiobutton, &QRadioButton::toggled, QEcalRec::instance(),
    [](bool checked) { if (checked) QEcalRec::instance()->setRecordMode(eCAL::rec::RecordMode::All); });
  connect(ui_.record_mode_whitelist_radiobutton, &QRadioButton::toggled, QEcalRec::instance(),
    [](bool checked) { if (checked) QEcalRec::instance()->setRecordMode(eCAL::rec::RecordMode::Whitelist); });
  connect(ui_.record_mode_blacklist_radiobutton, &QRadioButton::toggled, QEcalRec::instance(),
    [](bool checked) { if (checked) QEcalRec::instance()->setRecordMode(eCAL::rec::RecordMode::Blacklist); });

  connect(ui_.blacklist_button, &QAbstractButton::clicked, this, &TopicWidget::showBlacklistDialog);
  connect(ui_.whitelist_button, &QAbstractButton::clicked, this, &TopicWidget::showWhitelistDialog);

  // Topic list models
  auto topic_info_map = QEcalRec::instance()->topicInfo();
  auto blacklist      = QEcalRec::instance()->topicBlacklist();
  auto whitelist      = QEcalRec::instance()->topicWhitelist();

  all_topics_model_ = new TopicListModel(eCAL::rec::RecordMode::All,       this);
  blacklist_model_  = new TopicListModel(eCAL::rec::RecordMode::Blacklist, this);
  whitelist_model_  = new TopicListModel(eCAL::rec::RecordMode::Whitelist, this);

  all_topics_model_->reset(topic_info_map, {},        false);
  blacklist_model_ ->reset(topic_info_map, blacklist, true);
  whitelist_model_ ->reset(topic_info_map, whitelist, true);
  
  topics_hide_disabled_proxy_model_ = new QStableSortFilterProxyModel(this);
  topics_hide_disabled_proxy_model_->setFilterKeyColumn  ((int)TopicListModel::Columns::RECORDING_ENABLED);
  topics_hide_disabled_proxy_model_->setDynamicSortFilter(true);
  topics_hide_disabled_proxy_model_->setFilterRole(ItemDataRoles::FilterRole);

  topics_user_filter_proxy_model_ = new QMulticolumnSortFilterProxyModel(this);
  topics_user_filter_proxy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  topics_user_filter_proxy_model_->setFilterKeyColumn      ((int)TopicListModel::Columns::NAME);
  topics_user_filter_proxy_model_->setSortCaseSensitivity  (Qt::CaseSensitivity::CaseInsensitive);
  topics_user_filter_proxy_model_->setDynamicSortFilter    (true);
  topics_user_filter_proxy_model_->setFilterRole           (ItemDataRoles::FilterRole);
  topics_user_filter_proxy_model_->setSortRole             (ItemDataRoles::SortRole);
  topics_user_filter_proxy_model_->setSourceModel          (topics_hide_disabled_proxy_model_);
  
  if (QEcalRec::instance()->showDisabledElementsAtEnd())
    topics_user_filter_proxy_model_->setAlwaysSortedColumn((int)TopicListModel::Columns::RECORDING_ENABLED, Qt::SortOrder::DescendingOrder);

  ui_.topic_treeview->setModel(topics_user_filter_proxy_model_);

  row_height_delegate_ = new RowHeightDelegate(this);

  ui_.topic_treeview->setItemDelegate(row_height_delegate_);

  connect(QEcalRec::instance(), &QEcalRec::showDisabledElementsAtEndChanged, this
          , [this](bool show_at_end)
            {
              if (show_at_end)
                topics_user_filter_proxy_model_->setAlwaysSortedColumn((int)TopicListModel::Columns::RECORDING_ENABLED, Qt::SortOrder::DescendingOrder);
              else
                topics_user_filter_proxy_model_->setAlwaysSortedColumn(-1);
            });

  // Filter Combobox
  filter_columns_        = { (int)TopicListModel::Columns::NAME
                            , (int)TopicListModel::Columns::TYPE
                            , (int)TopicListModel::Columns::PUBLISHED_BY
                            , (int)TopicListModel::Columns::RECORDED_BY };

  filter_combobox_model_ = new QStandardItemModel(this);

  QStandardItem* all_columns_item = new QStandardItem("*");
  all_columns_item->setData(-1, Qt::ItemDataRole::UserRole);
  filter_combobox_model_->appendRow(all_columns_item);

  for (int column : filter_columns_)
  {
    QStandardItem* item = new QStandardItem(all_topics_model_->headerData(column, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString());
    item->setData(column, Qt::ItemDataRole::UserRole);
    filter_combobox_model_->appendRow(item);
  }

  ui_.filter_combobox->setModel(filter_combobox_model_);
  connect(ui_.filter_combobox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    [this]()
  {
    int filter_column = ui_.filter_combobox->currentData(Qt::UserRole).toInt();
    if (filter_column < 0)
      topics_user_filter_proxy_model_->setFilterKeyColumns(filter_columns_);
    else
      topics_user_filter_proxy_model_->setFilterKeyColumn(filter_column);
  });

  ui_.filter_combobox->setCurrentIndex(0);
  topics_user_filter_proxy_model_->setFilterKeyColumns(filter_columns_);

  // Tree view
  ui_.topic_treeview->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_.topic_treeview, &QAbstractItemView::customContextMenuRequested, this, &TopicWidget::treeviewContextMenuRequested);
  connect(ui_.topic_treeview, &QAdvancedTreeView::keySequenceCopyPressed,     this, &TopicWidget::copySelectedTopicNames);

  ui_.topic_treeview->sortByColumn((int)TopicListModel::Columns::NAME, Qt::SortOrder::AscendingOrder);

  // Alternating row colors
  ui_.topic_treeview->setAlternatingRowColors(QEcalRec::instance()->alternatingRowColorsEnabled());
  connect(QEcalRec::instance(), &QEcalRec::alternatingRowColorsEnabledChanged, ui_.topic_treeview, &QTreeView::setAlternatingRowColors);

  // Hide disabled checkbox
  connect(ui_.hide_disabled_checkbox, &QCheckBox::toggled, topics_hide_disabled_proxy_model_, [this](bool checked) {topics_hide_disabled_proxy_model_->setFilterFixedString(checked ? "Yes" : ""); });

  // Recorded topics label
  connect(QEcalRec::instance(), &QEcalRec::recordModeChangedSignal,     this, &TopicWidget::updateTopicCounterLabel);
  connect(QEcalRec::instance(), &QEcalRec::topicBlacklistChangedSignal, this, &TopicWidget::updateTopicCounterLabel);
  connect(QEcalRec::instance(), &QEcalRec::topicWhitelistChangedSignal, this, &TopicWidget::updateTopicCounterLabel);
  connect(QEcalRec::instance(), &QEcalRec::monitorUpdatedSignal,        this, &TopicWidget::updateTopicCounterLabel);

  // Filter lineedit
  connect(ui_.filter_lineedit, &QLineEdit::textChanged, this, [this](const QString& text) {topics_user_filter_proxy_model_->setFilterFixedString(text); });

  // Initial state
  recordModeChanged    (QEcalRec::instance()->recordMode());
  topicWhitelistChanged(QEcalRec::instance()->topicWhitelist());
  topicBlacklistChanged(QEcalRec::instance()->topicBlacklist());

  updateRecordModeEnabledStates();
}

TopicWidget::~TopicWidget()
{
  saveLayout();
}

///////////////////////////////////
// Save / Restore layout
///////////////////////////////////

void TopicWidget::showEvent(QShowEvent* /*event*/)
{
  if (first_show_event_)
  {
    // Auto-size the columns to some dummy data
    static std::map<TopicListModel::Columns, std::pair<bool, QString>> dummy_data
    {
      { TopicListModel::Columns::NAME,              { false, "3DSurroundViewImage" }},
      { TopicListModel::Columns::PUBLISHED_BY,      { false, "CARPC01 (3DSurroundViewImage)" }},
      { TopicListModel::Columns::RECORDING_ENABLED, { false, "Yes" }},
      { TopicListModel::Columns::PUBLISHED_BY,      { true,  "CARPC01 [9999]" }},
    };
    
    QFontMetrics tree_font_metrics   = ui_.topic_treeview->fontMetrics();
    QFontMetrics header_font_metrics = ui_.topic_treeview->header()->fontMetrics();

    int minimum_column_width = ui_.topic_treeview->header()->minimumSectionSize();
    
    for (int column = 0; column < ui_.topic_treeview->model()->columnCount(); column++)
    {
      auto dummy_it = dummy_data.find((TopicListModel::Columns)column);
      if (dummy_it != dummy_data.end())
      {
        // This calculation is far from exact, at some points is just relies on some magic numbers (for icon and frame sizes)
        int dummy_text_width = tree_font_metrics.boundingRect(dummy_it->second.second).width() + 4;
        int icon_width       = (dummy_it->second.first ? 25 : 0);
        int header_width     = header_font_metrics.boundingRect(ui_.topic_treeview->model()->headerData(column, Qt::Orientation::Horizontal).toString()).width() + 4;
        
        ui_.topic_treeview->header()->resizeSection(column, std::max(std::max((dummy_text_width + icon_width), header_width), minimum_column_width));
      }
    }

    // Fix the row height of the list
    int row_height = ui_.topic_treeview->style()->pixelMetric(QStyle::PixelMetric::PM_ListViewIconSize, nullptr, ui_.topic_treeview);
    row_height    +=  2 * ui_.topic_treeview->style()->pixelMetric(QStyle::PixelMetric::PM_DefaultFrameWidth, nullptr, ui_.topic_treeview);
    row_height_delegate_->setRowHeight(row_height);

    // Hide Columns that the user may not need that often
    ui_.topic_treeview->setColumnHidden((int)TopicListModel::Columns::TYPE,              true);
    ui_.topic_treeview->setColumnHidden((int)TopicListModel::Columns::RECORDING_ENABLED, true);

    saveInitialLayout();

    restoreLayout();
  }
  first_show_event_ = false;
}

void TopicWidget::saveLayout()
{
  QSettings settings;
  settings.setValue("topic_tree_state",     ui_.topic_treeview        ->saveState(eCAL::rec::Version()));
  settings.setValue("hide_disabled_topics", ui_.hide_disabled_checkbox->isChecked());
}

void TopicWidget::restoreLayout()
{
  QSettings settings;

  QVariant topic_tree_state_variant     = settings.value("topic_tree_state");
  QVariant hide_disabled_topics_variant = settings.value("hide_disabled_topics");

  if (topic_tree_state_variant.isValid())
  {
    QByteArray topic_tree_state = topic_tree_state_variant.toByteArray();
    ui_.topic_treeview->restoreState(topic_tree_state, eCAL::rec::Version());
  }

  if (hide_disabled_topics_variant.isValid())
  {
    bool hide_disabled_topics = hide_disabled_topics_variant.toBool();
    ui_.hide_disabled_checkbox->setChecked(hide_disabled_topics);
  }
}

void TopicWidget::saveInitialLayout()
{
  initial_topic_tree_state_     = ui_.topic_treeview->saveState();
  initial_hide_disabled_topics_ = ui_.hide_disabled_checkbox->isChecked();
}

void TopicWidget::resetLayout()
{
  ui_.topic_treeview        ->restoreState(initial_topic_tree_state_);
  ui_.hide_disabled_checkbox->setChecked  (initial_hide_disabled_topics_);
  ui_.filter_combobox       ->setCurrentIndex(0);
  ui_.filter_lineedit       ->clear();
}

///////////////////////////////////
// Private slots
///////////////////////////////////

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
  if (QEcalRec::instance()->recording())
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

void TopicWidget::monitorUpdated(const std::map<std::string, eCAL::rec_server::TopicInfo> topic_info_map)
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

  ui_.status_label->setText(tr("Selected ") + QString::number(recorded_topics) + " / " + QString::number(all_topics));
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

    if (QEcalRec::instance()->recording())
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
    ++index_it;
  }

  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(clipboard_string);
}
