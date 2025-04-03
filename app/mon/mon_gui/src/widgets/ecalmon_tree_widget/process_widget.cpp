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

#include "process_widget.h"

ProcessWidget::ProcessWidget(QWidget *parent)
  : EcalmonTreeWidget(parent)
{
  process_tree_model_ = new ProcessTreeModel(this);
  setModel(process_tree_model_);

  // Set default forced column
  setDefaultForcedColumn((int)ProcessTreeModel::Columns::UNIT_NAME);

  // Set the filter columns
  QVector<int> filter_columns
  {
    (int)ProcessTreeModel::Columns::UNIT_NAME,
    (int)ProcessTreeModel::Columns::HOST_NAME,
    (int)ProcessTreeModel::Columns::PROCESS_NAME,
    (int)ProcessTreeModel::Columns::PARAMETERS,
    (int)ProcessTreeModel::Columns::STATE,
    (int)ProcessTreeModel::Columns::INFO,
  };
  setFilterColumns(filter_columns);

  // Set the default group-by-settings
  QList<int> group_by_enabled_columns
  {
    (int)ProcessTreeModel::Columns::UNIT_NAME,
    (int)ProcessTreeModel::Columns::HOST_NAME,
    (int)ProcessTreeModel::Columns::PROCESS_NAME,
    //(int)ProcessTreeModel::Columns::STATE, //TODO: Enable dynamic re-grouping
    (int)ProcessTreeModel::Columns::TIMESYNC_STATE,
  };

  QList<EcalmonTreeWidget::GroupSetting> preconfigured_group_by_settings;

  EcalmonTreeWidget::GroupSetting nothing;
  nothing.group_by_columns = {};
  nothing.auto_expand = -1;
  nothing.name = "Nothing (Plain list)";
  preconfigured_group_by_settings.push_back(nothing);

  EcalmonTreeWidget::GroupSetting host;
  host.group_by_columns = { (int)ProcessTreeModel::Columns::HOST_NAME };
  host.auto_expand = 0;
  host.name = "Host";
  preconfigured_group_by_settings.push_back(host);

  EcalmonTreeWidget::GroupSetting shm_transport_domain;
  shm_transport_domain.group_by_columns = { (int)ProcessTreeModel::Columns::SHM_TRANSPORT_DOMAIN };
  shm_transport_domain.auto_expand = 0;
  shm_transport_domain.name = "SHM Transport Domain";
  preconfigured_group_by_settings.push_back(shm_transport_domain);

  setGroupSettings(preconfigured_group_by_settings, group_by_enabled_columns);

  // Initial layout
  autoSizeColumns();

  // Set the default visible columns
  QVector<int> default_visible_columns
  {
    (int)ProcessTreeModel::Columns::UNIT_NAME,
    (int)ProcessTreeModel::Columns::HOST_NAME,
    (int)ProcessTreeModel::Columns::PROCESS_ID,
    (int)ProcessTreeModel::Columns::PROCESS_NAME,
    (int)ProcessTreeModel::Columns::PARAMETERS,
    (int)ProcessTreeModel::Columns::STATE,
    (int)ProcessTreeModel::Columns::INFO,
    (int)ProcessTreeModel::Columns::HEARTBEAT,
    (int)ProcessTreeModel::Columns::ECAL_RUNTIME_VERSION,
  };
  setVisibleColumns(default_visible_columns);

  // Set the initial Tree Group
  ui_.group_by_combobox->setCurrentIndex(1);

  // Save the initial state for the resetLayout function
  saveInitialState();

  // Load the settings from the last start
  loadGuiSettings("process_widget");
}

ProcessWidget::~ProcessWidget()
{
  saveGuiSettings("process_widget");
}

void ProcessWidget::autoSizeColumns()
{
  eCAL::pb::Process example_process_pb;
  example_process_pb.set_registration_clock(999999);
  example_process_pb.set_host_name("HNAME00____");
  example_process_pb.set_process_id(999999);
  example_process_pb.set_process_name("");
  example_process_pb.set_unit_name("eCALProcessUnitNameABCDE___");
  example_process_pb.set_process_parameter("");

  example_process_pb.mutable_state()->set_severity(eCAL::pb::eProcessSeverity::proc_sev_warning);
  example_process_pb.mutable_state()->set_severity_level(eCAL::pb::eProcessSeverityLevel::proc_sev_level5);
  //example_process_pb.set_tsync_mode();

  ProcessTreeItem* example_process_item = new ProcessTreeItem(example_process_pb);
  GroupTreeItem* example_group_item = new GroupTreeItem("HNAME00____", "", "", QVariant(), "");

  process_tree_model_->insertItem(example_process_item);
  process_tree_model_->insertItem(example_group_item);

  QList<int> columns_to_resize
  {
    (int)ProcessTreeModel::Columns::GROUP,
    (int)ProcessTreeModel::Columns::UNIT_NAME,
    (int)ProcessTreeModel::Columns::HOST_NAME,
    (int)ProcessTreeModel::Columns::PROCESS_ID,
    (int)ProcessTreeModel::Columns::PROCESS_NAME,
    (int)ProcessTreeModel::Columns::STATE,
    (int)ProcessTreeModel::Columns::HEARTBEAT,
    (int)ProcessTreeModel::Columns::TIMESYNC_STATE,
    (int)ProcessTreeModel::Columns::TIMESYNC_MOD_NAME,
    (int)ProcessTreeModel::Columns::COMPONENT_INIT_INFO,
    (int)ProcessTreeModel::Columns::ECAL_RUNTIME_VERSION,
    (int)ProcessTreeModel::Columns::CONFIG_FILE_PATH,
  };

  for (int column : columns_to_resize)
  {
    ui_.tree_view->resizeColumnToContents(column);
  }

  process_tree_model_->removeItem(example_process_item);
  process_tree_model_->removeItem(example_group_item);
}