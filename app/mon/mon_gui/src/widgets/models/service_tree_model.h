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

#include "service_tree_item.h"

#include <QMap>
#include <QVector>
#include <QPair>

class ServiceTreeModel : public GroupTreeModel
{
  Q_OBJECT

public:

  enum class Columns : int
  {
    GROUP,
    UNIT_NAME,
    SERVICE_NAME,
    PROCESS_NAME,
    HOST_NAME,
    PID,
    TCP_PORT,
    METHOD_NAME,
    METHOD_REQUEST_TYPE,
    METHOD_RESPONSE_TYPE,
    HEARTBEAT,
    CALL_COUNT,

    COLUMN_COUNT
  };

  ServiceTreeModel(QObject *parent = nullptr);
  ~ServiceTreeModel();

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
    { Columns::HEARTBEAT,            "Heartbeat" },
    { Columns::HOST_NAME,            "Host" },
    { Columns::PID,                  "PID" },
    { Columns::PROCESS_NAME,         "Process Path" },
    { Columns::UNIT_NAME,            "Process" },
    { Columns::SERVICE_NAME,         "Service" },
    { Columns::TCP_PORT,             "TCP Port" },
    { Columns::METHOD_NAME,          "Method" },
    { Columns::METHOD_REQUEST_TYPE,  "Req. Type" },
    { Columns::METHOD_RESPONSE_TYPE, "Resp. Type" },
    { Columns::CALL_COUNT,           "Call count" },
  };

  std::map<Columns, int> tree_item_column_mapping =
  {
    { Columns::GROUP,                -1 },
    { Columns::HEARTBEAT,            (int)ServiceTreeItem::Columns::RCLOCK },
    { Columns::HOST_NAME,            (int)ServiceTreeItem::Columns::HNAME },
    { Columns::PID,                  (int)ServiceTreeItem::Columns::PID },
    { Columns::PROCESS_NAME,         (int)ServiceTreeItem::Columns::PNAME },
    { Columns::UNIT_NAME,            (int)ServiceTreeItem::Columns::UNAME },
    { Columns::SERVICE_NAME,         (int)ServiceTreeItem::Columns::SNAME },
    { Columns::TCP_PORT,             (int)ServiceTreeItem::Columns::TCP_PORT },
    { Columns::METHOD_NAME,          (int)ServiceTreeItem::Columns::MNAME },
    { Columns::METHOD_REQUEST_TYPE,  (int)ServiceTreeItem::Columns::REQ_TYPE },
    { Columns::METHOD_RESPONSE_TYPE, (int)ServiceTreeItem::Columns::RESP_TYPE },
    { Columns::CALL_COUNT,           (int)ServiceTreeItem::Columns::CALL_COUNT },
  };

  std::map<std::string, ServiceTreeItem*> tree_item_map_;
};
