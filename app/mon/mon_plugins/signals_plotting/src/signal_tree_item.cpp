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

#include "signal_tree_item.h"
#include "signal_tree_model.h"


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

SignalTreeItem::SignalTreeItem(int number, SignalTreeModel* model)
  : QAbstractTreeItem()
  , model_(model)
  , number_(number)
  , multi_checked_(Qt::CheckState::Unchecked)
  , single_checked_(Qt::CheckState::Unchecked)
  , accessed_(false)
  , background_color_(QColor())
{
  connect(this, &SignalTreeItem::sgn_itemValueChanged, model_, &SignalTreeModel::slt_itemValueChanged);
}

SignalTreeItem::~SignalTreeItem()
{}

QVariant SignalTreeItem::data(int column, Qt::ItemDataRole role) const
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

QVariant SignalTreeItem::data(Columns column, Qt::ItemDataRole role) const
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
  else if (role == Qt::ItemDataRole::TextAlignmentRole)
  {
    return Qt::AlignmentFlag::AlignLeft;
  }
  else if (role == Qt::ItemDataRole::CheckStateRole)
  {
    auto valid = this->flags((int)column).testFlag(Qt::ItemIsUserCheckable);
    if (valid)
    {
      if (column == Columns::MULTI)
        return multi_checked_;
      if (column == Columns::SINGLE)
        return single_checked_;
    }
    else
      return QVariant();
  }
  else if (role == Qt::BackgroundRole)
  {
    if (background_color_.isValid())
    {
      return background_color_.toQColor();
    }
  }
  return QVariant::Invalid;
}

Qt::ItemFlags SignalTreeItem::flags(int column) const
{
  static auto default_flags = Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  static auto check_flags = Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsUserCheckable;

  switch (column)
  {
  case (int)Columns::MULTI:
  case (int)Columns::SINGLE:
    if (data_.isValid())
    {
      if (data_.userType() == QMetaType::type("StringEnum"))
      {
        return check_flags;
      }
      switch ((QMetaType::Type)data_.type())
      {
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::Double:
      case QMetaType::Long:
      case QMetaType::Short:
      case QMetaType::ULong:
      case QMetaType::Float:
      case QMetaType::Bool:
        return check_flags;
      default:
        return default_flags;
      }
    }
    else
      return default_flags;
  default:
    return default_flags;
  }

}

Qt::CheckState SignalTreeItem::getItemCheckedState(int model_column) const
{
  if (model_column == (int)Columns::MULTI)
  {
    return multi_checked_;
  }
  else
  {
    return single_checked_;
  }
}

void SignalTreeItem::setItemCheckedState(Qt::CheckState state, int model_column)
{
  if (model_column == (int)Columns::MULTI)
  {
    multi_checked_ = state;
  }
  else
  {
    single_checked_ = state;
  }
}

void SignalTreeItem::setItemBackgroundColor(const QColor& color)
{
  background_color_.clear();
  background_color_ = SignalPlotting::Color(color);
}

int SignalTreeItem::type() const
{
  return kMonitoringContent;
}

void SignalTreeItem::setValue(const QVariant& value)
{
  // In error-cases there may be children, although this is a single value.
  removeAllChildren();

  data_ = value;
  emit sgn_itemValueChanged(this);
}

QVariant& SignalTreeItem::getValue()
{
  return data_;
}

void SignalTreeItem::clearValue()
{
  data_ = QVariant();
}

SignalTreeItem* SignalTreeItem::getChild(int index)
{
  SignalTreeItem* monitor_item = nullptr;
  for (int i = 0; i < childCount(); i++)
  {
    QAbstractTreeItem* item = child(i);
    if ((item->type() == kMonitoringContent)
      && (static_cast<SignalTreeItem*>(item)->data(Columns::NUMBER).toInt() == index))
    {
      // We found the (already existing) item
      monitor_item = static_cast<SignalTreeItem*>(item);
      monitor_item->setAccessed(true);
      break;
    }
  }

  if (!monitor_item)
  {
    // If we didn't find the item, we create a new one
    monitor_item = new SignalTreeItem(index, model_);
    monitor_item->setAccessed(true);
    model_->insertItem(monitor_item, model_->index(this), -1); // TODO: insert sorted?
  }

  return monitor_item;
}

void SignalTreeItem::markChildrenUnused()
{
  for (int i = 0; i < childCount(); i++)
  {
    QAbstractTreeItem* item = child(i);
    if (item->type() == kMonitoringContent)
    {
      static_cast<SignalTreeItem*>(item)->setAccessed(false);
    }
  }
}

void SignalTreeItem::deleteChildrenUnused()
{
  for (int i = childCount() - 1; i >= 0; i--) // iterate backwards
  {
    QAbstractTreeItem* item = child(i);
    if (item->type() == kMonitoringContent)
    {
      if (!static_cast<SignalTreeItem*>(item)->isAccessed())
      {
        model_->removeItem(item);
      }
    }
  }
}

void SignalTreeItem::UpdateModel()
{
  model_->updateItem(this);
}

void SignalTreeItem::setMetaData(const SignalTreeMetaData& meta_data)
{
  meta_data_ = meta_data;
}

// Getter for the visitor pattern to check whether this child has been processed
bool SignalTreeItem::isAccessed() const
{
  return accessed_;
}

// Setter for the visitor pattern to save whether this child has been processed
void SignalTreeItem::setAccessed(bool accessed)
{
  accessed_ = accessed;
}

QVariant SignalTreeItem::getDisplayValue() const
{
  if (data_.type() == QVariant::Type::ByteArray)
  {
    QByteArray bytes = data_.toByteArray();
    return asChecksum(bytes);
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
