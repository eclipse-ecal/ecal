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

#include "service_widget.h"

ServiceWidget::ServiceWidget(QWidget *parent)
  : EcalmonTreeWidget(parent)
{
  service_tree_model_ = new ServiceTreeModel(this);
  setModel(service_tree_model_);

  // Set default forced column
  setDefaultForcedColumn((int)ServiceTreeModel::Columns::SERVICE_NAME);

  // Set the filter columns
  QVector<int> filter_columns
  {
    (int)ServiceTreeModel::Columns::HOST_NAME,
    (int)ServiceTreeModel::Columns::PROCESS_NAME,
    (int)ServiceTreeModel::Columns::UNIT_NAME,
    (int)ServiceTreeModel::Columns::SERVICE_NAME,
    (int)ServiceTreeModel::Columns::METHOD_NAME,
    (int)ServiceTreeModel::Columns::METHOD_REQUEST_TYPE,
    (int)ServiceTreeModel::Columns::METHOD_RESPONSE_TYPE,
  };
  setFilterColumns(filter_columns);

  // Set the default group-by-settings
  QList<int> group_by_enabled_columns
  {
    (int)ServiceTreeModel::Columns::HOST_NAME,
    (int)ServiceTreeModel::Columns::PROCESS_NAME,
    (int)ServiceTreeModel::Columns::UNIT_NAME,
    (int)ServiceTreeModel::Columns::PROCESS_ID,
    (int)ServiceTreeModel::Columns::SERVICE_NAME,
    (int)ServiceTreeModel::Columns::SERVICE_TYPE,
    (int)ServiceTreeModel::Columns::TCP_PORT,
    (int)ServiceTreeModel::Columns::METHOD_NAME,
    (int)ServiceTreeModel::Columns::METHOD_REQUEST_TYPE,
    (int)ServiceTreeModel::Columns::METHOD_RESPONSE_TYPE,
  };

  QList<EcalmonTreeWidget::GroupSetting> preconfigured_group_by_settings;

  EcalmonTreeWidget::GroupSetting nothing;
  nothing.group_by_columns = {};
  nothing.auto_expand = -1;
  nothing.name = "Nothing (Plain list)";
  preconfigured_group_by_settings.push_back(nothing);

  EcalmonTreeWidget::GroupSetting process;
  process.group_by_columns = { (int)ServiceTreeModel::Columns::HOST_NAME, (int)ServiceTreeModel::Columns::UNIT_NAME };
  process.auto_expand = 1;
  process.name = "Process";
  preconfigured_group_by_settings.push_back(process);

  setGroupSettings(preconfigured_group_by_settings, group_by_enabled_columns);

  // Initial layout
  autoSizeColumns();

  // Set the default visible columns
  QVector<int> default_visible_columns
  {
    (int)ServiceTreeModel::Columns::UNIT_NAME,
    (int)ServiceTreeModel::Columns::SERVICE_NAME,
    (int)ServiceTreeModel::Columns::SERVICE_TYPE,
    (int)ServiceTreeModel::Columns::HOST_NAME,
    (int)ServiceTreeModel::Columns::PROCESS_ID,
    (int)ServiceTreeModel::Columns::METHOD_NAME,
    (int)ServiceTreeModel::Columns::METHOD_REQUEST_TYPE,
    (int)ServiceTreeModel::Columns::METHOD_RESPONSE_TYPE,
    (int)ServiceTreeModel::Columns::HEARTBEAT,
    (int)ServiceTreeModel::Columns::CALL_COUNT,
  };
  setVisibleColumns(default_visible_columns);

  // Set the initial Tree Group
  ui_.group_by_combobox->setCurrentIndex(1);

  // Save the initial state for the resetLayout function
  saveInitialState();

  // Load the settings from the last start
  loadGuiSettings("service_widget");
}

ServiceWidget::~ServiceWidget()
{
  saveGuiSettings("service_widget");
}

void ServiceWidget::autoSizeColumns()
{
  eCAL::pb::Service example_service_pb;

  example_service_pb.set_registration_clock(999999);
  example_service_pb.set_host_name("CARPC00____");
  example_service_pb.set_process_name("");
  example_service_pb.set_unit_name("eCALRPCService____");
  example_service_pb.set_process_id(999999);
  example_service_pb.set_service_name("eCALRPCService____");
  example_service_pb.set_tcp_port_v1(999999);

  eCAL::pb::Method* method = example_service_pb.mutable_methods()->Add();
  method->set_method_name("ShutdownProcessName____");
  method->set_req_type("ShutdownProcessNameRequest____");
  method->set_resp_type("ShutdownProcessNameResponse____");
  method->set_call_count(999999);

  auto* example_topic_item = new ServiceTreeItem<eCAL::pb::Service>(example_service_pb, *method);
  GroupTreeItem* example_group_item = new GroupTreeItem("__ / eCALRPCService____", "", "", QVariant(), "");

  service_tree_model_->insertItem(example_topic_item);
  service_tree_model_->insertItem(example_group_item);

  QList<int> columns_to_resize
  {
    (int)ServiceTreeModel::Columns::GROUP,
    (int)ServiceTreeModel::Columns::UNIT_NAME,
    (int)ServiceTreeModel::Columns::SERVICE_NAME,
    (int)ServiceTreeModel::Columns::HOST_NAME,
    (int)ServiceTreeModel::Columns::PROCESS_ID,
    (int)ServiceTreeModel::Columns::TCP_PORT,
    (int)ServiceTreeModel::Columns::METHOD_NAME,
    (int)ServiceTreeModel::Columns::METHOD_REQUEST_TYPE,
    (int)ServiceTreeModel::Columns::METHOD_RESPONSE_TYPE,
    (int)ServiceTreeModel::Columns::HEARTBEAT,
  };

  for (int column : columns_to_resize)
  {
    ui_.tree_view->resizeColumnToContents(column);
  }

  service_tree_model_->removeItem(example_topic_item);
  service_tree_model_->removeItem(example_group_item);
}