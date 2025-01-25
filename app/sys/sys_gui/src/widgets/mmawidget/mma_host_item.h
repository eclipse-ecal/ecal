/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include <memory>
#include <mutex>
#include <chrono>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/subscriber.h>
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/monitoring.pb.h>
#include <ecal/app/pb/mma/mma.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QString>
#include <QTimer>

#include "resource_bar.h"
#include "multi_label_item.h"

class MmaHostItem : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  MmaHostItem(QTreeWidget *tree_widget, const QString& hostname);
  ~MmaHostItem();

  void addWidgetsToTree();

  void setSysClientAvailable(bool available);

public slots:
  void setEnabled(bool enabled);

  void machineStateChanged(const eCAL::pb::mma::State state);

private:
  QString hostname_;

  std::unique_ptr<eCAL::protobuf::CSubscriber<eCAL::pb::mma::State>> mma_subscriber;

  bool enabled_;

  QTreeWidgetItem* cpu_item_;
  QTreeWidgetItem* ram_item_;
  QTreeWidgetItem* disks_group_item_;
  QTreeWidgetItem* network_group_item_;

  static const int ADDITIONAL_MULTI_LABEL_SPACING = 8;

  std::map<QString, std::tuple<QTreeWidgetItem*, ResourceBar*, MultiLabelItem*>> disk_items_; /**< In oder to have access to the Resource bar of all disks without having to subclass QTreeWidgetItem again, we create a map here and keep it in sync with the disk_group_item's children*/
  std::map<QString, std::pair<QTreeWidgetItem*, MultiLabelItem*>> network_items;              /**< We also keep a map of all Tree items representing networks for the same reason as the disk_itmes */

  MultiLabelItem* root_widget_;
  ResourceBar*    cpu_bar_;
  ResourceBar*    ram_bar_;
  MultiLabelItem* network_group_widget_;
  MultiLabelItem* disk_group_widget_;

  QTimer* deactivation_timer;

  void mmaReceivedCallback(const eCAL::STopicId& topic_id_, const eCAL::pb::mma::State& state_);

  QString normalizedDataAsString(unsigned long long bytes);

  QPixmap checkmark_pixmap_;
  QPixmap cross_pixmap_;

private slots:
  void disable();

signals:
  void mmaReceivedSignal(eCAL::pb::mma::State state);
};
