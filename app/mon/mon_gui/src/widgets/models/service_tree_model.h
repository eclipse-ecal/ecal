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
    SERVICE_TYPE,
    SERVICE_NAME,
    PROCESS_NAME,
    HOST_NAME,
    PROCESS_ID,
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
    { Columns::PROCESS_ID,           "Process Id" },
    { Columns::PROCESS_NAME,         "Process Path" },
    { Columns::UNIT_NAME,            "Process" },
    { Columns::SERVICE_NAME,         "Service" },
    { Columns::SERVICE_TYPE,         "Type" },
    { Columns::TCP_PORT,             "TCP Port" },
    { Columns::METHOD_NAME,          "Method" },
    { Columns::METHOD_REQUEST_TYPE,  "Req. Type" },
    { Columns::METHOD_RESPONSE_TYPE, "Resp. Type" },
    { Columns::CALL_COUNT,           "Call count" },
  };

  std::map<Columns, int> tree_item_column_mapping =
  {
    { Columns::GROUP,                -1 },
    { Columns::HEARTBEAT,            (int)ServiceTreeItem<eCAL::pb::Service>::Columns::REGISTRATION_CLOCK },
    { Columns::HOST_NAME,            (int)ServiceTreeItem<eCAL::pb::Service>::Columns::HOST_NAME },
    { Columns::PROCESS_ID,           (int)ServiceTreeItem<eCAL::pb::Service>::Columns::PROCESS_ID },
    { Columns::PROCESS_NAME,         (int)ServiceTreeItem<eCAL::pb::Service>::Columns::PROCESS_NAME },
    { Columns::UNIT_NAME,            (int)ServiceTreeItem<eCAL::pb::Service>::Columns::UNIT_NAME },
    { Columns::SERVICE_NAME,         (int)ServiceTreeItem<eCAL::pb::Service>::Columns::SERVICE_NAME },
    { Columns::SERVICE_TYPE,         (int)ServiceTreeItem<eCAL::pb::Service>::Columns::STYPE },
    { Columns::TCP_PORT,             (int)ServiceTreeItem<eCAL::pb::Service>::Columns::TCP_PORT },
    { Columns::METHOD_NAME,          (int)ServiceTreeItem<eCAL::pb::Service>::Columns::METHOD_NAME },
    { Columns::METHOD_REQUEST_TYPE,  (int)ServiceTreeItem<eCAL::pb::Service>::Columns::REQ_TYPE },
    { Columns::METHOD_RESPONSE_TYPE, (int)ServiceTreeItem<eCAL::pb::Service>::Columns::RESP_TYPE },
    { Columns::CALL_COUNT,           (int)ServiceTreeItem<eCAL::pb::Service>::Columns::CALL_COUNT },
  };

  std::map<std::string, ServiceTreeItem<eCAL::pb::Client>*> tree_item_client_map_;
  std::map<std::string, ServiceTreeItem<eCAL::pb::Service>*> tree_item_server_map_;
};
