/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

ProcessTreeItem::ProcessTreeItem(const eCAL::pb::Process& process)
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
    if (column == Columns::RCLOCK)
    {
      return process_.rclock();
    }
    else if (column == Columns::HNAME)
    {
      return process_.hname().c_str();
    }
    else if (column == Columns::PID)
    {
      return process_.pid();
    }
    else if (column == Columns::PNAME)
    {
      return process_.pname().c_str();
    }
    else if (column == Columns::UNAME)
    {
      return process_.uname().c_str();
    }
    else if (column == Columns::PPARAM)
    {
      return process_.pparam().c_str();
    }
    else if (column == Columns::PMEMORY)
    {
      return (long long)process_.pmemory();
    }
    else if (column == Columns::PCPU)
    {
      return (long long)process_.pcpu();
    }
    else if (column == Columns::USRPTIME)
    {
      return process_.usrptime();
    }
    else if (column == Columns::DATAWRITE)
    {
      return (long long)process_.datawrite();
    }
    else if (column == Columns::DATAREAD)
    {
      return (long long)process_.dataread();
    }
    else if (column == Columns::SEVERITY)
    {
      severityToCombinedInt(process_.state().severity(), process_.state().severity_level());
    }
    else if (column == Columns::INFO)
    {
      return process_.state().info().c_str();
    }
    else if (column == Columns::TSYNC_STATE)
    {
      return (int)process_.tsync_state();
    }
    else if (column == Columns::TSYNC_MOD_NAME)
    {
      return process_.tsync_mod_name().c_str();
    }
    else if (column == Columns::COMPONENT_INIT_INFO)
    {
      return process_.component_init_info().c_str();
    }
    else if (column == Columns::ECAL_RUNTIME_VERSION)
    {
      return process_.ecal_runtime_version().c_str();
    }
    else
    {
      return QVariant();
    }
  }

  else if ((role == Qt::ItemDataRole::DisplayRole) || (role == Qt::ItemDataRole::ToolTipRole))
  {
    if ((column == Columns::HNAME)
      || (column == Columns::PNAME)
      || (column == Columns::UNAME))
    {
      QString raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
      return (!raw_data.isEmpty() ? raw_data : "- ? -");
    }
    else if (column == Columns::PMEMORY)
    {
      auto memory_bytes = process_.pmemory();
      if (memory_bytes == 0)
      {
        return "0";
      }
      else
      {
        return QString::number(memory_bytes / 1024.0, 'f', 0);
      }
    }
    else if (column == Columns::PCPU)
    {
      double cpu_percentage = process_.pcpu();
      if (fabs(cpu_percentage) < DBL_EPSILON)
      {
        return "0";
      }
      else
      {
        return QString::number(cpu_percentage, 'f', 2);
      }
    }
    else if (column == Columns::SEVERITY)
    {
      return severityToString(process_.state().severity(), process_.state().severity_level());
    }
    else if (column == Columns::TSYNC_STATE)
    {
      switch (process_.tsync_state())
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
    if ((column == Columns::HNAME)
      || (column == Columns::PNAME)
      || (column == Columns::UNAME))
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
    else if (column == Columns::TSYNC_STATE)
    {
      switch (process_.tsync_state())
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
    if ((column == Columns::RCLOCK)
      || (column == Columns::PID)
      || (column == Columns::PMEMORY)
      || (column == Columns::PCPU)
      || (column == Columns::USRPTIME)
      || (column == Columns::DATAWRITE)
      || (column == Columns::DATAREAD)
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
    if (column == Columns::PID)
    {
      QStringList list{ process_.hname().c_str(), QString::number(process_.pid()) };
      return list;
    }
    if (column == Columns::PNAME)
    {
      QStringList list{process_.hname().c_str(), process_.pname().c_str()};
      return list;
    }
    else if (column == Columns::UNAME)
    {
      QStringList list{ process_.hname().c_str(), process_.uname().c_str(), QString::number(process_.pid()) };
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
      switch (process_.state().severity())
      {
      case eCAL::pb::proc_sev_healthy:
        return QColor(80, 225, 120);
      case eCAL::pb::proc_sev_warning:
        return QColor(240, 240, 50);
      case eCAL::pb::proc_sev_critical:
        return QColor(250, 130, 0);
      case eCAL::pb::proc_sev_failed:
        return QColor(240, 20, 20);
      default:
        return QVariant::Invalid;
      }
    }
  }

  else if (role == Qt::ItemDataRole::FontRole)
  {
    if ((column == Columns::HNAME)
      || (column == Columns::PNAME)
      || (column == Columns::UNAME))
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
        return QVariant::Invalid;
      }
    }
    
    else if (column == Columns::TSYNC_STATE)
    {
      QFont font;
      font.setItalic(true);
      switch (process_.tsync_state())
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

  return QVariant::Invalid;
}

int ProcessTreeItem::type() const
{
  return (int)TreeItemType::Process;
}

void ProcessTreeItem::update(const eCAL::pb::Process& process)
{
  process_.Clear();
  process_.CopyFrom(process);
}

eCAL::pb::Process ProcessTreeItem::processPb()
{
  eCAL::pb::Process process_pb;
  process_pb.CopyFrom(process_);
  return process_pb;
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

int ProcessTreeItem::severityToCombinedInt(const eCAL::pb::eProcessSeverity severity, const eCAL::pb::eProcessSeverityLevel level) const
{
  int severity_level_integer = level - eCAL::pb::eProcessSeverityLevel_MIN;
  int severity_integer =  severity - eCAL::pb::eProcessSeverity_MIN;
  return (eCAL::pb::eProcessSeverityLevel_MAX - eCAL::pb::eProcessSeverityLevel_MIN) * severity_integer + severity_level_integer;
}

QString ProcessTreeItem::severityToString(const eCAL::pb::eProcessSeverity severity, const eCAL::pb::eProcessSeverityLevel level) const
{
  QString severity_string;
  switch (severity)
  {
  case eCAL::pb::proc_sev_unknown:
    return "Unknown";
  case eCAL::pb::proc_sev_healthy:
    severity_string = "Healthy";
    break;
  case eCAL::pb::proc_sev_warning:
    severity_string = "Warning";
    break;
  case eCAL::pb::proc_sev_critical:
    severity_string = "Critical";
    break;
  case eCAL::pb::proc_sev_failed:
    severity_string = "Failed";
    break;
  default:
    break;
  }

  switch (level)
  {
  //case eCAL::pb::proc_sev_level_unknown:
  //  severity_string += " (Unknown)";
  //  break;
  case eCAL::pb::proc_sev_level1:
    severity_string += " (Lv. 1)";
    break;
  case eCAL::pb::proc_sev_level2:
    severity_string += " (Lv. 2)";
    break;
  case eCAL::pb::proc_sev_level3:
    severity_string += " (Lv. 3)";
    break;
  case eCAL::pb::proc_sev_level4:
    severity_string += " (Lv. 4)";
    break;
  case eCAL::pb::proc_sev_level5:
    severity_string += " (Lv. 5)";
    break;
  default:
    break;
  }

  return severity_string;
}
