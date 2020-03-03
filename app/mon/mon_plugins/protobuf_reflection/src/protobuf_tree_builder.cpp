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

#include "protobuf_tree_builder.h"

void ProtobufTreeBuilder::ArrayStart(const eCAL::protobuf::MessageInfo& info_, const google::protobuf::FieldDescriptor::Type &, size_t elements_)
{
  current_tree_item = current_tree_item->getChild(info_.id);
  current_tree_item->setMetaData({ QString::fromStdString(info_.field_name)  ,  QString::fromStdString(info_.type) + "[" + QString::number(elements_)  + "]", "" });
  current_tree_item->markChildrenUnused();
}

void ProtobufTreeBuilder::ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::int32  value_)
{
  ArrayValue(info_, value_);
}

void ProtobufTreeBuilder::ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::uint32 value_)
{
  ArrayValue(info_, value_);
}

void ProtobufTreeBuilder::ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::int64  value_)
{
  ArrayValue(info_, (qlonglong)value_);
}

void ProtobufTreeBuilder::ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::uint64 value_)
{
  ArrayValue(info_, (qulonglong)value_);
}

void ProtobufTreeBuilder::ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, float                    value_)
{
  ArrayValue(info_, value_);
}

void ProtobufTreeBuilder::ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, double value_)
{
  ArrayValue(info_, value_);
}

void ProtobufTreeBuilder::ArrayValueIntegral(const eCAL::protobuf::MessageInfo& info_, bool value_)
{
  ArrayValue(info_, value_);
}

void ProtobufTreeBuilder::ArrayValueString(const eCAL::protobuf::MessageInfo& info_, const std::string& value_)
{
  ArrayValue(info_, QString::fromStdString(value_));
}

void ProtobufTreeBuilder::ArrayValueBytes(const eCAL::protobuf::MessageInfo& info_, const std::string& value_)
{
  ArrayValue(info_, QByteArray::fromStdString(value_));
}

void ProtobufTreeBuilder::ArrayValueEnum(const eCAL::protobuf::MessageInfo& info_, int int_value_, const std::string & value_)
{
  QVariant variant;
  variant.setValue(StringEnum{ int_value_, QString{ value_.c_str() } });
  ArrayValue(info_, variant);
}

void ProtobufTreeBuilder::ArrayEnd()
{
  MessageEnd();
}

void ProtobufTreeBuilder::ScalarValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::int32  value_)
{
  ScalarValue(info_, value_);
}

void ProtobufTreeBuilder::ScalarValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::uint32 value_)
{
  ScalarValue(info_, value_);
}

void ProtobufTreeBuilder::ScalarValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::int64  value_)
{
  ScalarValue(info_, (qlonglong)value_);
}

void ProtobufTreeBuilder::ScalarValueIntegral(const eCAL::protobuf::MessageInfo& info_, google::protobuf::uint64 value_)
{
  ScalarValue(info_, (qulonglong)value_);
}

void ProtobufTreeBuilder::ScalarValueIntegral(const eCAL::protobuf::MessageInfo& info_, float                    value_)
{
  ScalarValue(info_, value_);
}

void ProtobufTreeBuilder::ScalarValueIntegral(const eCAL::protobuf::MessageInfo& info_, double value_)
{
  ScalarValue(info_, value_);
}

void ProtobufTreeBuilder::ScalarValueIntegral(const eCAL::protobuf::MessageInfo& info_, bool value_)
{
  ScalarValue(info_, value_);
}

void ProtobufTreeBuilder::ScalarValueString(const eCAL::protobuf::MessageInfo& info_, const std::string& value_)
{
  ScalarValue(info_, QString::fromStdString(value_));
}

void ProtobufTreeBuilder::ScalarValueBytes(const eCAL::protobuf::MessageInfo& info_, const std::string& value_)
{
  ScalarValue(info_, QByteArray::fromStdString(value_));
}

void ProtobufTreeBuilder::ScalarValueEnum(const eCAL::protobuf::MessageInfo& info_, int int_value_, const std::string &value_)
{
  QVariant variant;
  variant.setValue(StringEnum{ int_value_, QString{ value_.c_str() } });
  ScalarValue(info_, variant);
}

void ProtobufTreeBuilder::ScalarMessageStart(const eCAL::protobuf::MessageInfo& info_, const std::vector<std::string>& /*fields_*/)
{
  MessageStart(info_);
}

void ProtobufTreeBuilder::ScalarMessageEnd()
{
  MessageEnd();
}

void ProtobufTreeBuilder::ArrayMessageStart(const eCAL::protobuf::MessageInfo& info_, const std::vector<std::string>&)
{
  MessageStart(info_);
}

void ProtobufTreeBuilder::ArrayMessageEnd()
{
  MessageEnd();
}

/*
 * These functions are actually doing the work! The others are just forwarding the arguments.
 */
void ProtobufTreeBuilder::ArrayValue(const eCAL::protobuf::MessageInfo& info_, const QVariant &value_)
{
  auto child = current_tree_item->getChild(info_.id);
  child->setValue(value_);
  child->setMetaData({ QString::fromStdString(info_.field_name) + "[" + QString::number(info_.id) + "]", QString::fromStdString(info_.type), "" });
}

void ProtobufTreeBuilder::ScalarValue(const eCAL::protobuf::MessageInfo& info_, const QVariant &value_)
{
  auto child = current_tree_item->getChild(info_.id);
  child->setValue(value_);
  child->setMetaData({ QString::fromStdString(info_.field_name),  QString::fromStdString(info_.type), ""});
}

void ProtobufTreeBuilder::MessageStart(const eCAL::protobuf::MessageInfo& info_)
{
  if (current_tree_item == nullptr)
  {
    current_tree_item = (MonitorTreeItem*)model->root();
  }
  else
  {
    // we need to be able to get the childe by it's name
    current_tree_item = current_tree_item->getChild(info_.id);
  }

  current_tree_item->clearValue();
  current_tree_item->setMetaData({ QString::fromStdString(info_.field_name) ,  QString::fromStdString(info_.type), "" });
  current_tree_item->markChildrenUnused();
}

void ProtobufTreeBuilder::MessageEnd()
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
