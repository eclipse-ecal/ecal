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

#include "topic_list_model.h"

#include <models/item_data_roles.h>

#include <QFont>
#include <QColor>

#include <qecalrec.h>

#include <algorithm>

TopicListModel::TopicListModel(eCAL::rec::RecordMode mode, QObject *parent)
  : QAbstractItemModel(parent)
  , icon_warning_(":/ecalicons/WARNING")
  , mode_(mode)
{}

TopicListModel::~TopicListModel()
{}

QVariant TopicListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant::Invalid;

  const int     row    = index.row();
  const Columns column = (Columns)index.column();

  const auto& topic = *std::next(topics_.begin(), row);

  if (column == Columns::NAME)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return topic.name_;
    }
  }
  else if (column == Columns::TYPE)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      return topic.type_;
    }
  }
  else if (column == Columns::PUBLISHED_BY)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      QString publishers_string;
      auto publisher_it = topic.publishers_.begin();
      while (publisher_it != topic.publishers_.end())
      {
        if (publisher_it != topic.publishers_.begin())
        {
          publishers_string += QString(", ");
        }
        publishers_string += QString::fromStdString((*publisher_it).first);
        
        publishers_string += " (";
        for (auto unit_name_it = publisher_it->second.begin(); unit_name_it != publisher_it->second.end(); unit_name_it++)
        {
          if (unit_name_it != publisher_it->second.begin())
            publishers_string += QString(", ");
          publishers_string += QString::fromStdString(*unit_name_it);
        }
        publishers_string += ")";
        
        publisher_it++;
      }
      return publishers_string;
    }
  }
  else if (column == Columns::RECORDED_BY)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      QString recorders_string;
      auto recorder_it = topic.rec_subscribers_.begin();
      while (recorder_it != topic.rec_subscribers_.end())
      {
        if (recorder_it != topic.rec_subscribers_.begin())
        {
          recorders_string += QString(", ");
        }

        recorders_string += QString::fromStdString(recorder_it->first.first);

        if (recorder_it->second == 0.0)
          recorders_string += " [0 Hz]";
        else
          recorders_string += " [" + QString::number(recorder_it->second, 'f', 2) + " Hz]";

        recorder_it++;
      }
      return recorders_string;
    }
    else if (role == Qt::ItemDataRole::DecorationRole)
    {
      if (topic.rec_subscribers_.size() > 1)
      {
        return icon_warning_;
      }
    }
    else if (role == Qt::ItemDataRole::ToolTipRole)
    {
      if (topic.rec_subscribers_.size() > 1)
      {
        return "WARNING: Subscribed by multiple clients!";
      }
    }
    else if ((role == ItemDataRoles::FilterRole)
      || (role == ItemDataRoles::SortRole))
    {
      // The filter and sort roles omit the frequency
      QString recorders_string;
      auto recorder_it = topic.rec_subscribers_.begin();
      while (recorder_it != topic.rec_subscribers_.end())
      {
        if (recorder_it != topic.rec_subscribers_.begin())
        {
          recorders_string += QString(", ");
        }

        recorders_string += QString::fromStdString(recorder_it->first.first);

        recorder_it++;
      }
      return recorders_string;
    }
  }
  else if (column == Columns::RECORDING_ENABLED)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      bool recording_enabled =
        (((mode_ == eCAL::rec::RecordMode::Whitelist) && topic.listed_)
          || ((mode_ == eCAL::rec::RecordMode::Blacklist) && !topic.listed_)
          || (mode_ == eCAL::rec::RecordMode::All));
      return recording_enabled ? "Yes" : "No";
    }
  }

  if (role == Qt::ItemDataRole::FontRole)
  {
    if (mode_ == eCAL::rec::RecordMode::Blacklist)
    {
      if (topic.listed_)
      {
        QFont font;
        font.setStrikeOut(true);
        return font;
      }
    }
    else if (mode_ == eCAL::rec::RecordMode::Whitelist)
    {
      if (topic.listed_)
      {
        QFont font;
        font.setBold(true);
        return font;
      }
    }
  }
  else if (role == Qt::ItemDataRole::ForegroundRole)
  {
    if (mode_ == eCAL::rec::RecordMode::Blacklist)
    {
      if (topic.listed_)
      {
        return QColor(150, 150, 150);
      }
    }
    else if(mode_ == eCAL::rec::RecordMode::Whitelist)
    {
      if (topic.listed_)
      {
        return QColor(0, 128, 0);
      }
      else
      {
        return QColor(150, 150, 150);
      }
    }
  }
  else if ((role == ItemDataRoles::FilterRole)
    || (role == ItemDataRoles::SortRole)
    || (role == Qt::ItemDataRole::ToolTipRole))
  {
    // Fallback to the display role
    return data(index, Qt::ItemDataRole::DisplayRole);
  }
  return QVariant::Invalid;
}

