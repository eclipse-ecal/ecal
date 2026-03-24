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

#include "process_tree_item.h"

#include <QColor>
#include <QFont>

#include <cmath>
#include <cfloat>

#include "item_data_roles.h"

#include "tree_item_type.h"

ProcessTreeItem::ProcessTreeItem()
  : QAbstractTreeItem()
{
}

ProcessTreeItem::ProcessTreeItem(const eCAL::Monitoring::SProcess& process)
  : QAbstractTreeItem()
{
  update(process);
}

ProcessTreeItem::~ProcessTreeItem()
{
}

QVariant ProcessTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant ProcessTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if (role == (Qt::ItemDataRole)ItemDataRoles::RawDataRole) //-V1016 //-V547
  {
    if (column == Columns::REGISTRATION_CLOCK)
    {
      return process_.registration_clock;
    }
    else if (column == Columns::HOST_NAME)
    {
      return process_.host_name.c_str();
    }
    else if (column == Columns::SHM_TRANSPORT_DOMAIN)
    {
      return process_.shm_transport_domain.c_str();
    }
    else if (column == Columns::PROCESS_ID)
    {
      return process_.process_id;
    }
    else if (column == Columns::PROCESS_NAME)
    {
      return process_.process_name.c_str();
    }
    else if (column == Columns::UNIT_NAME)
    {
      return process_.unit_name.c_str();
    }
    else if (column == Columns::PROCESS_PARAMETER)
    {
      return process_.process_parameter.c_str();
    }
    else if (column == Columns::SEVERITY)
    {
      return severityToCombinedInt(
        static_cast<eCAL::Process::eSeverity>(process_.state_severity), 
        static_cast<eCAL::Process::eSeverityLevel>(process_.state_severity_level)
      );
    }
    else if (column == Columns::INFO)
    {
      return process_.state_info.c_str();
    }
    else if (column == Columns::TIME_SYNC_STATE)
    {
      return (int)process_.time_sync_state;
    }
    else if (column == Columns::TSYNC_MOD_NAME)
    {
      return process_.time_sync_module_name.c_str();
    }
    else if (column == Columns::COMPONENT_INIT_INFO)
    {
      return process_.component_init_info.c_str();
    }
    else if (column == Columns::ECAL_RUNTIME_VERSION)
    {
      return process_.ecal_runtime_version.c_str();
    }
    else if (column == Columns::CONFIG_FILE_PATH)
    {
      return process_.config_file_path.c_str();
    }
    else
    {
      return QVariant();
    }
  }

  else if ((role == Qt::ItemDataRole::DisplayRole) || (role == Qt::ItemDataRole::ToolTipRole))
  {
    if ((column == Columns::HOST_NAME)
      || (column == Columns::SHM_TRANSPORT_DOMAIN)
      || (column == Columns::PROCESS_NAME)
      || (column == Columns::UNIT_NAME))
    {
      QString raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
      return (!raw_data.isEmpty() ? raw_data : "- ? -");
    }
    else if (column == Columns::SEVERITY)
    {
      return severityToString(
        static_cast<eCAL::Process::eSeverity>(process_.state_severity),
        static_cast<eCAL::Process::eSeverityLevel>(process_.state_severity_level));
    }
    else if (column == Columns::TIME_SYNC_STATE)
    {
      switch (process_.time_sync_state)
      {
      case 0:
        return "None";
      case 1:
        return "Realtime";
      case 2:
        return "Replay";
      default:
        return "- ? -";
      }
    }
    else
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
  }

  else if (role == (Qt::ItemDataRole)ItemDataRoles::SortRole) //-V1016 //-V547
  {
    return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
  }

  else if (role == (Qt::ItemDataRole)ItemDataRoles::FilterRole) //-V1016 //-V547
  {
    if ((column == Columns::HOST_NAME)
      || (column == Columns::SHM_TRANSPORT_DOMAIN)
      || (column == Columns::PROCESS_NAME)
      || (column == Columns::UNIT_NAME))
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
    else if (column == Columns::TIME_SYNC_STATE)
    {
      switch (process_.time_sync_state)
      {
      case 0:
        return "None";
      case 1:
        return "Realtime";
      case 2:
        return "Replay";
      default:
        return QVariant();
      }
    }
    else
    {
      return data(column, Qt::ItemDataRole::DisplayRole);
    }
  }

  else if (role == Qt::ItemDataRole::TextAlignmentRole)
  {
    if ((column == Columns::REGISTRATION_CLOCK)
      || (column == Columns::PROCESS_ID)
      )
    {
      return Qt::AlignmentFlag::AlignRight;
    }
    else
    {
      return Qt::AlignmentFlag::AlignLeft;
    }
  }

  else if (role == (Qt::ItemDataRole)ItemDataRoles::GroupRole) //-V1016 //-V547
  {
    if (column == Columns::PROCESS_ID)
    {
      const QStringList list{ process_.host_name.c_str(), QString::number(process_.process_id) };
      return list;
    }
    if (column == Columns::PROCESS_NAME)
    {
      const QStringList list{process_.host_name.c_str(), process_.process_name.c_str()};
      return list;
    }
    else if (column == Columns::UNIT_NAME)
    {
      const QStringList list{ process_.host_name.c_str(), process_.unit_name.c_str(), QString::number(process_.process_id) };
      return list;
    }
    else
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
  }

  else if (role == Qt::ItemDataRole::BackgroundRole)
  {
    if (column == Columns::SEVERITY)
    {
      auto severity = static_cast<eCAL::Process::eSeverity>(process_.state_severity);
      switch (severity)
      {
      case eCAL::Process::eSeverity::healthy:
        return QColor(80, 225, 120);
      case eCAL::Process::eSeverity::warning:
        return QColor(240, 240, 50);
      case eCAL::Process::eSeverity::critical:
        return QColor(250, 130, 0);
      case eCAL::Process::eSeverity::failed:
        return QColor(240, 20, 20);
      default:
        return QVariant(); // Invalid QVariant
      }
    }
  }

  else if (role == Qt::ItemDataRole::FontRole)
  {
    if ((column == Columns::HOST_NAME)
      || (column == Columns::SHM_TRANSPORT_DOMAIN)
      || (column == Columns::PROCESS_NAME)
      || (column == Columns::UNIT_NAME))
    {
      QString raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
      if (raw_data.isEmpty())
      {
        QFont font;
        font.setItalic(true);
        return font;
      }
      else
      {
        return QVariant(); // Invalid QVariant
      }
    }
    
    else if (column == Columns::TIME_SYNC_STATE)
    {
      QFont font;
      font.setItalic(true);
      switch (process_.time_sync_state)
      {
      case 0:
      case 1:
      case 2:
        return QVariant();
      default:
        return font;
      }
    }

    else
    {
      return QVariant();
    }
  }

  return QVariant(); // Invalid QVariant
}

