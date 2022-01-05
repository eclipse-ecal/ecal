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

#include "protobuf_tree_item.h"

#include "item_data_roles.h"

#include "tree_item_type.h"

#include <QSet>
#include <QByteArray>
#include "protobuf_tree_model.h"

ProtobufTreeItem::ProtobufTreeItem(int number, ProtobufTreeModel* model)
  : QAbstractTreeItem()
  , model_(model)
  , number_(number)
{}

ProtobufTreeItem::~ProtobufTreeItem()
{}

QVariant ProtobufTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant ProtobufTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if ((role == Qt::ItemDataRole::DisplayRole) || (role == Qt::ItemDataRole::ToolTipRole))
  {
    if (column == Columns::FIELD_NAME)
    {
      return field_name_;
    }
    else if (column == Columns::FIELD_RULE)
    {
      return field_rule_;
    }
    else if (column == Columns::NUMBER)
    {
      return number_;
    }
    else if (column == Columns::TYPE)
    {
      return type_;
    }
    else if (column == Columns::VALUE)
    {
      if (raw_value_.type() == QVariant::Type::ByteArray)
      {
        if (model_->displayBlobs())
        {
          QByteArray bytes = raw_value_.toByteArray();

          int original_length = bytes.size();
          int max_length = 256;

          if (original_length >= max_length)
          {
            bytes.chop(original_length - max_length);
          }
          return "0x " 
#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
            + bytesToHex(bytes, ' ')
#else //QT_VERSION
            + bytes.toHex(' ') 
#endif // QT_VERSION
            + (original_length > max_length ? " ..." : "");
        }
        else
        {
          return display_value_;
        }
      }
      else if ((raw_value_.type() == QVariant::Type::Double)
        || (raw_value_.userType() == (int)QMetaType::Float))
      {
        return QString::number(raw_value_.toDouble());
      }
      else
      {
        return display_value_;
      }
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
      return raw_value_;
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
      return raw_value_;
    }
    return data(column, Qt::ItemDataRole::DisplayRole);
  }

  return QVariant::Invalid;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
