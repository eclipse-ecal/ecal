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

#pragma once

#include <QDialog>

#include "ui_topiclist_dialog.h"

#include "qecalrec.h"

#include <QStandardItemModel>

#include "topic_list_model.h"
#include <CustomQt/QMulticolumnSortFilterProxyModel.h>

class TopicListDialog : public QDialog
{
Q_OBJECT

public:
  TopicListDialog(eCAL::rec::RecordMode mode, std::set<std::string> initial_topic_list, QWidget *parent = 0);
  ~TopicListDialog();

  std::set<std::string> getTopicList();

private slots:
  void toggleVisibleTopicsWidgetHidden();
  void setVisibleTopicsHidden(bool hidden);

  void updateListedTopicsButtons();
  void updateVisibleTopicsButtons();

  void monitorUpdated(const std::map<std::string, eCAL::rec_server::TopicInfo>& topic_info_map);
  void updateListedTopicsInVisibleTopics();

  void addNewTopic();
  void editSelection();
  void removeSelectedTopics();
  void removeAllTopics();
  void openTopicInputDialog();

  void listedTopicsContextMenu(const QPoint& pos);
  void visibleTopicsContextMenu(const QPoint& pos);

  void addSelectedVisibleTopicsToList();


private:
  Ui::TopicListDialog ui_;

  eCAL::rec::RecordMode mode_;

  QStandardItemModel*          selected_topics_model_;
  QStableSortFilterProxyModel* selected_topics_proxy_model_;

  TopicListModel*                   visible_topics_model_;
  QMulticolumnSortFilterProxyModel* visible_topics_proxy_model_;

  QModelIndex addTopicToList(const QString& topic_name = "");
};
