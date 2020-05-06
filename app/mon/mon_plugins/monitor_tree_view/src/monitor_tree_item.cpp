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

#ifdef _MSC_VER
#pragma warning(disable : 4718)  // push and pop warnings not working ??
#endif
#include "monitor_tree_item.h"

#include "item_data_roles.h"

#include "tree_item_type.h"

#include <QSet>
#include <QByteArray>
#include "monitor_tree_model.h"

#include <QtDebug>


#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
QByteArray bytesToHex(const QByteArray& byte_array, char separator)
{
  QByteArray hex_string;
  hex_string.reserve(byte_array.size() * 2 + (separator ? byte_array.size() : 0));

  for (int i = 0; i < byte_array.size(); i++)
  {
    QByteArray temp_array;
    temp_array.push_back(byte_array[i]);
    hex_string += temp_array.toHex();
    if (separator)
    {
      hex_string += separator;
    }
  }
  return hex_string;
}
#endif //QT_VERSION

MonitorTreeItem::MonitorTreeItem(int number, MonitorTreeModel* model)
  : QAbstractTreeItem()
  , model_(model)
  , number_(number)
  , accessed_(false)
{}

MonitorTreeItem::~MonitorTreeItem()
{}

QVariant MonitorTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

// Return bytes as a byte array readable format
// But don't display all in case of of lenght > 256 bytes
QByteArray asByteArrayBlob(const QByteArray& bytes)
{
  int original_length = bytes.size();
  int max_length = 256;

  QByteArray chopped;
  if (original_length <= max_length)
  {
    chopped = bytes;
  }
  else
  {
    chopped.append(bytes.data(), max_length);
  }

  return "0x "
#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
    + bytesToHex(chopped, ' ')
#else //QT_VERSION
    + chopped.toHex(' ')
#endif // QT_VERSION
    + (original_length > max_length ? " ..." : "");
}

// Return crc16 checksum of the byte Array
QString asChecksum(const QByteArray& bytes)
{
  quint16 crc16 = qChecksum(bytes.data(), (uint)bytes.length());
  return QString("%1").arg(QString::number(crc16, 16).toUpper(), 4, '0');
}

QVariant MonitorTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if ((role == Qt::ItemDataRole::DisplayRole) || (role == Qt::ItemDataRole::ToolTipRole))
  {
    if (column == Columns::FIELD_NAME)
    {
      return meta_data_.field_name_;
    }
    else if (column == Columns::FIELD_RULE)
    {
      return meta_data_.field_rule_;
    }
    else if (column == Columns::NUMBER)
    {
      return number_;
    }
    else if (column == Columns::TYPE)
    {
      return meta_data_.type_;
    }
    else if (column == Columns::VALUE)
    {
      return getDisplayValue();
    }
    else
    {
      return QVariant();
    }
  }

  else if (role == ItemDataRoles::SortRole) //-V547
  {
    if (column == Columns::VALUE)
    {
      return data_;
    }
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  else if (role == ItemDataRoles::FilterRole) //-V547
  {
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  else if (role == Qt::ItemDataRole::TextAlignmentRole)
  {
    return Qt::AlignmentFlag::AlignLeft;
  }

  else if (role == ItemDataRoles::GroupRole) //-V547
  {
    if (column == Columns::VALUE)
    {
      return data_;
    }
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  return QVariant::Invalid;
}

int MonitorTreeItem::type() const
{
  return (int)TreeItemType::MonitoringContent;
}

void MonitorTreeItem::setValue(const QVariant& value)
{
  // In error-cases there may be children, although this is a single value.
  removeAllChildren();

  data_ = value;
}

void MonitorTreeItem::clearValue()
{
  data_ = QVariant();
}

MonitorTreeItem * MonitorTreeItem::getChild(int index)
{
  MonitorTreeItem* monitor_item = nullptr;
  for (int i = 0; i < childCount(); i++)
  {
    QAbstractTreeItem* item = child(i);
    if ((item->type() == (int)TreeItemType::MonitoringContent)
      && (static_cast<MonitorTreeItem*>(item)->data(Columns::NUMBER).toInt() == index))
    {
      // We found the (already existing) item
      monitor_item = static_cast<MonitorTreeItem*>(item);
      monitor_item->setAccessed(true);
      break;
    }
  }

  if (!monitor_item)
  {
    // If we didn't find the item, we create a new one
    monitor_item = new MonitorTreeItem(index, model_);
    monitor_item->setAccessed(true);
    model_->insertItem(monitor_item, model_->index(this), -1); // TODO: insert sorted?
  }

  return monitor_item;
}

void MonitorTreeItem::markChildrenUnused()
{
  for (int i = 0; i < childCount(); i++)
  {
    QAbstractTreeItem* item = child(i);
    if (item->type() == (int)TreeItemType::MonitoringContent)
    {
      static_cast<MonitorTreeItem*>(item)->setAccessed(false);
    }
  }
}

void MonitorTreeItem::deleteChildrenUnused()
{
  for (int i = childCount() - 1; i >= 0; i--) // iterate backwards
  {
    QAbstractTreeItem* item = child(i);
    if (item->type() == (int)TreeItemType::MonitoringContent)
    {
      if (!static_cast<MonitorTreeItem*>(item)->isAccessed())
      {
        model_->removeItem(item);
      }
    }
  }
}

void MonitorTreeItem::UpdateModel()
{
  model_->updateItem(this);
}

void MonitorTreeItem::setMetaData(const MonitorTreeMetaData& meta_data)
{
  meta_data_ = meta_data;
}

// Getter for the visitor pattern to check whether this child has been processed
bool MonitorTreeItem::isAccessed() const
{
  return accessed_;
}

// Setter for the visitor pattern to save whether this child has been processed
void MonitorTreeItem::setAccessed(bool accessed)
{
  accessed_ = accessed;
}

QVariant MonitorTreeItem::getDisplayValue() const
{
  if (data_.type() == QVariant::Type::ByteArray)
  {
    //TODO
    QByteArray bytes = data_.toByteArray();
    if (model_->displayBlobs())
    {
      return asByteArrayBlob(bytes);
    }
    else
    {
      return asChecksum(bytes);
    }
  }
  else if (data_.userType() == QMetaType::type("StringEnum"))
  {
    StringEnum string_enum = data_.value<StringEnum>();
    return string_enum.name + "(" + QString::number(string_enum.value) + ")";
  }
  else if (data_.userType() == (int)QMetaType::Bool)
  {
    return (data_.toBool() ? "True" : "False");
  }
  else if (data_.userType() == (int)QMetaType::Double)
  {
    // Print doubles with high enough accuracy
    // The 'g' specifier (which is the most useful for this case) doesn not allow
    // to specify the decimal points but significant digits altogether, see
    // http://doc.qt.io/qt-5/qstring.html#argument-formats
    return locale.toString(data_.toDouble(), 'g', 9);
  }
  else
  {
    return data_;
  }
}
