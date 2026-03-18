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

#include "CustomQt/QAbstractTreeItem.h"

#include <ecal/types/monitoring.h>

class ProcessTreeItem :
  public QAbstractTreeItem
{
public:

  enum class Columns : int
  {
    REGISTRATION_CLOCK,
    HOST_NAME,
    SHM_TRANSPORT_DOMAIN,
    PROCESS_ID,
    PROCESS_NAME,
    UNIT_NAME,
    PROCESS_PARAMETER,
    SEVERITY,
    INFO,
    TIME_SYNC_STATE,
    TSYNC_MOD_NAME,
    COMPONENT_INIT_INFO,
    ECAL_RUNTIME_VERSION,
    CONFIG_FILE_PATH
  };

  ProcessTreeItem();
  ProcessTreeItem(const eCAL::Monitoring::SProcess& process);

  ~ProcessTreeItem();

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  int type() const;

  void update(const eCAL::Monitoring::SProcess& process);

  eCAL::Monitoring::SProcess getProcess();


  int severityToCombinedInt(const int32_t severity, const int32_t level) const;

  QString severityToString(const int32_t severity, const int32_t level) const;


private:
  eCAL::Monitoring::SProcess process_;

  static QString toFrequencyString(long long freq);
};

