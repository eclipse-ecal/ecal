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

#include "GroupTreeModel.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4146 4800)
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "TopicTreeItem.h"
#include "GroupTreeItem.h"

#include <QMap>
#include <QVector>
#include <QPair>

class TopicTreeModel : public GroupTreeModel
{
  Q_OBJECT

public:

  enum class Columns : int
  {
    GROUP,
    TOPIC_ID,
    TOPIC_NAME,
    DIRECTION,
    UNIT_NAME,
    HOST_NAME,
    PID,
    PROCESS_NAME,
    MESSAGE_TYPE,
    HEARTBEAT,
    //TOPIC_DESCRIPTION,
    QOS,
    TRANSPORT_LAYER,
    TOPIC_SIZE,
    CONNECTIONS_LOCAL,
    CONNECTIONS_EXTERNAL,
    MESSAGE_DROPS,
    DATA_CLOCK,
    DATA_FREQUENCY,
    DATA_FREQUENCY_MIN,
    DATA_FREQUENCY_MAX,
    DATA_FREQUENCY_MIN_ERR,
    DATA_FREQUENCY_MAX_ERR,

    COLUMN_COUNT
  };

  TopicTreeModel(QObject *parent = nullptr);
  ~TopicTreeModel();

  int columnCount(const QModelIndex &parent = QModelIndex()) const;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  QVector<QPair<int, QString>> getTreeItemColumnNameMapping() const;

  void monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb);

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;
  int groupColumn() const;

private:
  std::map<Columns, QString> columnLabels =
  {
    { Columns::GROUP,                  "Group" },
    { Columns::HEARTBEAT,              "Heartbeat" },
    { Columns::HOST_NAME,              "Host" },
    { Columns::PID,                    "PID" },
    { Columns::PROCESS_NAME,           "Process Path" },
    { Columns::UNIT_NAME,              "Process" },
    { Columns::TOPIC_ID,               "Topic ID" },
    { Columns::TOPIC_NAME,             "Topic" },
    { Columns::DIRECTION,              "Direction" },
    { Columns::MESSAGE_TYPE,           "Message Type" },
    //{ Columns::TOPIC_DESCRIPTION,      "Description" },
    { Columns::QOS,                    "QoS" },
    { Columns::TRANSPORT_LAYER,        "Layer" },
    { Columns::TOPIC_SIZE,             "Size [Byte]" },
    { Columns::CONNECTIONS_LOCAL,      "Loc. Connections" },
    { Columns::CONNECTIONS_EXTERNAL,   "Ext. Connections" },
    { Columns::MESSAGE_DROPS,          "Drops" },
    { Columns::DATA_CLOCK,             "Data Clock" },
    { Columns::DATA_FREQUENCY,         "Frequency [Hz]" },
    { Columns::DATA_FREQUENCY_MIN,     "Min Ref. Freq. [Hz]" },
    { Columns::DATA_FREQUENCY_MAX,     "Max Ref. Freq. [Hz]" },
    { Columns::DATA_FREQUENCY_MIN_ERR, "Min Ref. Freq. Error [Hz]" },
    { Columns::DATA_FREQUENCY_MAX_ERR, "Max Ref. Freq. Error [Hz]" },
  };

  std::map<Columns, int> topic_tree_item_column_mapping =
  {
    { Columns::GROUP,                  -1 },
    { Columns::HEARTBEAT,              (int)TopicTreeItem::Columns::RCLOCK },
    { Columns::HOST_NAME,              (int)TopicTreeItem::Columns::HNAME },
    { Columns::PID,                    (int)TopicTreeItem::Columns::PID },
    { Columns::PROCESS_NAME,           (int)TopicTreeItem::Columns::PNAME },
    { Columns::UNIT_NAME,              (int)TopicTreeItem::Columns::UNAME },
    { Columns::TOPIC_ID,               (int)TopicTreeItem::Columns::TID },
    { Columns::TOPIC_NAME,             (int)TopicTreeItem::Columns::TNAME },
    { Columns::DIRECTION,              (int)TopicTreeItem::Columns::DIRECTION },
    { Columns::MESSAGE_TYPE,           (int)TopicTreeItem::Columns::TTYPE },
    //{ Columns::TOPIC_DESCRIPTION,      (int)TopicTreeItem::Columns::TDESC },
    { Columns::QOS,                    (int)TopicTreeItem::Columns::TQOS },
    { Columns::TRANSPORT_LAYER,        (int)TopicTreeItem::Columns::TLAYER },
    { Columns::TOPIC_SIZE,             (int)TopicTreeItem::Columns::TSIZE },
    { Columns::CONNECTIONS_LOCAL,      (int)TopicTreeItem::Columns::CONNECTIONS_LOC },
    { Columns::CONNECTIONS_EXTERNAL,   (int)TopicTreeItem::Columns::CONNECTIONS_EXT },
    { Columns::MESSAGE_DROPS,          (int)TopicTreeItem::Columns::MESSAGE_DROPS },
    { Columns::DATA_CLOCK,             (int)TopicTreeItem::Columns::DCLOCK },
    { Columns::DATA_FREQUENCY,         (int)TopicTreeItem::Columns::DFREQ },
    { Columns::DATA_FREQUENCY_MIN,     (int)TopicTreeItem::Columns::DFREQ_MIN },
    { Columns::DATA_FREQUENCY_MAX,     (int)TopicTreeItem::Columns::DFREQ_MAX },
    { Columns::DATA_FREQUENCY_MIN_ERR, (int)TopicTreeItem::Columns::DFREQ_MIN_ERR },
    { Columns::DATA_FREQUENCY_MAX_ERR, (int)TopicTreeItem::Columns::DFREQ_MAX_ERR },
  };

  std::map<std::string, TopicTreeItem*> topic_tree_item_map_;
};
