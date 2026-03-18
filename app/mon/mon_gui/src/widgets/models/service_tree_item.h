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

#include <QColor>
#include <QFont>

#include <type_traits>

#include "item_data_roles.h"
#include "tree_item_type.h"
#include <ecal/types/monitoring.h>

template <class T>
class ServiceTreeItem :
  public QAbstractTreeItem
{
public:

  enum class Columns : int
  {
    REGISTRATION_CLOCK,
    HOST_NAME,
    PROCESS_NAME,
    UNIT_NAME,
    PROCESS_ID,
    SERVICE_NAME,
    SERVICE_ID,
    STYPE,
    TCP_PORT,
    METHOD_NAME,
    REQ_TYPE,
    RESP_TYPE,
    CALL_COUNT,
  };

  ServiceTreeItem() : QAbstractTreeItem()
  {
  }

  ServiceTreeItem(const T& service, const eCAL::Monitoring::SMethod& method)
    : QAbstractTreeItem()
  {
    update(service, method);

  }

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const override
  {
    return data((Columns)column, role);
  }

  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const
  {
    if (role == (Qt::ItemDataRole)ItemDataRoles::RawDataRole) //-V1016 //-V547
    {
      if (column == Columns::REGISTRATION_CLOCK)
      {
        return service_.registration_clock;
      }
      else if (column == Columns::HOST_NAME)
      {
        return service_.host_name.c_str();
      }
      else if (column == Columns::PROCESS_NAME)
      {
        return service_.process_name.c_str();
      }
      else if (column == Columns::UNIT_NAME)
      {
        return service_.unit_name.c_str();
      }
      else if (column == Columns::PROCESS_ID)
      {
        return service_.process_id;
      }
      else if (column == Columns::SERVICE_NAME)
      {
        return service_.service_name.c_str();
      }
      else if (column == Columns::SERVICE_ID)
      {
        return std::to_string(service_.service_id).c_str();
      }
      else if (column == Columns::STYPE)
      {
        return std::is_same<T, eCAL::Monitoring::SServer>::value ? "Server" : "Client";
      }
      else if (column == Columns::TCP_PORT)
      {
        return tcpPort() != 0 ? tcpPort() : QVariant();
      }
      else if (column == Columns::METHOD_NAME)
      {
        return method_.method_name.c_str();
      }
      else if (column == Columns::REQ_TYPE)
      {
        return method_.request_datatype_information.name.c_str();
      }
      else if (column == Columns::RESP_TYPE)
      {
        return method_.response_datatype_information.name.c_str();
      }
      else if (column == Columns::CALL_COUNT)
      {
        return (long long)method_.call_count;
      }
      else
      {
        return QVariant();
      }
    }
    else if ((role == Qt::ItemDataRole::DisplayRole) || (role == Qt::ItemDataRole::ToolTipRole))
    {
      if ((column == Columns::HOST_NAME)
        || (column == Columns::PROCESS_NAME)
        || (column == Columns::UNIT_NAME)
        || (column == Columns::SERVICE_NAME)
        || (column == Columns::METHOD_NAME)
        || (column == Columns::REQ_TYPE)
        || (column == Columns::RESP_TYPE))
      {
        QString raw_data = data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
        return (!raw_data.isEmpty() ? raw_data : "- ? -");
      }
      else
      {
        return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
      }
    }

    else if (role == ItemDataRoles::SortRole) //-V547
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }

    else if (role == ItemDataRoles::FilterRole) //-V547
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }

    else if (role == Qt::ItemDataRole::TextAlignmentRole)
    {
      if ((column == Columns::REGISTRATION_CLOCK)
        || (column == Columns::PROCESS_ID)
        || (column == Columns::TCP_PORT)
        || (column == Columns::CALL_COUNT)
        )
      {
        return Qt::AlignmentFlag::AlignRight;
      }
      else
      {
        return Qt::AlignmentFlag::AlignLeft;
      }
    }

    else if (role == ItemDataRoles::GroupRole) //-V547
    {
        if (column == Columns::PROCESS_NAME)
        {
          QStringList list{ service_.host_name.c_str(), service_.process_name.c_str() };
          return list;
        }
        else if (column == Columns::PROCESS_ID)
        {
          QStringList list{ service_.host_name.c_str(), QString::number(service_.process_id) };
          return list;
        }
        else if (column == Columns::UNIT_NAME)
        {
          QStringList list{ service_.host_name.c_str(), service_.unit_name.c_str(), QString::number(service_.process_id) };
          return list;
        }
        else if (column == Columns::SERVICE_NAME)
        {
          QStringList list{ service_.service_name.c_str(), service_.host_name.c_str(), service_.unit_name.c_str(), QString::number(service_.process_id) };
          return list;
        }
        else
        {
          return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
        }
    }

    else if (role == Qt::ItemDataRole::FontRole)
    {
      if ((column == Columns::HOST_NAME)
        || (column == Columns::PROCESS_NAME)
        || (column == Columns::UNIT_NAME)
        || (column == Columns::SERVICE_NAME)
        || (column == Columns::STYPE)
        || (column == Columns::METHOD_NAME)
        || (column == Columns::REQ_TYPE)
        || (column == Columns::RESP_TYPE))
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
      else
      {
        return QVariant(); // Invalid QVariant
      }
    }

    return QVariant(); // Invalid QVariant
  }

  int type() const override
  {
    return (int)TreeItemType::Service;
  }

  std::string identifier() const
  {
    return identifier_;
  }

  static std::string generateIdentifier(const T& service, const eCAL::Monitoring::SMethod& method)
  {
    return std::to_string(service.process_id) + "@" + service.host_name + "@" + service.service_name + "@" + method.method_name;
  }

  void update(const T& service, const eCAL::Monitoring::SMethod& method)
  {
    service_ = service;
    method_ = method;
    identifier_ = generateIdentifier(service_, method_);
  }

private:
  // This workaround is required to utilize this template class with
  // eCAL::Monitoring::SClient even though it has no tcp_port_v1 field.
  // However, when upgrading eCAL to a newer C++ standard 
  // in the future, the workaround can be replace with if constexpr.
  template <class U = T>
  typename std::enable_if<std::is_same<eCAL::Monitoring::SServer, U>::value, int>::type
    tcpPort() const {
    return service_.tcp_port_v1;
  }
  template <class U = T>
  typename std::enable_if<std::is_same<eCAL::Monitoring::SClient, U>::value, int>::type
    tcpPort() const {
    return 0;
  }

  T service_;
  eCAL::Monitoring::SMethod  method_;
  std::string identifier_;
};

