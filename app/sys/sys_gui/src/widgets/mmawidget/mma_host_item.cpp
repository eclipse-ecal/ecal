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

#include "mma_host_item.h"
#include "tree_widget_item_types.h"

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/subscriber.h>
#include <ecal/core/pb/monitoring.pb.h>
#include <ecal/app/pb/mma/mma.pb.h>

#include <QApplication>
#include <QPalette>

#include <iomanip>
#include <sstream>
#include <cmath>

MmaHostItem::MmaHostItem(QTreeWidget* tree_widget, const QString& hostname)
  : QObject(tree_widget)
  , QTreeWidgetItem(tree_widget, TreeWidgetItemType::MmaHostItem)
  , hostname_(hostname)
  , enabled_(true)
{
  // Format the root node 
  root_widget_ = new MultiLabelItem(2, treeWidget());
  connect(this, &QObject::destroyed, root_widget_, &QObject::deleteLater);
  root_widget_->setText(1, hostname_);
  treeWidget()->setItemWidget(this, 0, root_widget_);

  checkmark_pixmap_ = QPixmap(":ecalicons/CHECKMARK").scaled(root_widget_->height(), root_widget_->height(), Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
  cross_pixmap_     = QPixmap(":ecalicons/CROSS")    .scaled(root_widget_->height(), root_widget_->height(), Qt::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
  
  setSysClientAvailable(false);

  // Initial Format
  QFont font;
  font.setBold(true);
  root_widget_->setFont(1, font);
  
  // Fill this node with content
  cpu_item_           = new QTreeWidgetItem(this);
  ram_item_           = new QTreeWidgetItem(this);
  disks_group_item_   = new QTreeWidgetItem(this);
  network_group_item_ = new QTreeWidgetItem(this);

  this->addChild(cpu_item_);
  this->addChild(ram_item_);
  this->addChild(disks_group_item_);
  this->addChild(network_group_item_);

  // Add the progress-bars etc. to the QTreeWidget
  addWidgetsToTree();

  setEnabled(false);

  // Create eCAL Subscriber
  mma_subscriber = std::unique_ptr<eCAL::protobuf::CSubscriber<eCAL::pb::mma::State>>
                    (new eCAL::protobuf::CSubscriber<eCAL::pb::mma::State>("machine_state_" + hostname_.toStdString()));
  mma_subscriber->SetReceiveCallback(std::bind(&MmaHostItem::mmaReceivedCallback, this, std::placeholders::_1, std::placeholders::_2));

  // Register custom Type in order to directly pass the monitoring state
  qRegisterMetaType<eCAL::pb::mma::State>("eCAL::pb::mma::State");
  connect(this, &MmaHostItem::mmaReceivedSignal, this, &MmaHostItem::machineStateChanged);

  // Create the timer that deactivates this object if no data is received
  deactivation_timer = new QTimer(this);
  deactivation_timer->setSingleShot(true);
  connect(deactivation_timer, &QTimer::timeout, this, &MmaHostItem::disable);
}

MmaHostItem::~MmaHostItem()
{}

void MmaHostItem::addWidgetsToTree()
{
  cpu_bar_ = new ResourceBar(treeWidget());
  ram_bar_ = new ResourceBar(treeWidget());
  network_group_widget_ = new MultiLabelItem(3, treeWidget());
  disk_group_widget_    = new MultiLabelItem(3, treeWidget());

  cpu_bar_->label->setText("CPU:");
  ram_bar_->label->setText("RAM:");

  network_group_widget_->setText(0, "Network:");
  network_group_widget_->setText(1, "");
  network_group_widget_->setText(2, "");
  disk_group_widget_->setText(0, "Disks:");
  disk_group_widget_->setText(1, "");
  disk_group_widget_->setText(2, "");

  // "measure" how wide the labels need to be
  QLabel test_label(treeWidget());
  test_label.setText("Network:");
  int name_width = test_label.sizeHint().width() + ADDITIONAL_MULTI_LABEL_SPACING;
  test_label.setText("W: 99.9 MiB/s");
  int io_width = test_label.sizeHint().width() + ADDITIONAL_MULTI_LABEL_SPACING;

  // Set the width
  network_group_widget_->setMinimumWidth(0, name_width);
  network_group_widget_->setMinimumWidth(1, io_width);
  network_group_widget_->setMinimumWidth(2, io_width);
  disk_group_widget_   ->setMinimumWidth(0, name_width);
  disk_group_widget_   ->setMinimumWidth(1, io_width);
  disk_group_widget_   ->setMinimumWidth(2, io_width);

  // The widgets might both be deleted when removing this object or when destroying the treeWidget, so we connect the appropriate signals
  connect(this, &QObject::destroyed, cpu_bar_,              &QObject::deleteLater);
  connect(this, &QObject::destroyed, ram_bar_,              &QObject::deleteLater);
  connect(this, &QObject::destroyed, network_group_widget_, &QObject::deleteLater);
  connect(this, &QObject::destroyed, disk_group_widget_,    &QObject::deleteLater);

  treeWidget()->setItemWidget(cpu_item_, 0, cpu_bar_);
  treeWidget()->setItemWidget(ram_item_, 0, ram_bar_);
  treeWidget()->setItemWidget(network_group_item_, 0, network_group_widget_);
  treeWidget()->setItemWidget(disks_group_item_, 0, disk_group_widget_);
}

void MmaHostItem::setSysClientAvailable(bool available)
{
  if (available)
  {
    root_widget_->setPixmap(0, checkmark_pixmap_);
    this->setToolTip(0, "Sys Client is available");
  }
  else
  {
    root_widget_->setPixmap(0, cross_pixmap_);
    this->setToolTip(0, "Sys Client is not available");
  }
}

void MmaHostItem::setEnabled(bool enabled)
{
  if (enabled_ == enabled)
  {
    return;
  }

  enabled_ = enabled;

  // clean children of the network and disk item
  for (auto it = network_items.begin(); it != network_items.end();)
  {
    treeWidget()->removeItemWidget(it->second.first->child(0), 0);
    network_group_item_->removeChild(it->second.first);
    delete it->second.first;
    it->second.second->deleteLater();
    it = network_items.erase(it);
  }
  for (auto it = disk_items_.begin(); it != disk_items_.end();)
  {
    treeWidget()->removeItemWidget(std::get<0>(it->second), 0);
    treeWidget()->removeItemWidget(std::get<0>(it->second)->child(0), 0);
    disks_group_item_->removeChild(std::get<0>(it->second));
    delete std::get<0>(it->second);
    std::get<1>(it->second)->deleteLater();
    std::get<2>(it->second)->deleteLater();
    it = disk_items_.erase(it);
  }

  // Set Colors and Fonts
  for (int col : {0})
  {
    // this item
    if (!enabled)
    {
      root_widget_->setStyleSheet(1, "QLabel { color: grey; }");
    }
    else
    {
      root_widget_->setStyleSheet(1, "");
    }

    // children
    for (int i = 0; i < childCount(); i++)
    {
      QTreeWidgetItem* child_item = child(i);
      if (!enabled)
      {
        child_item->setForeground(col, QColor(128, 128, 128));
      }
      else
      {
        child_item->setForeground(col, QApplication::palette().windowText().color());
      }
    }
  }

  cpu_bar_->setEnabled(enabled);
  cpu_bar_->bar->setValue(0);
  cpu_bar_->bar->setFormat(enabled? "%p%" : "");

  ram_bar_->setEnabled(enabled);
  ram_bar_->bar->setValue(0);
  ram_bar_->bar->setFormat(enabled ? "%p%" : "");

  network_group_widget_->setEnabled(enabled);
  disk_group_widget_   ->setEnabled(enabled);
  if (!enabled)
  {
    network_group_widget_->setText(1, "");
    network_group_widget_->setText(2, "");
    disk_group_widget_   ->setText(1, "");
    disk_group_widget_   ->setText(2, "");
  }
}

void MmaHostItem::disable()
{
  setEnabled(false);
}

void MmaHostItem::mmaReceivedCallback(const eCAL::STopicId& /*topic_id_*/, const eCAL::pb::mma::State& state_)
{
  // only emit the signal in order to make use of the Qt event loop
  emit mmaReceivedSignal(state_);
}

void MmaHostItem::machineStateChanged(eCAL::pb::mma::State state)
{
  if (!enabled_)
  {
    setEnabled(true);
  }
  // (re-)start the timer that deactivates this item after 3 seconds of inactivity
  deactivation_timer->start(3000);

  // Update CPU % bar
  double cpu_load = state.cpu_load();
  cpu_bar_->setRelativeValue(cpu_load / 100.0);

  // Update Ram % bar
  unsigned long long total_mem = state.memory().total();
  unsigned long long available_mem = state.memory().available();
  unsigned long long mem_in_use = total_mem - available_mem;
  double rel_mem_in_use = (double)(mem_in_use) / (double)(total_mem);
  ram_bar_->setRelativeValue(rel_mem_in_use);
  ram_bar_->bar->setFormat(normalizedDataAsString(mem_in_use) + " / " + normalizedDataAsString(total_mem));

  // Update Disks
  bool disk_added = false;
  unsigned long long cummulative_bytes_write = 0;
  unsigned long long cummulative_bytes_read  = 0;
  for (auto& disk : state.disks())
  {
    QString name(disk.name().c_str());
    unsigned long long total_capacity  = disk.capacity();
    unsigned long long available_space = disk.available();
    
    // The MMA Application sends an unsigned(!) -1, as default value, which is somewhat around 16384 Petabyte as uint64.
    // This is a workaround to simply set everything to 0, when that happens.
    total_capacity  = (total_capacity  == (decltype(disk.capacity()))(-1))  ? 0 : total_capacity;
    available_space = (available_space == (decltype(disk.available()))(-1)) ? 0 : available_space;

    unsigned long long space_in_use = total_capacity - available_space;

    double rel_space_in_use = 0.0;
    if (total_capacity > 0)
    {
      rel_space_in_use = (double)(space_in_use) / (double)(total_capacity);
    }

    ResourceBar*    disk_bar;
    MultiLabelItem* disk_io_widget;
    // Check if we already have a disk with that name
    if (disk_items_.find(name) != disk_items_.end())
    {
      // The disk does exist
      disk_bar       = std::get<1>(disk_items_[name]);
      disk_io_widget = std::get<2>(disk_items_[name]);
    }
    else
    {
      // We need to create a new item for a new disk
      disk_bar = new ResourceBar(treeWidget());
      QTreeWidgetItem* disk_item    = new QTreeWidgetItem(disks_group_item_);
      QTreeWidgetItem* disk_io_item = new QTreeWidgetItem(disk_item);
      disk_io_widget = new MultiLabelItem(2, treeWidget());

      // "measure" how wide the labels need to be
      QLabel test_label(treeWidget());
      test_label.setText("Write: 99.9 MiB/s");
      int io_width = test_label.sizeHint().width() + ADDITIONAL_MULTI_LABEL_SPACING;
      disk_io_widget->setMinimumWidth(0, io_width);
      disk_io_widget->setMinimumWidth(1, io_width);

      treeWidget()->setItemWidget(disk_io_item, 0, disk_io_widget);

      disk_bar->label->setText(name);
      
      // The widgets might be deleted when removing this object or when destroying the treeWidget, so we connect the appropriate signal
      connect(this, &QObject::destroyed, disk_bar,       &QObject::deleteLater);
      connect(this, &QObject::destroyed, disk_io_widget, &QObject::deleteLater);

      treeWidget()->setItemWidget(disk_item, 0, disk_bar);
      disk_items_.emplace(name, std::make_tuple(disk_item, disk_bar, disk_io_widget));
      disk_added = true;
    }

    // Update the disk bar
    disk_bar->setRelativeValue(rel_space_in_use);
    disk_bar->bar->setFormat(normalizedDataAsString(available_space) + " free (of " + normalizedDataAsString(total_capacity) + ")");

    // Update disk I/O
    double bytes_write = disk.write();
    double bytes_read = disk.read();
    bytes_write = std::max(bytes_write, 0.0);
    bytes_read  = std::max(bytes_read, 0.0);
    QString bytes_write_string = "Write: " + normalizedDataAsString((unsigned long long)bytes_write) + "/s";
    QString bytes_read_string  = "Read: "  + normalizedDataAsString((unsigned long long)bytes_read) + "/s";
    //disk_io_item->setText(0, "Read: " + bytes_read_string + "    Write: " + bytes_write_string);
    disk_io_widget->setText(0, bytes_read_string);
    disk_io_widget->setText(1, bytes_write_string);

    cummulative_bytes_write += bytes_write;
    cummulative_bytes_read  += bytes_read;
  }

  QString cummulative_read_string  = "R: " + normalizedDataAsString((unsigned long long)cummulative_bytes_read)  + "/s";
  QString cummulative_write_string = "W: " + normalizedDataAsString((unsigned long long)cummulative_bytes_write) + "/s";
  disk_group_widget_->setText(1, cummulative_read_string);
  disk_group_widget_->setText(2, cummulative_write_string);

  // Re-sort children if any disk has been added
  if (disk_added)
  {
    disks_group_item_->sortChildren(0, Qt::SortOrder::AscendingOrder);
  }

  // Remove old disks that are not present any more, e.g. someone may have unplugged an USB stick
  for (auto it = disk_items_.begin(); it != disk_items_.end();)
  {
    bool still_available = false;
    for (auto& disk : state.disks())
    {
      if (disk.name() == it->first.toStdString())
      {
        still_available = true;
        break;
      }
    }

    if (still_available)
    {
      it++;
    }
    else
    {
      treeWidget()->removeItemWidget(std::get<0>(it->second), 0);
      treeWidget()->removeItemWidget(std::get<0>(it->second)->child(0), 0);
      disks_group_item_->removeChild(std::get<0>(it->second));
      delete std::get<0>(it->second);
      std::get<1>(it->second)->deleteLater();
      std::get<2>(it->second)->deleteLater();
      it = disk_items_.erase(it);
    }
  }

  // Update Networks
  bool network_added = false;
  double cummulative_bytes_receive = 0.0;
  double cummulative_bytes_send = 0.0;
  for (auto& network : state.networks())
  {
    QString name(network.name().c_str());
    double bytes_receive = network.receive();
    double bytes_send    = network.send();
    bytes_receive = std::max(bytes_receive, 0.0);
    bytes_send    = std::max(bytes_send, 0.0);

    cummulative_bytes_receive += bytes_receive;
    cummulative_bytes_send    += bytes_send;

    //QTreeWidgetItem* network_io_item;
    MultiLabelItem*  network_io_widget = nullptr;
    // Check if we already have a network with that name
    if (network_items.find(name) != network_items.end())
    {
      // The network does exist
      network_io_widget = network_items[name].second;
    }
    else
    {
      // We need to create a new item for a new network connection.
      QTreeWidgetItem* network_item    = new QTreeWidgetItem(network_group_item_);
      QTreeWidgetItem* network_io_item = new QTreeWidgetItem(network_item);
      network_io_widget                = new MultiLabelItem(2, treeWidget());

      network_item->setText(0, name);
      network_items.emplace(name, std::make_pair(network_item, network_io_widget));

      // "measure" how wide the labels need to be
      QLabel test_label(treeWidget());
      test_label.setText(QString("\342\226\262 ") + "99.9 MiB/s");
      int io_width = test_label.sizeHint().width() + ADDITIONAL_MULTI_LABEL_SPACING;
      network_io_widget->setMinimumWidth(0, io_width);
      network_io_widget->setMinimumWidth(1, io_width);
      treeWidget()->setItemWidget(network_io_item, 0, network_io_widget);
      connect(this, &QObject::destroyed, network_io_widget, &QObject::deleteLater);

      network_added = true;
    }

    // Update the I/O information
    QString send_string = QString("\342\226\262 ") + normalizedDataAsString((unsigned long long) bytes_send) + QString("/s");
    QString receive_string = QString("\342\226\274 ") + normalizedDataAsString((unsigned long long) bytes_receive) + QString("/s");

    network_io_widget->setText(0, receive_string);
    network_io_widget->setText(1, send_string);
  }

  QString cummulative_receive_string = QString("\342\226\274 ") + normalizedDataAsString((unsigned long long) cummulative_bytes_receive) + QString("/s");
  QString cummulative_send_string    = QString("\342\226\262 ") + normalizedDataAsString((unsigned long long) cummulative_bytes_send) + QString("/s");

  network_group_widget_->setText(1, cummulative_receive_string);
  network_group_widget_->setText(2, cummulative_send_string);

  // Re-sort children if any network has been added
  if (network_added)
  {
    network_group_item_->sortChildren(0, Qt::SortOrder::AscendingOrder);
  }

  // Remove old networks that are not present any more. This will probably only happen on Linux.
  for (auto it = network_items.begin(); it != network_items.end();)
  {
    bool still_available = false;
    for (auto& network : state.networks())
    {
      if (network.name() == it->first.toStdString())
      {
        still_available = true;
        break;
      }
    }

    if (still_available)
    {
      it++;
    }
    else
    {
      treeWidget()->removeItemWidget(it->second.first->child(0), 0);
      network_group_item_->removeChild(it->second.first);
      delete it->second.first;
      it->second.second->deleteLater();
      it = network_items.erase(it);
    }
  }
}

QString MmaHostItem::normalizedDataAsString(unsigned long long bytes)
{
  if (bytes == 0)
  {
    return QString::number(bytes) + " B";
  }

  static const std::vector<std::pair<std::string, unsigned long long>> unit_table =
  { std::make_pair(std::string("PiB"), 1125899906842624) ,
    std::make_pair(std::string("TiB"),    1099511627776) ,
    std::make_pair(std::string("GiB"),       1073741824) ,
    std::make_pair(std::string("MiB"),          1048576) ,
    std::make_pair(std::string("KiB"),             1024) ,
    std::make_pair(std::string("B"),                  1) ,};
  
  for (std::pair<std::string, unsigned long long> unit : unit_table)
  {
    double temp = (double)bytes / (double)unit.second;
    if (temp >= 1.0)
    {
      // This is the correct unit!
      std::stringstream ss;
      if (temp < 10.0)
      {
        ss << std::setprecision(2) << std::fixed;
      }
      else if (temp < 100.0)
      {
        ss << std::setprecision(1) << std::fixed;
      }
      else if (temp < 1000.0)
      {
        ss << std::setprecision(0) << std::fixed;
      }
      else
      {
        ss << std::setprecision(0) << std::fixed;
      }
      ss << ((double)bytes / (double)unit.second) << " " << unit.first;
      return QString(ss.str().c_str());
    }
  }

  return QString::number(bytes) + " B";
}