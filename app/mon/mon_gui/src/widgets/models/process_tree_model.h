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

#include "process_tree_item.h"
#include "group_tree_item.h"

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
    DATAWRITE,
    DATAREAD,
    TIMESYNC_STATE,
    TIMESYNC_MOD_NAME,
    COMPONENT_INIT_INFO,
    ECAL_RUNTIME_VERSION,

    COLUMN_COUNT
  };

  ProcessTreeModel(QObject *parent = nullptr);
  ~ProcessTreeModel();

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QVector<QPair<int, QString>> getTreeItemColumnNameMapping() const;

  void monitorUpdated(const eCAL::pb::Monitoring& monitoring_pb) override;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;
  int groupColumn() const override;

private:
  std::map<Columns, QString> columnLabels =
  {
    { Columns::GROUP,                "Group" },
    { Columns::UNIT_NAME,            "Process" },
    { Columns::HOST_NAME,            "Host" },
    { Columns::PID,                  "PID" },
    { Columns::PROCESS_NAME,         "Process Path" },
    { Columns::PARAMETERS,           "Command line" },
    { Columns::STATE,                "State" },
    { Columns::INFO,                 "Info" },
    { Columns::HEARTBEAT,            "Heartbeat" },
    { Columns::MEMORY,               "RAM [KiB]" },
    { Columns::CPU_PERCENTAGE,       "CPU %" },
    { Columns::USER_TIME,            "User Time" },
    { Columns::DATAWRITE,            "Data Write [Byte/s]" },
    { Columns::DATAREAD,             "Data Read [Byte/s]" },
    { Columns::TIMESYNC_STATE,       "Timesync State" },
    { Columns::TIMESYNC_MOD_NAME,    "Timesync Mod Name" },
    { Columns::COMPONENT_INIT_INFO,  "Component Initialization" },
    { Columns::ECAL_RUNTIME_VERSION, "eCAL Runtime Version" },
  };

  std::map<Columns, int> column_mapping =
  {
    { Columns::GROUP,                -1 },
    { Columns::UNIT_NAME,            (int)ProcessTreeItem::Columns::UNAME },
    { Columns::HOST_NAME,            (int)ProcessTreeItem::Columns::HNAME },
    { Columns::PID,                  (int)ProcessTreeItem::Columns::PID },
    { Columns::PROCESS_NAME,         (int)ProcessTreeItem::Columns::PNAME },
    { Columns::PARAMETERS,           (int)ProcessTreeItem::Columns::PPARAM },
    { Columns::STATE,                (int)ProcessTreeItem::Columns::SEVERITY },
    { Columns::INFO,                 (int)ProcessTreeItem::Columns::INFO },
    { Columns::HEARTBEAT,            (int)ProcessTreeItem::Columns::RCLOCK },
    { Columns::CPU_PERCENTAGE,       (int)ProcessTreeItem::Columns::PCPU },
    { Columns::MEMORY,               (int)ProcessTreeItem::Columns::PMEMORY },
    { Columns::USER_TIME,            (int)ProcessTreeItem::Columns::USRPTIME },
    { Columns::DATAWRITE,            (int)ProcessTreeItem::Columns::DATAWRITE },
    { Columns::DATAREAD,             (int)ProcessTreeItem::Columns::DATAREAD },
    { Columns::TIMESYNC_STATE,       (int)ProcessTreeItem::Columns::TSYNC_STATE },
    { Columns::TIMESYNC_MOD_NAME,    (int)ProcessTreeItem::Columns::TSYNC_MOD_NAME },
    { Columns::COMPONENT_INIT_INFO,  (int)ProcessTreeItem::Columns::COMPONENT_INIT_INFO },
    { Columns::ECAL_RUNTIME_VERSION, (int)ProcessTreeItem::Columns::ECAL_RUNTIME_VERSION },
  };

  std::map<std::string, ProcessTreeItem*> tree_item_map_;
};