QVariant TopicListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((role == Qt::ItemDataRole::DisplayRole)
    && (orientation == Qt::Orientation::Horizontal))
  {
    return column_labels_.at((Columns)section);
  }
  return QVariant::Invalid;
}

QModelIndex TopicListModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
  return createIndex(row, column, nullptr);
}

QModelIndex TopicListModel::parent(const QModelIndex& /*index*/) const
{
  return QModelIndex();
}

int TopicListModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
  {
    // the invisible root node has children
    return (int)topics_.size();
  }

  // The entries do not have children
  return 0;
}

int TopicListModel::columnCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
  {
    // the invisible root node has children
    return (int)Columns::COLUMN_COUNT;
  }

  // The entries do not have children
  return 0;
}

void TopicListModel::reset(const std::map<std::string, eCAL::rec_server::TopicInfo>& topic_info_map, const std::set<std::string>& listed_topics, bool add_additional_listed_topics)
{
  beginResetModel();

  topics_.clear();

  for (const auto& topic : topic_info_map)
  {
    Topic topic_info;
    topic_info.name_              = topic.first.c_str();
    topic_info.type_              = topic.second.type_.c_str();
    topic_info.publishers_        = topic.second.publishers_;
    topic_info.listed_            = (listed_topics.find(topic.first) != listed_topics.end());
    topic_info.in_topic_info_map_ = true;

    insertTopicWithoutUpdate(topic_info);
  }

  // Add listed topics that were not in the info map
  if (add_additional_listed_topics)
  {
    for (const std::string& listed_topic : listed_topics)
    {
      const QString listed_topic_qstr = listed_topic.c_str();
      if (std::find_if(topics_.begin(), topics_.end(), [&listed_topic_qstr](const Topic& topic) {return topic.name_ == listed_topic_qstr; }) == topics_.end())
      {
        Topic topic;
        topic.name_              = listed_topic_qstr;
        topic.type_              = "";
        topic.publishers_        = {};
        topic.listed_            = true;
        topic.in_topic_info_map_ = false;

        insertTopicWithoutUpdate(topic);
      }
    }
  }

  endResetModel();
}

void TopicListModel::insertTopic(const Topic& topic_info)
{
  auto insert_it = topics_.begin();
  int insert_row = 0;

  while (insert_it != topics_.end())
  {
    if (insert_it->name_.compare(topic_info.name_, Qt::CaseSensitivity::CaseInsensitive) > 0)
    {
      break;
    }
    ++insert_it;
    ++insert_row;
  }

  beginInsertRows(QModelIndex(), insert_row, insert_row);
  topics_.insert(insert_it, topic_info);
  endInsertRows();
}

void TopicListModel::insertTopicWithoutUpdate(const Topic& topic_info)
{
  auto insert_it = topics_.begin();

  while (insert_it != topics_.end())
  {
    if (insert_it->name_.compare(topic_info.name_, Qt::CaseSensitivity::CaseInsensitive) > 0)
    {
      break;
    }
    ++insert_it;
  }
  topics_.insert(insert_it, topic_info);
}

void TopicListModel::removeTopic(const QString& name)
{
  auto remove_it = topics_.begin();
  int remove_row = 0;

  while (remove_it != topics_.end())
  {
    if (remove_it->name_ == name)
    {
      beginRemoveRows(QModelIndex(), remove_row, remove_row);
      topics_.erase(remove_it);
      endRemoveRows();
      break;
    }
    ++remove_it;
    ++remove_row;
  }
}

bool TopicListModel::isListed(int row) const
{
  return std::next(topics_.begin(), row)->listed_;
}

bool TopicListModel::isListed(const QModelIndex& index) const
{
  if (!index.isValid())
    return false;
  return isListed(index.row());
}

int TopicListModel::listedTopicsCount() const
{
  int count = 0;
  for (const Topic& topic : topics_)
  {
    if (topic.listed_)
      count++;
  }
  return count;
}

