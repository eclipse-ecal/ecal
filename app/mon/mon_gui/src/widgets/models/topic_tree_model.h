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

#pragma once

#include "group_tree_model.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/monitoring.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "topic_tree_item.h"
#include "group_tree_item.h"

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
    SHM_TRANSPORT_DOMAIN,
    PROCESS_ID,
    PROCESS_NAME,
    TOPIC_TYPE,
    HEARTBEAT,
    TOPIC_ENCODING,
    TOPIC_DESCRIPTOR,
    TRANSPORT_LAYER,
    TOPIC_SIZE,
    CONNECTIONS_LOCAL,
    CONNECTIONS_EXTERNAL,
    MESSAGE_DROPS,
    DATA_CLOCK,
    DATA_FREQUENCY,

    COLUMN_COUNT
  };

  TopicTreeModel(QObject *parent = nullptr);
  ~TopicTreeModel();

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QVector<QPair<int, QVariant>> getTreeItemColumnNameMapping() const;

  void monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb) override;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;
  int groupColumn() const override;

private:
  std::map<Columns, QString> columnLabels =
  {
    { Columns::GROUP,                  "Group" },
    { Columns::HEARTBEAT,              "Heartbeat" },
    { Columns::HOST_NAME,              "Host" },
    { Columns::SHM_TRANSPORT_DOMAIN,   "SHM Transport Domain" },
    { Columns::PROCESS_ID,             "Process Id" },
    { Columns::PROCESS_NAME,           "Process Path" },
    { Columns::UNIT_NAME,              "Process" },
    { Columns::TOPIC_ID,               "Topic ID" },
    { Columns::TOPIC_NAME,             "Topic" },
    { Columns::DIRECTION,              "Direction" },
    { Columns::TOPIC_ENCODING,         "Encoding" },
    { Columns::TOPIC_TYPE,             "Topic Type" },
    { Columns::TOPIC_DESCRIPTOR,       "Descriptor" },
    { Columns::TRANSPORT_LAYER,        "Layer" },
    { Columns::TOPIC_SIZE,             "Size [Byte]" },
    { Columns::CONNECTIONS_LOCAL,      "Loc. Connections" },
    { Columns::CONNECTIONS_EXTERNAL,   "Ext. Connections" },
    { Columns::MESSAGE_DROPS,          "Drops" },
    { Columns::DATA_CLOCK,             "Data Clock" },
    { Columns::DATA_FREQUENCY,         "Frequency [Hz]" },
  };

  std::map<Columns, int> topic_tree_item_column_mapping =
  {
    { Columns::GROUP,                  -1 },
    { Columns::HEARTBEAT,              (int)TopicTreeItem::Columns::REGISTRATION_CLOCK },
    { Columns::HOST_NAME,              (int)TopicTreeItem::Columns::HOST_NAME },
    { Columns::SHM_TRANSPORT_DOMAIN,   (int)TopicTreeItem::Columns::SHM_TRANSPORT_DOMAIN },
    { Columns::PROCESS_ID,             (int)TopicTreeItem::Columns::PROCESS_ID },
    { Columns::PROCESS_NAME,           (int)TopicTreeItem::Columns::PROCESS_NAME },
    { Columns::UNIT_NAME,              (int)TopicTreeItem::Columns::UNIT_NAME },
    { Columns::TOPIC_ID,               (int)TopicTreeItem::Columns::TOPIC_ID },
    { Columns::TOPIC_NAME,             (int)TopicTreeItem::Columns::TOPIC_NAME },
    { Columns::DIRECTION,              (int)TopicTreeItem::Columns::DIRECTION },
    { Columns::TOPIC_ENCODING,         (int)TopicTreeItem::Columns::TENCODING },
    { Columns::TOPIC_TYPE,             (int)TopicTreeItem::Columns::TTYPE },
    { Columns::TOPIC_DESCRIPTOR,       (int)TopicTreeItem::Columns::TDESC },
    { Columns::TRANSPORT_LAYER,        (int)TopicTreeItem::Columns::TRANSPORT_LAYER },
    { Columns::TOPIC_SIZE,             (int)TopicTreeItem::Columns::TOPIC_SIZE },
    { Columns::CONNECTIONS_LOCAL,      (int)TopicTreeItem::Columns::CONNECTIONS_LOCAL },
    { Columns::CONNECTIONS_EXTERNAL,   (int)TopicTreeItem::Columns::CONNECTIONS_EXTERNAL },
    { Columns::MESSAGE_DROPS,          (int)TopicTreeItem::Columns::MESSAGE_DROPS },
    { Columns::DATA_CLOCK,             (int)TopicTreeItem::Columns::DATA_CLOCK },
    { Columns::DATA_FREQUENCY,         (int)TopicTreeItem::Columns::DFREQ },
  };

  struct STopicTreeEntry
  {
    TopicTreeItem*  tree_item = nullptr;
    bool            default_font = false;
    bool            new_topic_timer = false;
    bool            striked_out = false;
    bool            deleted_topic_timer = false;
    bool            topic_removed = false;
  };
  std::map<std::string, STopicTreeEntry> topic_tree_item_map_;
};
