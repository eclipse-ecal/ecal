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

#include <QAbstractItemModel>

#include <set>
#include <string>

#include <QIcon>

#include "qecalrec.h"

class TopicListModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum class Columns : int
  {
    NAME,
    TYPE,
    PUBLISHED_BY,
    RECORDED_BY,
    RECORDING_ENABLED,

    COLUMN_COUNT
  };

  struct Topic
  {
    QString name_;
    QString type_;
    std::map<std::string, std::set<std::string>>      publishers_;
    std::map<std::pair<std::string, int32_t>, double> rec_subscribers_;
    bool listed_;
    bool in_topic_info_map_;
  };

  explicit TopicListModel(eCAL::rec::RecordMode mode, QObject *parent = 0);
  ~TopicListModel();

  QVariant    data       (const QModelIndex &index, int role)                                   const override;
  QVariant    headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QModelIndex index      (int row, int column, const QModelIndex &parent = QModelIndex())       const override;
  QModelIndex parent     (const QModelIndex &index)                                             const override;
  int         rowCount   (const QModelIndex &parent = QModelIndex())                            const override;
  int         columnCount(const QModelIndex &parent = QModelIndex())                            const override;

  void reset(const std::map<std::string, eCAL::rec_server::TopicInfo>& topic_info_map, const std::set<std::string>& listed_topics, bool add_additional_listed_topics);
  void insertTopic(const Topic& topic_info);
  void removeTopic(const QString& name);

  bool isListed(int row) const;
  bool isListed(const QModelIndex& index) const;

  int listedTopicsCount() const;

public slots:
  void updateVisibleTopics(const std::map<std::string, eCAL::rec_server::TopicInfo>& topic_info_map, bool add_new_entries);
  void updateListedTopics (const std::set<std::string>& listed_topics, bool add_new_entries);

  void clean(bool keep_all_visible, bool keep_all_listed);

private:
  const std::map<Columns, QString> column_labels_
  {
    { Columns::NAME,              "Topics" } ,
    { Columns::TYPE,              "Type" } ,
    { Columns::PUBLISHED_BY,      "Published by" } ,
    { Columns::RECORDED_BY,       "Recorded by" } ,
    { Columns::RECORDING_ENABLED, "Allowed by filter" } ,
  };
  const QIcon icon_warning_;

  void insertTopicWithoutUpdate(const Topic& topic_info);

  const eCAL::rec::RecordMode mode_;
  std::list<Topic> topics_;

};