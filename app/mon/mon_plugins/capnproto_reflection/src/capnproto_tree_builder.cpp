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
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include "capnproto_tree_builder.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "monitor_tree_model.h"
#include "monitor_tree_item.h"

#include <QSet>
#include <QByteArray>

QString get_name(const capnp::StructSchema::Field& field)
{
  return field.getProto().getName().cStr();
}

bool hasSchema(capnp::Type type)
{
  auto which = type.which();
  switch (which)
  {
  case capnp::schema::Type::Which::LIST:
  case capnp::schema::Type::Which::ENUM:
  case capnp::schema::Type::Which::STRUCT:
  case capnp::schema::Type::Which::INTERFACE:
    return true;
  default:
    return false;
  }
}

QString primitiveTypeAsString(capnp::Type type)
{
  auto which = type.which();
  switch (which)
  {
  case capnp::schema::Type::Which::VOID:
    return "void";
  case capnp::schema::Type::Which::BOOL:
    return "bool";
  case capnp::schema::Type::Which::INT8:
    return "int8";
  case capnp::schema::Type::Which::INT16:
    return "int16";
  case capnp::schema::Type::Which::INT32:
    return "int32";
  case capnp::schema::Type::Which::INT64:
    return "int64";
  case capnp::schema::Type::Which::UINT8:
    return "uint8";
  case capnp::schema::Type::Which::UINT16:
    return "uint16";
  case capnp::schema::Type::Which::UINT32:
    return "uint32";
  case capnp::schema::Type::Which::UINT64:
    return "uint64";
  case capnp::schema::Type::Which::FLOAT32:
    return "float32";
  case capnp::schema::Type::Which::FLOAT64:
    return "float64";
  case capnp::schema::Type::Which::TEXT:
    return "Text";
  case capnp::schema::Type::Which::DATA:
    return "Data";
  case capnp::schema::Type::Which::LIST:
    return "List";
  case capnp::schema::Type::Which::ENUM:
    return "Enum";
  case capnp::schema::Type::Which::STRUCT:
    return "Struct";
  case capnp::schema::Type::Which::INTERFACE:
    return "Interface";
  case capnp::schema::Type::Which::ANY_POINTER:
    return "AnyPointer";
  default:
    return "";
  }
}


QString get_type_as_string(const capnp::Type& type)
{
  QString type_string;
  if (hasSchema(type))
  {
    if (type.isEnum())
    {
      type_string = type.asEnum().getShortDisplayName().cStr();
    }
    else if (type.isStruct())
    {
      type_string = type.asStruct().getShortDisplayName().cStr();
    }
    else if (type.isInterface())
    {
      type_string = type.asInterface().getShortDisplayName().cStr();
    }
    else if (type.isList())
    {
      type_string = get_type_as_string(type.asList().getElementType());
    }
  }
  else
  {
    type_string = primitiveTypeAsString(type);
  }
  return type_string;
}

QString get_type(const capnp::StructSchema::Field& field)
{
  return get_type_as_string(field.getType());
}


void MonitorTreeBuilder::ArrayStart(const MessageInfo &info_, size_t elements_)
{
  current_tree_item = current_tree_item->getChild(static_cast<int>(info_.id));
  current_tree_item->setMetaData({ info_.field_name ,  info_.type + "[" + QString::number(elements_) + "]", "" });
  current_tree_item->markChildrenUnused();
}

void MonitorTreeBuilder::ArrayValue(const MessageInfo & info_, const QVariant & value_)
{
  auto child = current_tree_item->getChild(static_cast<int>(info_.id));
  child->setValue(value_);
  child->setMetaData({ info_.field_name + "[" + QString::number(info_.id) + "]", info_.type, "" });

}

void MonitorTreeBuilder::ArrayEnd()
{
  MessageEnd();
}

void MonitorTreeBuilder::MessageStart(const MessageInfo &info_)
{
  if (current_tree_item == nullptr)
  {
    current_tree_item = (MonitorTreeItem*)model->root();
  }
  else
  {
    // we need to be able to get the childe by it's name
    current_tree_item = current_tree_item->getChild(static_cast<int>(info_.id));
  }

  current_tree_item->clearValue();
  current_tree_item->setMetaData({ info_.field_name ,  info_.type, "" });
  current_tree_item->markChildrenUnused();
}