int ProcessTreeItem::type() const
{
  return (int)TreeItemType::Process;
}

void ProcessTreeItem::update(const eCAL::Monitoring::SProcess& process)
{
  process_ = process;
}

eCAL::Monitoring::SProcess ProcessTreeItem::getProcess()
{
  return process_;
}

QString ProcessTreeItem::toFrequencyString(long long freq)
{
  if (freq == 0)
  {
    return "0";
  }
  else
  {
    return QString::number(((double)freq) / 1000.0, 'f', 2);
  }
}

int ProcessTreeItem::severityToCombinedInt(const eCAL::Process::eSeverity severity, const eCAL::Process::eSeverityLevel level) const
{
  return static_cast<int>(severity) * static_cast<int>(eCAL::Process::eSeverity::count) + static_cast<int>(level);
}

QString ProcessTreeItem::severityToString(const eCAL::Process::eSeverity severity, const eCAL::Process::eSeverityLevel level) const
{
  QString severity_string;
  switch (severity)
  {
  case eCAL::Process::eSeverity::unknown:
    return "Unknown";
  case eCAL::Process::eSeverity::healthy:
    severity_string = "Healthy";
    break;
  case eCAL::Process::eSeverity::warning:
    severity_string = "Warning";
    break;
  case eCAL::Process::eSeverity::critical:
    severity_string = "Critical";
    break;
  case eCAL::Process::eSeverity::failed:
    severity_string = "Failed";
    break;
  default:
    break;
  }

  switch (level)
  {
  //case eCAL::Registration::eProcessSeverityLevel::proc_sev_level_unknown:
  //  severity_string += " (Unknown)";
  //  break;
  case eCAL::Process::eSeverityLevel::level1:
    severity_string += " (Lv. 1)";
    break;
  case eCAL::Process::eSeverityLevel::level2:
    severity_string += " (Lv. 2)";
    break;
  case eCAL::Process::eSeverityLevel::level3:
    severity_string += " (Lv. 3)";
    break;
  case eCAL::Process::eSeverityLevel::level4:
    severity_string += " (Lv. 4)";
    break;
  case eCAL::Process::eSeverityLevel::level5:
    severity_string += " (Lv. 5)";
    break;
  default:
    break;
  }

  return severity_string;
}
