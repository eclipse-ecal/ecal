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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

template <class T>
class ServiceTreeItem :
  public QAbstractTreeItem
{
public:

  enum class Columns : int
  {
    REGISTRATION_CLOCK,
    HOST_NAME,
    PNAME,
    UNAME,
    PID,
    SNAME,
    STYPE,
    TCP_PORT,
    MNAME,
    REQ_TYPE,
    RESP_TYPE,
    CALL_COUNT,
  };

  ServiceTreeItem() : QAbstractTreeItem()
  {
  }

  ServiceTreeItem(const T& service, const eCAL::pb::Method& method)
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
        return service_.registration_clock();
      }
      else if (column == Columns::HOST_NAME)
      {
        return service_.host_name().c_str();
      }
      else if (column == Columns::PNAME)
      {
        return service_.pname().c_str();
      }
      else if (column == Columns::UNAME)
      {
        return service_.uname().c_str();
      }
      else if (column == Columns::PID)
      {
        return service_.pid();
      }
      else if (column == Columns::SNAME)
      {
        return service_.sname().c_str();
      }
      else if (column == Columns::STYPE)
      {
        return std::is_same<T, eCAL::pb::Service>::value ? "Server" : "Client";
      }
      else if (column == Columns::TCP_PORT)
      {
        return tcpPort() != 0 ? tcpPort() : QVariant();
      }
      else if (column == Columns::MNAME)
      {
        return method_.mname().c_str();
      }
      else if (column == Columns::REQ_TYPE)
      {
        return method_.req_type().c_str();
      }
      else if (column == Columns::RESP_TYPE)
      {
        return method_.resp_type().c_str();
      }
      else if (column == Columns::CALL_COUNT)
      {
        return (long long)method_.call_count();
      }
      else
      {
        return QVariant();
      }
    }
    else if ((role == Qt::ItemDataRole::DisplayRole) || (role == Qt::ItemDataRole::ToolTipRole))
    {
      if ((column == Columns::HOST_NAME)
        || (column == Columns::PNAME)
        || (column == Columns::UNAME)
        || (column == Columns::SNAME)
        || (column == Columns::MNAME)
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
        || (column == Columns::PID)
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
        if (column == Columns::PNAME)
        {
          QStringList list{ service_.host_name().c_str(), service_.pname().c_str() };
          return list;
        }
        else if (column == Columns::PID)
        {
          QStringList list{ service_.host_name().c_str(), QString::number(service_.pid()) };
          return list;
        }
        else if (column == Columns::UNAME)
        {
          QStringList list{ service_.host_name().c_str(), service_.uname().c_str(), QString::number(service_.pid()) };
          return list;
        }
        else if (column == Columns::SNAME)
        {
          QStringList list{ service_.sname().c_str(), service_.host_name().c_str(), service_.uname().c_str(), QString::number(service_.pid()) };
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
        || (column == Columns::PNAME)
        || (column == Columns::UNAME)
        || (column == Columns::SNAME)
        || (column == Columns::STYPE)
        || (column == Columns::MNAME)
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

  static std::string generateIdentifier(const T& service, const eCAL::pb::Method& method)
  {
    return std::to_string(service.pid()) + "@" + service.host_name() + "@" + service.sname() + "@" + method.mname();
  }

  void update(const T& service, const eCAL::pb::Method& method)
  {
    service_.Clear();
    service_.CopyFrom(service);
    method_.Clear();
    method_.CopyFrom(method);
    identifier_ = generateIdentifier(service_, method_);
  }

private:
  // This workaround is required to utilize this template class with
  // eCAL::pb::Client even though it has no tcp_port_v1() signature
  // exposed. However, when upgrading eCAL to a newer C++ standard 
  // in the future, the workaround can be replace with if constexpr.
  template <class U = T>
  typename std::enable_if<std::is_same<eCAL::pb::Service, U>::value, int>::type
    tcpPort() const {
    return service_.tcp_port_v1();
  }
  template <class U = T>
  typename std::enable_if<std::is_same<eCAL::pb::Client, U>::value, int>::type
    tcpPort() const {
    return 0;
  }

  T service_;
  eCAL::pb::Method  method_;
  std::string identifier_;
};

