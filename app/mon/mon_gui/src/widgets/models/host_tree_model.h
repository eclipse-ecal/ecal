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

#include "group_tree_model.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "host_tree_item.h"
#include "group_tree_item.h"

#include <QMap>
#include <QVector>
#include <QPair>

class HostTreeModel : public GroupTreeModel
{
  Q_OBJECT

public:

  enum class Columns : int
  {
    GROUP,
    HOST_NAME,
    PUBLISHER_COUNT,
    SUBSCRIBER_COUNT,
    SENT_DATA,
    RECEIVED_DATA,

    COLUMN_COUNT
  };

  HostTreeModel(QObject *parent = nullptr);
  ~HostTreeModel();

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QVector<QPair<int, QString>> getTreeItemColumnNameMapping() const;

  void monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb) override;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;
  int groupColumn() const override;

private:
  std::map<Columns, QString> column_labels_ =
  {
    { Columns::GROUP,            "Group" },
    { Columns::HOST_NAME,        "Host" },
    { Columns::PUBLISHER_COUNT,  "Publisher" },
    { Columns::SUBSCRIBER_COUNT, "Subscriber" },
    { Columns::SENT_DATA,        "Pub. Traffic [KiB/s]" },
    { Columns::RECEIVED_DATA,    "Sub. Traffic [KiB/s]" },
    { Columns::PUBLISHER_COUNT,  "OS" },
  };

  std::map<Columns, int> item_column_mapping_ =
  {
    { Columns::GROUP,            -1 },
    { Columns::HOST_NAME,        (int)HostTreeItem::Columns::HOST_NAME },
    { Columns::PUBLISHER_COUNT,  (int)HostTreeItem::Columns::PUBLISHER_COUNT },
    { Columns::SUBSCRIBER_COUNT, (int)HostTreeItem::Columns::SUBSCRIBER_COUNT },
    { Columns::SENT_DATA,        (int)HostTreeItem::Columns::SENT_DATA },
    { Columns::RECEIVED_DATA,    (int)HostTreeItem::Columns::RECEIVED_DATA },
  };

  std::map<std::string, HostTreeItem*> tree_item_map_;
};
