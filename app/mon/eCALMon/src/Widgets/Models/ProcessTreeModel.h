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

#include "ProcessTreeItem.h"
#include "GroupTreeItem.h"

#include <QMap>
#include <QVector>
#include <QPair>

class ProcessTreeModel : public GroupTreeModel
{
  Q_OBJECT

public:

  enum class Columns : int
  {
    GROUP,
    UNIT_NAME,
    HOST_NAME,
    PID,
    PROCESS_NAME,
    PARAMETERS,
    STATE,
    INFO,
    HEARTBEAT,
    MEMORY,
    CPU_PERCENTAGE,
    USER_TIME,
    UDP_SENT,
    UDP_RECEIVED,
    TIMESYNC_MODE,

    COLUMN_COUNT
  };

  ProcessTreeModel(QObject *parent = nullptr);
  ~ProcessTreeModel();

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
    { Columns::GROUP,         "Group" },
    { Columns::UNIT_NAME,     "Process" },
    { Columns::HOST_NAME,     "Host" },
    { Columns::PID,           "PID" },
    { Columns::PROCESS_NAME,  "Process Path" },
    { Columns::PARAMETERS,    "Command line" },
    { Columns::STATE,         "State" },
    { Columns::INFO,          "Info" },
    { Columns::HEARTBEAT,     "Heartbeat" },
    { Columns::MEMORY,        "RAM [KiB]" },
    { Columns::CPU_PERCENTAGE,"CPU %" },
    { Columns::USER_TIME,     "User Time" },
    { Columns::UDP_SENT,      "UDP Sent [Byte/s]" },
    { Columns::UDP_RECEIVED,  "UDP Rec [Byte/s]" },
    { Columns::TIMESYNC_MODE, "Timesync Mode" },
  };

  std::map<Columns, int> column_mapping =
  {
    { Columns::GROUP,          -1 },
    { Columns::UNIT_NAME,      (int)ProcessTreeItem::Columns::UNAME },
    { Columns::HOST_NAME,      (int)ProcessTreeItem::Columns::HNAME },
    { Columns::PID,            (int)ProcessTreeItem::Columns::PID },
    { Columns::PROCESS_NAME,   (int)ProcessTreeItem::Columns::PNAME },
    { Columns::PARAMETERS,     (int)ProcessTreeItem::Columns::PPARAM },
    { Columns::STATE,          (int)ProcessTreeItem::Columns::SEVERITY },
    { Columns::INFO,           (int)ProcessTreeItem::Columns::INFO },
    { Columns::HEARTBEAT,      (int)ProcessTreeItem::Columns::RCLOCK },
    { Columns::CPU_PERCENTAGE, (int)ProcessTreeItem::Columns::PCPU },
    { Columns::MEMORY,         (int)ProcessTreeItem::Columns::PMEMORY },
    { Columns::USER_TIME,      (int)ProcessTreeItem::Columns::USRPTIME },
    { Columns::UDP_SENT,       (int)ProcessTreeItem::Columns::UDPSBYTES },
    { Columns::UDP_RECEIVED,   (int)ProcessTreeItem::Columns::UDPRBYTES },
    { Columns::TIMESYNC_MODE,  (int)ProcessTreeItem::Columns::TSYNC_MODE },

  };

  std::map<std::string, ProcessTreeItem*> tree_item_map_;
};
