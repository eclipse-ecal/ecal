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

#pragma once

#include <google/protobuf/message.h>
#include "monitor_tree_model.h"

#include <ecal/protobuf/ecal_proto_visitor.h>
#include <stack>

class ProtobufTreeBuilder : public eCAL::protobuf::MessageVisitor
{
public:
  ProtobufTreeBuilder(MonitorTreeModel* model_) : model(model_), current_tree_item(nullptr) {};

protected:
  virtual void ArrayStart(const eCAL::protobuf::MessageInfo& /*info_*/, const google::protobuf::FieldDescriptor::Type& /*type_*/, size_t /*size_*/) override;
  virtual void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, google::protobuf::int32  /*value_*/) override;
  virtual void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, google::protobuf::uint32 /*value_*/) override;
  virtual void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, google::protobuf::int64  /*value_*/) override;
  virtual void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, google::protobuf::uint64 /*value_*/) override;
  virtual void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, float                    /*value_*/) override;
  virtual void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, double                   /*value_*/) override;
  virtual void ArrayValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, bool                     /*value_*/) override;
  virtual void ArrayValueString(const eCAL::protobuf::MessageInfo& /*info_*/, const std::string&              /*value_*/) override;
  virtual void ArrayValueBytes(const eCAL::protobuf::MessageInfo& /*info_*/, const std::string&              /*value_*/) override;
  virtual void ArrayValueEnum(const eCAL::protobuf::MessageInfo& /*info_*/, int                      /*value_*/, const std::string& /*name_*/) override;
  virtual void ArrayEnd() override;

  virtual void ScalarValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, google::protobuf::int32  /*value_*/) override;
  virtual void ScalarValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, google::protobuf::uint32 /*value_*/) override;
  virtual void ScalarValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, google::protobuf::int64  /*value_*/) override;
  virtual void ScalarValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, google::protobuf::uint64 /*value_*/) override;
  virtual void ScalarValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, float                    /*value_*/) override;
  virtual void ScalarValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, double                   /*value_*/) override;
  virtual void ScalarValueIntegral(const eCAL::protobuf::MessageInfo& /*info_*/, bool                     /*value_*/) override;
  virtual void ScalarValueString(const eCAL::protobuf::MessageInfo& /*info_*/,   const std::string&              /*value_*/) override;
  virtual void ScalarValueBytes(const eCAL::protobuf::MessageInfo& /*info_*/, const std::string&              /*value_*/) override;
  virtual void ScalarValueEnum(const eCAL::protobuf::MessageInfo& /*info_*/, int                      /*value_*/, const std::string& /*name*/) override;

  virtual void ScalarMessageStart(const eCAL::protobuf::MessageInfo& /*info_*/, const std::vector<std::string>& /*fields_*/) override;
  virtual void ScalarMessageEnd() override;
  virtual void ArrayMessageStart(const eCAL::protobuf::MessageInfo& /*info_*/, const std::vector<std::string>& /*fields_*/) override;
  virtual void ArrayMessageEnd() override;

private:

  void ArrayValue(const eCAL::protobuf::MessageInfo& /*info_*/, const QVariant& value_);
  void ScalarValue(const eCAL::protobuf::MessageInfo& /*info_*/, const QVariant& value_);
  void MessageStart(const eCAL::protobuf::MessageInfo& /*info_*/);
  void MessageEnd();

  MonitorTreeModel* model;
  MonitorTreeItem* current_tree_item;
};