void TopicListModel::updateVisibleTopics(const std::map<std::string, eCAL::rec_server::TopicInfo>& topic_info_map, bool add_new_entries)
{
  {
    // Update existing topics
    auto topic_it = topics_.begin();
    int  row = 0;

    while (topic_it != topics_.end())
    {
      const std::string topic_name = topic_it->name_.toStdString();

      auto topic_info_map_it = topic_info_map.find(topic_name);
      bool in_topic_info_map = (topic_info_map_it != topic_info_map.end());

      bool update = false;

      // Update type_
      if (in_topic_info_map
        &&(topic_it->type_ != QString::fromStdString(topic_info_map_it->second.type_)))
      {
        topic_it->type_ = QString::fromStdString(topic_info_map_it->second.type_);
        update = true;
      }

      // Update publishers_
      if (in_topic_info_map
        && (topic_it->publishers_ != topic_info_map_it->second.publishers_))
      {
        topic_it->publishers_ = topic_info_map_it->second.publishers_;
        update = true;
      }

      // Update rec_subscribers_
      if (in_topic_info_map
        && (topic_it->rec_subscribers_ != topic_info_map_it->second.rec_subscribers_))
      {
        topic_it->rec_subscribers_ = topic_info_map_it->second.rec_subscribers_;
        update = true;
      }

      // Update in_topic_info_map_
      if (topic_it->in_topic_info_map_ != in_topic_info_map)
      {
        topic_it->in_topic_info_map_ = in_topic_info_map;
        update = true;
      }

      if (update)
      {
        dataChanged(index(row, 0), index(row, columnCount() - 1));
      }

      ++topic_it;
      ++row;
    }
  }

  // Add new topics
  {
    if (add_new_entries)
    {
      for (const auto& topic_info : topic_info_map)
      {
        const QString topic_name_qstr = topic_info.first.c_str();
        const bool already_exists = (std::find_if(topics_.begin(), topics_.end(),
            [&topic_name_qstr](const Topic& topic)
            {
              return (topic.name_ == topic_name_qstr);
            })
          != topics_.end());

        if (!already_exists)
        {
          // Create a new topic entry
          Topic topic;
          topic.name_              = QString::fromStdString(topic_info.first.c_str());
          topic.type_              = QString::fromStdString(topic_info.second.type_);
          topic.publishers_        = topic_info.second.publishers_;
          topic.rec_subscribers_   = topic_info.second.rec_subscribers_;
          topic.listed_            = false;
          topic.in_topic_info_map_ = true;
          insertTopic(topic);
        }
      }
    }
  }
}

void TopicListModel::updateListedTopics(const std::set<std::string>& listed_topics, bool add_new_entries)
{
  //// Update existing topics
  //{
  //  auto topic_it = topics_.rbegin();
  //  int  row = rowCount() - 1;

  //  while (topic_it != topics_.rend())
  //  {
  //    const std::string topic_name = topic_it->name_.toStdString();
  //    bool is_listed = (listed_topics.find(topic_name) != listed_topics.end());

  //    if (topic_it->listed_ != is_listed)
  //    {
  //      topic_it->listed_ = is_listed;
  //      dataChanged(index(row, 0), index(row, columnCount() - 1));
  //    }

  //    topic_it++;
  //    row--; // We iterate backwards
  //  }
  //}

  // Update existing topics
  int row = 0;
  for (auto& topic : topics_)
  {
    const std::string topic_name = topic.name_.toStdString();
    bool is_listed = (listed_topics.find(topic_name) != listed_topics.end());

    if (topic.listed_ != is_listed)
    {
      topic.listed_ = is_listed;
      dataChanged(index(row, 0), index(row, columnCount() - 1));
    }

    row++;
  }

  // Add new topics
  if (add_new_entries)
  {
    for (const std::string& listed_topic : listed_topics)
    {
      const QString listed_topic_qstr = listed_topic.c_str();
      if (std::find_if(topics_.begin(), topics_.end(), [&listed_topic_qstr](const Topic& topic) {return topic.name_ == listed_topic_qstr; }) == topics_.end())
      {
        Topic topic;
        topic.name_              = listed_topic_qstr;
        topic.type_              = "";
        topic.publishers_        = {};
        topic.listed_            = true;
        topic.in_topic_info_map_ = false;

        insertTopic(topic);
      }
    }
  }
}

void TopicListModel::clean(bool keep_all_visible, bool keep_all_listed)
{
  auto topic_it = topics_.begin();
  int  row = 0;

  while (topic_it != topics_.end())
  {
    bool keep_topic =
      ((topic_it->in_topic_info_map_ && keep_all_visible)
        || (topic_it->listed_ && keep_all_listed)
        || (topic_it->in_topic_info_map_ && topic_it->listed_));


    if (!keep_topic)
    {
      beginRemoveRows(QModelIndex(), row, row);
      ++topic_it;
      topics_.erase(std::prev(topic_it));
      endRemoveRows();
    }
    else
    {
      ++topic_it;
      row++;
    }
  }
}