QString ProtobufTreeItem::bytesToHex(const QByteArray& byte_array, char separator)
{
  QString hex_string;
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

int ProtobufTreeItem::type() const
{
  return (int)TreeItemType::Protobuf;
}

void ProtobufTreeItem::setValue(const QVariant& raw_value, const QVariant& display_value)
{
  // In error-cases there may be children, although this is a single value.
  removeAllChildren();
  children_.clear();

  raw_value_ = raw_value;

  if (display_value.isValid())
  {
    display_value_ = display_value;
  }
  else
  {
    if (raw_value.userType() == (int)QMetaType::Bool)
    {
      display_value_ = (raw_value.toBool() ? "True" : "False");
    }
    else
    {
      display_value_ = raw_value_;
    }
  }
}

void ProtobufTreeItem::setMessage(const google::protobuf::Message& message)
{
  // In error-cases there may be a value stored, although this is a message node.
  raw_value_ = QVariant();
  display_value_ = QVariant();

  const google::protobuf::Reflection* reflection = message.GetReflection();

  if (!reflection)
  {
    removeAllChildren();
    children_.clear();
    return;
  }

  int count = message.GetDescriptor()->field_count();

  // Remove old Items that are not present any more
  QSet<int> existing_field_numbers;
  for (int i = 0; i < count; ++i)
  {
    auto field = message.GetDescriptor()->field(i);
    existing_field_numbers.insert(field->number());
  }
  for (int child_number : children_.keys())
  {
    if (!existing_field_numbers.contains(child_number))
    {
      model_->removeItem(children_.value(child_number));
      children_.remove(child_number);
    }
  }

  for (int i = 0; i < count; ++i)
  {
    auto field = message.GetDescriptor()->field(i);
    ProtobufTreeItem* child;
    int field_number = field->number();

    // Get an existing child or create a new one, based on the field's number
    if (children_.contains(field_number))
    {
      child = children_[field_number];
    }
    else
    {
      child = new ProtobufTreeItem(field_number, model_);

      // Insert the new child sorted
      int insert_row = -1;
      for (int existing_number : children_.keys())
      {
        if (existing_number > field_number)
        {
          insert_row = children_[existing_number]->row();
          break;
        }
      }

      model_->insertItem(child, model_->index(this), insert_row);
      children_[field_number] = child;
    }

    child->setField(message, field);
  }
}

void ProtobufTreeItem::setField(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field_descriptor)
{
  setFieldMetaInformation(message, field_descriptor);

  auto cpp_type    = field_descriptor->cpp_type();
  auto proto_type  = field_descriptor->type();
  auto reflection  = message.GetReflection();
  bool is_repeated = field_descriptor->is_repeated();

  if (is_repeated)
  {
    // In error-cases there may be a value stored, although this is a repeated-message-group-node.
    raw_value_ = QVariant();
    display_value_ = QVariant();

    int repeated_size = reflection->FieldSize(message, field_descriptor);
    int child_count   = children_.size();

    // Remove non-existing items
    for (int i = repeated_size; i < child_count; i++)
    {
      // As this is a repeated field and the numbers _always_ start with 0,1,2,... we simply remove all items with numbers that are too high.
      model_->removeItem(children_.value(i));
      children_.remove(i);
    }
    
    for (int i = 0; i < repeated_size; i++)
    {
      ProtobufTreeItem* child;

      if (children_.contains(i))
      {
        child = children_[i];
      }
      else
      {
        child = new ProtobufTreeItem(i, model_);

        // Insert the new child sorted (This is only relevant in error-cases, where a message node has been transformed into a repeated node
        int insert_row = -1;
        for (int existing_number : children_.keys())
        {
          if (existing_number > i)
          {
            insert_row = children_[existing_number]->row();
            break;
          }
        }
        model_->insertItem(child, model_->index(this), insert_row);
        children_[i] = child;
      }
      child->setRepeatedField(message, field_descriptor, i);
    }
  }
  else
  {
    if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_INT32)
    {
      setValue(reflection->GetInt32(message, field_descriptor));
    }
    else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_INT64)
    {
      setValue((long long)reflection->GetInt64(message, field_descriptor));
    }
    else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_UINT32)
    {
      setValue(reflection->GetUInt32(message, field_descriptor));
    }
    else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_UINT64)
    {
      setValue((unsigned long long)reflection->GetUInt64(message, field_descriptor));
    }
    else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE)
    {
      setValue(reflection->GetDouble(message, field_descriptor));
    }
    else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT)
    {
      setValue(reflection->GetFloat(message, field_descriptor));
    }
    else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_BOOL)
    {
      setValue(reflection->GetBool(message, field_descriptor));
    }
    else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_ENUM)
    {
      auto proto_enum = reflection->GetEnum(message, field_descriptor);
      int raw_value = proto_enum->number();
      QString name = proto_enum->name().c_str();
      setValue(raw_value, name + " (" + QString::number(raw_value) + ")");
    }
    else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_STRING)
    {
      // Byte arrays are passed as strings, but we want to differ between those.
      if (proto_type == google::protobuf::FieldDescriptor::TYPE_STRING)
      {
        setValue(reflection->GetString(message, field_descriptor).c_str());
      }
      else
      {
        std::string string_data = reflection->GetString(message, field_descriptor);
        quint16 crc16 = qChecksum(string_data.data(), (uint)string_data.length());
        QString crc16_string = QString("%1").arg(QString::number(crc16, 16).toUpper(), 4, '0');
        setValue(QByteArray(string_data.data(), std::min(257, (int)string_data.size())), "Binary data of " + QString::number((int)string_data.size()) + " Bytes (CRC16: " + crc16_string + ")");
      }
    }
    else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
    {
      setMessage(reflection->GetMessage(message, field_descriptor));
    }
  }
  model_->updateItem(this);
}