void MonitorTreeBuilder::ScalarValue(const MessageInfo &info_, const QVariant & value_)
{
  auto child = current_tree_item->getChild(static_cast<int>(info_.id));
  child->setValue(value_);
  child->setMetaData({info_.field_name, info_.type, ""});
}

void MonitorTreeBuilder::MessageEnd()
{
  current_tree_item->deleteChildrenUnused();
  model->updateItem(current_tree_item);
  if (current_tree_item == model->root())
  {
    current_tree_item = nullptr;
  }
  else
  {
    current_tree_item = (MonitorTreeItem*)current_tree_item->parentItem();
  }
}

QVariant DynamicValueToVariant(capnp::DynamicValue::Reader reader)
{
  using namespace capnp;

  auto cpp_type = reader.getType();
  if (cpp_type == DynamicValue::INT)
  {
    return { static_cast<qlonglong>(reader.as<int64_t>()) };
  }
  else if (cpp_type == DynamicValue::UINT)
  {
    return { static_cast<qulonglong>(reader.as<uint64_t>()) };
  }
  else if (cpp_type == DynamicValue::FLOAT)
  {
    return{ reader.as<double>() };
  }
  else if (cpp_type == DynamicValue::BOOL)
  {
    return { reader.as<bool>() };
  }
  else if (cpp_type == DynamicValue::VOID)
  {
    return { "" };
  }
  else if (cpp_type == DynamicValue::ENUM)
  {
    auto enumValue = reader.as<DynamicEnum>();
    int raw_value = enumValue.getRaw();
    QString name;
    KJ_IF_MAYBE(enumerant, enumValue.getEnumerant()) {
      name = enumerant->getProto().getName().cStr();
    }
    QVariant variant;
    variant.setValue(StringEnum{ raw_value, name });
    return variant;
  }
  else if (cpp_type == DynamicValue::TEXT)
  {
    return { reader.as<Text>().cStr() };
  }
  else if (cpp_type == DynamicValue::DATA)
  {
    auto data = reader.as<Data>();
    auto chars{ data.asChars() };
    QByteArray byte_array(chars.begin(), static_cast<int>(chars.size()));

    return { byte_array };
  }
  return {};
}

void CapnprotoIterator::ProcessMessage(capnp::DynamicStruct::Reader message, const MonitorTreeBuilder::MessageInfo& parent_info)
{
  using namespace capnp;

  auto schema = message.getSchema();
  auto field_list = schema.getFields();

  tree_builder.MessageStart(parent_info);

  // iterate through fields to set data
  for (auto field : field_list)
  {
    if (message.has(field))
    {
      auto value = message.get(field);
      auto cpp_type = value.getType();
      bool is_repeated = (cpp_type == DynamicValue::LIST);

      if (is_repeated)
      {
        auto list_reader = value.as<DynamicList>();
        size_t repeated_size{list_reader.size()};
        MonitorTreeBuilder::MessageInfo info_array{ get_name(field) , parent_info.group_name + "." + parent_info.field_name , field.getIndex(), get_type(field)};
        tree_builder.ArrayStart(info_array, repeated_size);
        for (capnp::uint i = 0; i < repeated_size; i++)
        {
          MonitorTreeBuilder::MessageInfo info_value{ info_array.field_name , info_array.group_name, i, get_type(field) };
          auto element = list_reader[i];
          auto element_type = element.getType();
          if (element_type != DynamicValue::STRUCT)
          {
            QVariant display_value = DynamicValueToVariant(element);
            tree_builder.ArrayValue(info_value, display_value);
          }
          else
          {
            auto structValue = element.as<DynamicStruct>();
            ProcessMessage(structValue, MonitorTreeBuilder::MessageInfo{ info_value.field_name + "[" + QString::number(i) + "]", info_value.group_name, i,  info_value.type});
          }
        }
        tree_builder.ArrayEnd();
      }
      else
      {
        MonitorTreeBuilder::MessageInfo info{ get_name(field) , parent_info.group_name + "." + parent_info.field_name, field.getIndex(), get_type(field) };
        if (cpp_type != DynamicValue::STRUCT)
        {
          QVariant display_value = DynamicValueToVariant(value);
          tree_builder.ScalarValue(info, display_value);
        }
        else // if it is a struct, recursively call this function
        {
          auto structValue = value.as<DynamicStruct>();
          ProcessMessage(structValue, info);
        }
      }
    }
  }

  tree_builder.MessageEnd();
}
