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

#include "mma_widget.h"

#include <QTimer>

#include "resource_bar.h"
#include "mma_widget.h"
#include "mma_host_item.h"

#include <globals.h>

MmaWidget::MmaWidget(QWidget *parent)
    : QWidget(parent)
{
  ui_.setupUi(this);
}

MmaWidget::~MmaWidget()
{
}

void MmaWidget::monitorUpdated()
{
  auto all_hosts = Globals::EcalSysInstance()->GetAllHosts();

  // Add new hosts if necessary
  bool hosts_added = false;
  for (std::string host : all_hosts)
  {
    auto hosts_item_it = host_items_.find(host);
    if (hosts_item_it == host_items_.end())
    {
      // The host is new
      MmaHostItem* host_item = new MmaHostItem(ui_.mma_tree, QString(host.c_str()));
      host_item->setSysClientAvailable((host == eCAL::Process::GetHostName()) || Globals::EcalSysInstance()->IseCALSysClientRunningOnHost(host));
      ui_.mma_tree->addTopLevelItem(host_item);
      host_items_[host] = host_item;
      hosts_added = true;
    }
    else
    {
      hosts_item_it->second->setSysClientAvailable((host == eCAL::Process::GetHostName()) || Globals::EcalSysInstance()->IseCALSysClientRunningOnHost(host));
    }
  }

  // Re-sort hosts, of at least one has been added
  if (hosts_added)
  {
    ui_.mma_tree->sortByColumn(0, Qt::SortOrder::AscendingOrder);
  }

  // Remove old hosts if necessary
  for (auto it = host_items_.begin(); it != host_items_.end();)
  {
    if (all_hosts.find(it->first) == all_hosts.end())
    {
      // The host is not present any more
      it->second->deleteLater();
      it = host_items_.erase(it);
    }
    else
    {
      // The host is still present and we move on
      it++;
    }
  }
}
