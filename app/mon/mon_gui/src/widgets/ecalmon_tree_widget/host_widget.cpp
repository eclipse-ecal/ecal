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

#include "host_widget.h"

HostWidget::HostWidget(QWidget *parent)
  : EcalmonTreeWidget(parent)
{
  host_tree_model_ = new HostTreeModel(this);
  setModel(host_tree_model_);

  // Set default forced column
  setDefaultForcedColumn((int)HostTreeModel::Columns::HOST_NAME);

  // Set the filter columns
  QVector<int> filter_columns
  {
    (int)HostTreeModel::Columns::HOST_NAME,
  };
  setFilterColumns(filter_columns);

  // Set the default group-by-settings
  QList<int> group_by_enabled_columns
  {
    (int)HostTreeModel::Columns::HOST_NAME,
  };

  QList<EcalmonTreeWidget::GroupSetting> preconfigured_group_by_settings;

  EcalmonTreeWidget::GroupSetting nothing;
  nothing.group_by_columns = {};
  nothing.auto_expand = -1;
  nothing.name = "Nothing (Plain list)";
  preconfigured_group_by_settings.push_back(nothing);

  setGroupSettings(preconfigured_group_by_settings, group_by_enabled_columns);

  // Initial layout
  autoSizeColumns();

  // Set the default visible columns
  QVector<int> default_visible_columns
  {
    (int)HostTreeModel::Columns::HOST_NAME,
    (int)HostTreeModel::Columns::PUBLISHER_COUNT,
    (int)HostTreeModel::Columns::SUBSCRIBER_COUNT,
    (int)HostTreeModel::Columns::SENT_DATA,
    (int)HostTreeModel::Columns::RECEIVED_DATA,
  };
  setVisibleColumns(default_visible_columns);

  // Set the initial Tree Group
  ui_.group_by_combobox->setCurrentIndex(0);

  // Save the initial state for the resetLayout function
  saveInitialState();

  // Load the settings from the last start
  loadGuiSettings("host_widget");
}

HostWidget::~HostWidget()
{
  saveGuiSettings("host_widget");
}

void HostWidget::autoSizeColumns()
{
  eCAL::pb::Monitoring example_monitoring_pb;

  auto example_process_pb = example_monitoring_pb.add_processes();

  example_process_pb->set_registration_clock(999999);
  example_process_pb->set_host_name("CARPC00____");
  example_process_pb->set_process_id(999999);
  example_process_pb->set_process_name("");
  example_process_pb->set_unit_name("CameraSensorMapFusionCAF___");
  example_process_pb->set_process_parameter("");

  HostTreeItem* example_host_item = new HostTreeItem("CARPC00____");
  example_host_item->update(example_monitoring_pb);

  host_tree_model_->insertItem(example_host_item);

  QList<int> columns_to_resize
  {
    (int)HostTreeItem::Columns::HOST_NAME,
    (int)HostTreeItem::Columns::PUBLISHER_COUNT,
    (int)HostTreeItem::Columns::SUBSCRIBER_COUNT,
    (int)HostTreeItem::Columns::SENT_DATA,
    (int)HostTreeItem::Columns::RECEIVED_DATA,
  };

  for (int column : columns_to_resize)
  {
    ui_.tree_view->resizeColumnToContents(column);
  }

  host_tree_model_->removeItem(example_host_item);
}
