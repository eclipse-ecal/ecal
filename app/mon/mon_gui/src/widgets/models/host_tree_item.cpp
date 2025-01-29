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

#include "host_tree_item.h"

#include <QColor>
#include <QFont>

#include "item_data_roles.h"

#include "tree_item_type.h"

HostTreeItem::HostTreeItem(const QString& host_name)
  : QAbstractTreeItem()
  , host_name_(host_name)
  , publisher_count_(0)
  , subscriber_count_(0)
  , data_sent_bytes_(0)
  , data_received_bytes_(0)
{
}

HostTreeItem::~HostTreeItem()
{
}

QVariant HostTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant HostTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if (role == (Qt::ItemDataRole)ItemDataRoles::RawDataRole) //-V1016 //-V547
  {
    if (column == Columns::HOST_NAME)
    {
      return host_name_;
    }
    else if (column == Columns::PUBLISHER_COUNT)
    {
      return publisher_count_;
    }
    else if (column == Columns::SUBSCRIBER_COUNT)
    {
      return subscriber_count_;
    }
    else if (column == Columns::SENT_DATA)
    {
      return data_sent_bytes_;
    }
    else if (column == Columns::RECEIVED_DATA)
    {
      return data_received_bytes_;
    }
    else
    {
      return QVariant();
    }
  }

  else if ((role == Qt::ItemDataRole::DisplayRole) || (role == Qt::ItemDataRole::ToolTipRole))
  {
    if (column == Columns::HOST_NAME)
    {
      QString raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
      return (!raw_data.isEmpty() ? raw_data : "- ? -");
    }
    else if ((column == Columns::SENT_DATA)
         || (column  == Columns::RECEIVED_DATA))
    {
      long long raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toLongLong(); //-V1016
      return QString::number(raw_data / 1024.0, 'f', 2);
    }
    else
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
  }

  else if (role == ItemDataRoles::SortRole) //-V547
  {
    return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
  }

  else if (role == ItemDataRoles::FilterRole) //-V547
  {
    if (column == Columns::HOST_NAME)
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
    else
    {
      return data(column, Qt::ItemDataRole::DisplayRole);
    }
  }

  else if (role == Qt::ItemDataRole::TextAlignmentRole)
  {
    if ( (column == Columns::PUBLISHER_COUNT)
      || (column == Columns::SUBSCRIBER_COUNT)
      || (column == Columns::SENT_DATA)
      || (column == Columns::RECEIVED_DATA))
    {
      return Qt::AlignmentFlag::AlignRight;
    }
    return Qt::AlignmentFlag::AlignLeft;
  }

  else if (role == ItemDataRoles::GroupRole) //-V547
  {
    return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
  }

  else if (role == Qt::ItemDataRole::FontRole)
  {
    if (column == Columns::HOST_NAME)
    {
      QString raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
      if (raw_data.isEmpty())
      {
        QFont font;
        font.setItalic(true);
        return font;
      }
      else
      {
        return QVariant(); // Invalid QVariant
      }
    }
    else
    {
      return QVariant(); // Invalid QVariant
    }
  }

  return QVariant(); // Invalid QVariant
}

int HostTreeItem::type() const
{
  return (int)TreeItemType::Host;
}

void HostTreeItem::update(const eCAL::pb::Monitoring& monitoring_pb)
{
  // Clear old data
  publisher_count_     = 0;
  subscriber_count_    = 0;
  data_sent_bytes_     = 0;
  data_received_bytes_ = 0;

  // Fill variables with accumulated data
  for (int i = 0; i <monitoring_pb.topics_size(); ++i)
  {
    const auto& topic = monitoring_pb.topics(i);
    if (QString(topic.host_name().c_str()).compare(host_name_, Qt::CaseSensitivity::CaseInsensitive) == 0)
    {
      QString direction = topic.direction().c_str();
      if (direction.compare("publisher", Qt::CaseSensitivity::CaseInsensitive) == 0)
      {
        publisher_count_++;
        data_sent_bytes_ += ((long long)topic.topic_size() * (long long)topic.data_frequency()) / 1000;
      }
      else if (direction.compare("subscriber", Qt::CaseSensitivity::CaseInsensitive) == 0)
      {
        subscriber_count_++;
        data_received_bytes_ += ((long long)topic.topic_size() * (long long)topic.data_frequency()) / 1000;
      }
    }
  }
}