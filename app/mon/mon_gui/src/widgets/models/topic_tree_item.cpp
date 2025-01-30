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

#include "topic_tree_item.h"

#include <QFont>
#include <QString>
#include <QByteArray>

#include "item_data_roles.h"

#include "tree_item_type.h"

#include <ecal/ecal.h>

TopicTreeItem::TopicTreeItem(const eCAL::pb::Topic& topic)
{
  update(topic);
}

QVariant TopicTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant TopicTreeItem::data(Columns column, Qt::ItemDataRole role) const
{

  if (role == (Qt::ItemDataRole)ItemDataRoles::RawDataRole) //-V1016 //-V547
  {
    if (column == Columns::REGISTRATION_CLOCK)
    {
      return topic_.registration_clock();
    }
    else if (column == Columns::HOST_NAME)
    {
      return topic_.host_name().c_str();
    }
    else if (column == Columns::SHM_TRANSPORT_DOMAIN)
    {
      return topic_.shm_transport_domain().c_str();
    }
    else if (column == Columns::PROCESS_ID)
    {
      return topic_.process_id();
    }
    else if (column == Columns::PROCESS_NAME)
    {
      return topic_.process_name().c_str();
    }
    else if (column == Columns::UNIT_NAME)
    {
      return topic_.unit_name().c_str();
    }
    else if (column == Columns::TOPIC_ID)
    {
      return topic_.topic_id().c_str();
    }
    else if (column == Columns::TOPIC_NAME)
    {
      return topic_.topic_name().c_str();
    }
    else if (column == Columns::DIRECTION)
    {
      return topic_.direction().c_str();
    }
    else if (column == Columns::TENCODING)
    {
      return topic_.datatype_information().encoding().c_str();
    }
    else if (column == Columns::TTYPE)
    {
      return topic_.datatype_information().name().c_str();
    }
    else if (column == Columns::TDESC)
    {
      return topic_.datatype_information().descriptor_information().c_str();
    }
    else if (column == Columns::TRANSPORT_LAYER)
    {
      QList<QVariant> layers;
      auto layer_pb = topic_.transport_layer();
      for (const auto& layer : layer_pb)
      {
        layers.push_back(layer.type());
      }
      return layers;
    }
    else if (column == Columns::TOPIC_SIZE)
    {
      return topic_.topic_size();
    }
    else if (column == Columns::CONNECTIONS_LOCAL)
    {
      return topic_.connections_local();
    }
    else if (column == Columns::CONNECTIONS_EXTERNAL)
    {
      return topic_.connections_external();
    }
    else if (column == Columns::MESSAGE_DROPS)
    {
      return topic_.message_drops();
    }
    else if (column == Columns::DATA_CLOCK)
    {
      return (long long)topic_.data_clock();
    }
    else if (column == Columns::DFREQ)
    {
      return topic_.data_frequency();
    }
    else
    {
      return QVariant();
    }
  }

  else if ((role == Qt::ItemDataRole::DisplayRole) || (role == Qt::ItemDataRole::ToolTipRole))
  {
    if ((column == Columns::HOST_NAME)
      || (column == Columns::PROCESS_NAME)
      || (column == Columns::UNIT_NAME)
      || (column == Columns::TOPIC_NAME)
      || (column == Columns::DIRECTION)
      || (column == Columns::TENCODING)
      || (column == Columns::TTYPE))
    {
      const QString raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
      return (!raw_data.isEmpty() ? raw_data : "- ? -");
    }
    else if (column == Columns::TDESC)
    {
      const std::string& raw_data = topic_.datatype_information().descriptor_information();

      if (!raw_data.empty())
      {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const quint16 crc16 = qChecksum(raw_data.data(), static_cast<uint>(raw_data.length()));
#else
        const quint16 crc16 = qChecksum(raw_data);
#endif
      
        const QString crc16_string = QString("%1").arg(QString::number(crc16, 16).toUpper(), 4, '0');
        const QString size_text    = QString::number(raw_data.size()) + " byte" + (raw_data.size() != 1 ? "s" : "")
                                     + " (CRC16: " + crc16_string + ")";

        return size_text;
      }
      else
      {
        return "None";
      }
    }
    else if (column == Columns::TRANSPORT_LAYER)
    {
      QString layer_string;

      auto layer_pb = topic_.transport_layer();

      for (const auto& layer : layer_pb)
      {
        QString this_layer_string;
        if (layer.active())
        {
          switch (layer.type())
          {
          case eCAL::pb::eTransportLayerType::tl_ecal_tcp:
            this_layer_string = "tcp";
            break;
          case eCAL::pb::eTransportLayerType::tl_ecal_udp_mc:
            this_layer_string = "udp_mc";
            break;
          case eCAL::pb::eTransportLayerType::tl_ecal_shm:
            this_layer_string = "shm";
            break;
          case eCAL::pb::eTransportLayerType::tl_all:
            this_layer_string = "all";
            break;
          default:
            this_layer_string = ("Unknown (" + QString::number((int)layer.type()) + ")");
          }
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
      const long long raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toLongLong(); //-V1016
      return toFrequencyString(raw_data);
    }
    else
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
  }

  else if (role == (Qt::ItemDataRole)ItemDataRoles::SortRole) //-V1016 //-V547
  {
    if (column == Columns::TDESC)
    {
      const std::string& raw_data = topic_.datatype_information().descriptor_information();
      return static_cast<int>(raw_data.size());
    }

    return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
  }

  else if (role == (Qt::ItemDataRole)ItemDataRoles::FilterRole) //-V1016 //-V547
  {
    if ((column == Columns::HOST_NAME)
      || (column == Columns::SHM_TRANSPORT_DOMAIN)
      || (column == Columns::PROCESS_NAME)
      || (column == Columns::UNIT_NAME)
      || (column == Columns::TOPIC_NAME)
      || (column == Columns::DIRECTION)
      || (column == Columns::TENCODING)
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
    if ((column == Columns::TOPIC_ID)
      || (column == Columns::PROCESS_ID)
      || (column == Columns::REGISTRATION_CLOCK)
      || (column == Columns::TOPIC_SIZE)
      || (column == Columns::CONNECTIONS_LOCAL)
      || (column == Columns::CONNECTIONS_EXTERNAL)
      || (column == Columns::MESSAGE_DROPS)
      || (column == Columns::DATA_CLOCK)
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
    if (column == Columns::PROCESS_ID)
    {
      const QStringList list{ topic_.host_name().c_str(), QString::number(topic_.process_id()) };
      return list;
    }
    else if (column == Columns::PROCESS_NAME)
    {
      const QStringList list{topic_.host_name().c_str(), topic_.process_name().c_str()};
      return list;
    }
    else if (column == Columns::UNIT_NAME)
    {
      const QStringList list{ topic_.host_name().c_str(), topic_.unit_name().c_str(), QString::number(topic_.process_id()) };
      return list;
    }
    else
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
  }

  else if (role == Qt::ItemDataRole::FontRole)
  {

    if ((column == Columns::HOST_NAME)
      || (column == Columns::SHM_TRANSPORT_DOMAIN)
      || (column == Columns::PROCESS_NAME)
      || (column == Columns::UNIT_NAME)
      || (column == Columns::TOPIC_NAME)
      || (column == Columns::DIRECTION)
      || (column == Columns::TENCODING)
      || (column == Columns::TTYPE))
    {
      const QString raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
      if (raw_data.isEmpty())
      {
        QFont font;
        font.setItalic(true);
        font.setBold(itemfont.bold());
        return font;
      }
    }
    else if (column == Columns::TDESC)
    {
      const std::string& raw_data = topic_.datatype_information().descriptor_information();
      if (raw_data.empty())
      {
        QFont font;
        font.setItalic(true);
        font.setBold(itemfont.bold());
        return font;
      }
    }

    return itemfont;
  }

  return QVariant(); // Invalid QVariant
}

bool TopicTreeItem::setFont(const QFont& font)
{
  itemfont = font;
  return false;
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
  return topic_.topic_id();
}
