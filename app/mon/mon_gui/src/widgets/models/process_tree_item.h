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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/process.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
  ProcessTreeItem(const eCAL::pb::Process& process);

  ~ProcessTreeItem();

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  int type() const;

  void update(const eCAL::pb::Process& process);

  eCAL::pb::Process processPb();


  int severityToCombinedInt(const eCAL::pb::eProcessSeverity severity, const eCAL::pb::eProcessSeverityLevel level) const;

  QString severityToString(const eCAL::pb::eProcessSeverity severity, const eCAL::pb::eProcessSeverityLevel level) const;


private:
  eCAL::pb::Process process_;

  static QString toFrequencyString(long long freq);
};

