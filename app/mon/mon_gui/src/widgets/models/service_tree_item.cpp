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

#include "service_tree_item.h"

#include <QColor>
#include <QFont>

#include "item_data_roles.h"

#include "tree_item_type.h"

ServiceTreeItem::ServiceTreeItem()
  : QAbstractTreeItem()
  , identifier_("")
{
}

ServiceTreeItem::ServiceTreeItem(const eCAL::pb::Service& service, const eCAL::pb::Method& method)
  : QAbstractTreeItem()
  , identifier_("")
{
  update(service, method);
}

ServiceTreeItem::~ServiceTreeItem()
{
}

QVariant ServiceTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant ServiceTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if (role == (Qt::ItemDataRole)ItemDataRoles::RawDataRole) //-V1016 //-V547
  {
    if (column == Columns::RCLOCK)
    {
      return service_.rclock();
    }
    else if (column == Columns::HNAME)
    {
      return service_.hname().c_str();
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
    else if (column == Columns::TCP_PORT)
    {
      return service_.tcp_port();
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
    if ((column == Columns::HNAME)
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
    if ((column == Columns::RCLOCK)
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
      QStringList list{service_.hname().c_str(), service_.pname().c_str()};
      return list;
    }
    else if (column == Columns::PID)
    {
      QStringList list{ service_.hname().c_str(), QString::number(service_.pid()) };
      return list;
    }
    else if (column == Columns::UNAME)
    {
      QStringList list{ service_.hname().c_str(), service_.uname().c_str(), QString::number(service_.pid()) };
      return list;
    }
    else if (column == Columns::SNAME)
    {
      QStringList list{ service_.sname().c_str(), service_.hname().c_str(), service_.uname().c_str(), QString::number(service_.pid()) };
      return list;
    }
    else
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
  }

  else if (role == Qt::ItemDataRole::FontRole)
  {
    if ((column == Columns::HNAME)
      || (column == Columns::PNAME)
      || (column == Columns::UNAME)
      || (column == Columns::SNAME)
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
        return QVariant::Invalid;
      }
    }
    else
    {
      return QVariant::Invalid;
    }
  }

  return QVariant::Invalid;
}

int ServiceTreeItem::type() const
{
  return (int)TreeItemType::Service;
}

std::string ServiceTreeItem::generateIdentifier(const eCAL::pb::Service& service, const eCAL::pb::Method& method)
{
  return std::to_string(service.pid()) + "@" + service.hname() + "@" + method.mname();
}

std::string ServiceTreeItem::identifier() const
{
  return identifier_;
}

void ServiceTreeItem::update(const eCAL::pb::Service& service, const eCAL::pb::Method& method)
{
  service_.Clear();
  service_.CopyFrom(service);
  method_.Clear();
  method_.CopyFrom(method);
  identifier_ = generateIdentifier(service_, method_);
}