void ProtobufTreeItem::setRepeatedField(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field_descriptor, int index)
{
  setRepeatedFieldMetaInformation(message, field_descriptor, index);

  auto cpp_type   = field_descriptor->cpp_type();
  auto proto_type = field_descriptor->type();
  auto reflection = message.GetReflection();

  if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_INT32)
  {
    setValue(reflection->GetRepeatedInt32(message, field_descriptor, index));
  }
  else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_INT64)
  {
    setValue((long long)reflection->GetRepeatedInt64(message, field_descriptor, index));
  }
  else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_UINT32)
  {
    setValue(reflection->GetRepeatedUInt32(message, field_descriptor, index));
  }
  else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_UINT64)
  {
    setValue((unsigned long long)reflection->GetRepeatedUInt64(message, field_descriptor, index));
  }
  else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE)
  {
    setValue(reflection->GetRepeatedDouble(message, field_descriptor, index));
  }
  else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT)
  {
    setValue(reflection->GetRepeatedFloat(message, field_descriptor, index));
  }
  else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_BOOL)
  {
    setValue(reflection->GetRepeatedBool(message, field_descriptor, index));
  }
  else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_ENUM)
  {
    auto proto_enum = reflection->GetRepeatedEnum(message, field_descriptor, index);
    int raw_value = proto_enum->number();
    QString name = proto_enum->name().c_str();
    setValue(raw_value, name + " (" + QString::number(raw_value) + ")");
  }
  else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_STRING)
  {
    // Byte arrays are passed as strings, but we want to differ between those.
    if (proto_type == google::protobuf::FieldDescriptor::TYPE_STRING)
    {
      setValue(reflection->GetRepeatedString(message, field_descriptor, index).c_str());
    }
    else
    {
      std::string string_data = reflection->GetRepeatedString(message, field_descriptor, index);
      setValue(QByteArray(string_data.data(), std::min(257, (int)string_data.size())), "Binary data of " + QString::number((int)string_data.size()) + " Bytes");
    }
  }
  else if (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
  {
    setMessage(reflection->GetRepeatedMessage(message, field_descriptor, index));
  }
  model_->updateItem(this);
}

void ProtobufTreeItem::setFieldMetaInformation(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field_descriptor)
{
  auto reflection = message.GetReflection();

  auto cpp_type = field_descriptor->cpp_type();

  bool is_repeated  = field_descriptor->is_repeated();
  int repeated_size = (is_repeated ? reflection->FieldSize(message, field_descriptor) : 0);
  bool is_message   = (cpp_type == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE);

  // Name
  field_name_ = field_descriptor->name().c_str();

  // Type
  type_       = field_descriptor->type_name();

  if (is_repeated)
  {
    type_ += (" [" + QString::number(repeated_size) + "]");

    if (is_message)
    {
      if (repeated_size > 0)
      {
        QString message_type = reflection->GetRepeatedMessage(message, field_descriptor, 0).GetTypeName().c_str();
        if (!message_type.isEmpty())
        {
          type_ += (" (" + message_type + ")");
        }
      }
    }
  }
  else
  {
    if (is_message)
    {
      QString message_type = reflection->GetMessage(message, field_descriptor).GetTypeName().c_str();
      if (!message_type.isEmpty())
      {
        type_ += (" (" + message_type + ")");
      }
    }
  }

  // Rule
  if (is_repeated)
    field_rule_ = "repeated";
  else if (field_descriptor->is_required())
    field_rule_ = "required";
  else if (field_descriptor->is_optional())
    field_rule_ = "optional";
  else
    field_rule_.clear();
}

void ProtobufTreeItem::setRepeatedFieldMetaInformation(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field_descriptor, int index)
{
  auto reflection = message.GetReflection();

  bool is_message = (field_descriptor->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE);

  // Name
  field_name_ = (QString(field_descriptor->name().c_str()) + " [" + QString::number(index) + "]");

  // Type
  type_       = field_descriptor->type_name();

  if (is_message)
  {
    QString message_type = reflection->GetRepeatedMessage(message, field_descriptor, 0).GetTypeName().c_str();
    if (!message_type.isEmpty())
    {
      type_ += (" (" + message_type + ")");
    }
  }

  // Rule
  field_rule_.clear();
}