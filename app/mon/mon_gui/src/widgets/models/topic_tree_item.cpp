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

#include "topic_tree_item.h"

#include <QColor>
#include <QFont>

#include "item_data_roles.h"

#include "tree_item_type.h"

#include <ecal/ecal.h>

TopicTreeItem::TopicTreeItem()
  : QAbstractTreeItem()
{
}

TopicTreeItem::TopicTreeItem(const eCAL::pb::Topic& topic)
  : QAbstractTreeItem()
{
  update(topic);
}

TopicTreeItem::~TopicTreeItem()
{
}

QVariant TopicTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant TopicTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if (role == (Qt::ItemDataRole)ItemDataRoles::RawDataRole) //-V1016 //-V547
  {
    if (column == Columns::RCLOCK)
    {
      return topic_.rclock();
    }
    else if (column == Columns::HNAME)
    {
      return topic_.hname().c_str();
    }
    else if (column == Columns::PID)
    {
      return topic_.pid();
    }
    else if (column == Columns::PNAME)
    {
      return topic_.pname().c_str();
    }
    else if (column == Columns::UNAME)
    {
      return topic_.uname().c_str();
    }
    else if (column == Columns::TID)
    {
      return topic_.tid().c_str();
    }
    else if (column == Columns::TNAME)
    {
      return topic_.tname().c_str();
    }
    else if (column == Columns::DIRECTION)
    {
      return topic_.direction().c_str();
    }
    else if (column == Columns::TTYPE)
    {
      // When the monitor didn't tell us the topic type, we ask eCAL::Util instead
      std::string monitor_topic_type = topic_.ttype();
      if (!monitor_topic_type.empty())
      {
        return monitor_topic_type.c_str();
      }
      else
      {
        std::string monitor_topic_name = topic_.tname();
        if (!monitor_topic_name.empty())
        {
          return eCAL::Util::GetTypeName(monitor_topic_name).c_str();
        }
      }
      return "";
    }
    else if (column == Columns::TDESC)
    {
      return topic_.tdesc().c_str();
    }
    else if (column == Columns::TQOS)
    {
      auto reliability = topic_.tqos().reliability();

      QString qos_string;

      if (reliability == eCAL::pb::QOS::eQOSPolicy_Reliability::QOS_eQOSPolicy_Reliability_reliable_reliability_qos)
      {
        auto topic_tqos = topic_.tqos();
        qos_string += "Reliable";

        auto history = topic_tqos.history();
        if (history == eCAL::pb::QOS::eQOSPolicy_HistoryKind::QOS_eQOSPolicy_HistoryKind_keep_all_history_qos)
        {
          qos_string += " (History: unlimited)";
        }
        else if (history == eCAL::pb::QOS::eQOSPolicy_HistoryKind::QOS_eQOSPolicy_HistoryKind_keep_last_history_qos)
        {
          qos_string += (" (History: " + QString::number(topic_tqos.history_depth()) + ")");
        }
      }
      else
      {
        if (reliability == eCAL::pb::QOS::eQOSPolicy_Reliability::QOS_eQOSPolicy_Reliability_best_effort_reliability_qos)
        {
          qos_string += "Best effort";
        }
      }
      return qos_string;
    }
    else if (column == Columns::TLAYER)
    {
      QList<QVariant> layers;
      auto layer_pb = topic_.tlayer();
      for (auto layer = layer_pb.begin(); layer != layer_pb.end(); layer++)
      {
        layers.push_back((*layer).type());
      }
      return layers;
    }
    else if (column == Columns::TSIZE)
    {
      return topic_.tsize();
    }
    else if (column == Columns::CONNECTIONS_LOC)
    {
      return topic_.connections_loc();
    }
    else if (column == Columns::CONNECTIONS_EXT)
    {
      return topic_.connections_ext();
    }
    else if (column == Columns::MESSAGE_DROPS)
    {
      return topic_.message_drops();
    }
    else if (column == Columns::DCLOCK)
    {
      return (long long)topic_.dclock();
    }
    else if (column == Columns::DFREQ)
    {
      return topic_.dfreq();
    }
    else
    {
      return QVariant();
    }
  }

  else if ((role == Qt::ItemDataRole::DisplayRole) || (role == Qt::ItemDataRole::ToolTipRole))
  {
    if ((column == Columns::HNAME)
      || (column == Columns::PNAME)
      || (column == Columns::UNAME)
      || (column == Columns::TNAME)
      || (column == Columns::DIRECTION)
      || (column == Columns::TTYPE))
    {
      QString raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
      return (!raw_data.isEmpty() ? raw_data : "- ? -");
    }
    else if (column == Columns::TLAYER)
    {
      QString layer_string;

      auto layer_pb = topic_.tlayer();

      for (auto layer = layer_pb.begin(); layer != layer_pb.end(); layer++)
      {
        QString this_layer_string;
        switch ((*layer).type())
        {
        case eCAL::pb::eTLayerType::tl_ecal_tcp:
          this_layer_string = "tcp";
          break;
        case eCAL::pb::eTLayerType::tl_ecal_udp_mc:
          this_layer_string = "udp_mc";
          break;
        case eCAL::pb::eTLayerType::tl_ecal_shm:
          this_layer_string = "shm";
          break;
        case eCAL::pb::eTLayerType::tl_inproc:
          this_layer_string = "inproc";
          break;
        case eCAL::pb::eTLayerType::tl_all:
          this_layer_string = "all";
          break;
        default:
          this_layer_string = ("Unknown (" + QString::number((int)(*layer).type()) + ")");
        }

        if (!layer_string.isEmpty() && !this_layer_string.isEmpty())
        {
          layer_string += ", ";
        }

        layer_string += this_layer_string;
      }

      return layer_string;
    }

    else if (column == Columns::DFREQ)
    {
      long long raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toLongLong(); //-V1016
      return toFrequencyString(raw_data);
    }
    else
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
  }

  else if (role == (Qt::ItemDataRole)ItemDataRoles::SortRole) //-V1016 //-V547
  {
    return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
  }

  else if (role == (Qt::ItemDataRole)ItemDataRoles::FilterRole) //-V1016 //-V547
  {
    if ((column == Columns::HNAME)
      || (column == Columns::PNAME)
      || (column == Columns::UNAME)
      || (column == Columns::TNAME)
      || (column == Columns::DIRECTION)
      || (column == Columns::TTYPE))
    {
      auto raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
      return raw_data;
    }
    else
    {
      return data(column, Qt::ItemDataRole::DisplayRole);
    }
  }

  else if (role == Qt::ItemDataRole::TextAlignmentRole)
  {
    if ((column == Columns::TID)
      || (column == Columns::PID)
      || (column == Columns::RCLOCK)
      || (column == Columns::TSIZE)
      || (column == Columns::CONNECTIONS_LOC)
      || (column == Columns::CONNECTIONS_EXT)
      || (column == Columns::MESSAGE_DROPS)
      || (column == Columns::DCLOCK)
      || (column == Columns::DFREQ)
      )
    {
      return Qt::AlignmentFlag::AlignRight;
    }
    else
    {
      return Qt::AlignmentFlag::AlignLeft;
    }
  }

  else if (role == (Qt::ItemDataRole)ItemDataRoles::GroupRole) //-V1016 //-V547
  {
    if (column == Columns::PID)
    {
      QStringList list{ topic_.hname().c_str(), QString::number(topic_.pid()) };
      return list;
    }
    else if (column == Columns::PNAME)
    {
      QStringList list{topic_.hname().c_str(), topic_.pname().c_str()};
      return list;
    }
    else if (column == Columns::UNAME)
    {
      QStringList list{ topic_.hname().c_str(), topic_.uname().c_str(), QString::number(topic_.pid()) };
      return list;
    }
    else
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
  }

  else if (role == Qt::ItemDataRole::FontRole)
  {

    if ((column == Columns::HNAME)
      || (column == Columns::PNAME)
      || (column == Columns::UNAME)
      || (column == Columns::TNAME)
      || (column == Columns::DIRECTION)
      || (column == Columns::TTYPE))
    {
      QString raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
      if (raw_data.isEmpty())
      {
        QFont font;
        font.setItalic(true);
        return font;
      }
    }
    return QVariant::Invalid;
  }

  return QVariant::Invalid;
}

int TopicTreeItem::type() const
{
  return (int)TreeItemType::Topic;
}

void TopicTreeItem::update(const eCAL::pb::Topic& topic)
{
  topic_.Clear();
  topic_.CopyFrom(topic);
}

eCAL::pb::Topic TopicTreeItem::topicPb()
{
  eCAL::pb::Topic topic_pb;
  topic_pb.CopyFrom(topic_);
  return topic_pb;
}

QString TopicTreeItem::toFrequencyString(long long freq)
{
  if (freq == 0)
  {
    return "0";
  }
  else
  {
    return QString::number(((double)freq) / 1000.0, 'f', 2);
  }
}

std::string TopicTreeItem::topicId() const
{
  return topic_.tid();
}
