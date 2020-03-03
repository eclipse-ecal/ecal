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

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /*_MSC_VER*/
#include <capnp/dynamic.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif /*_MSC_VER*/
#include "monitor_tree_model.h"

class MonitorTreeBuilder
{
public:
  MonitorTreeBuilder(MonitorTreeModel* model_) : model(model_), current_tree_item(nullptr) {};

  struct MessageInfo
  {
    QString field_name;
    QString group_name;
    size_t id;
    QString type;
  };

  void MessageStart(const MessageInfo& /*info_*/);

  void ArrayStart(const MessageInfo& /*info_*/, size_t /*size_*/);
  void ArrayValue(const MessageInfo& /*info_*/, const QVariant& /*name_*/);
  void ArrayEnd();

  void ScalarValue(const MessageInfo& /*info_*/, const QVariant& value_);
  void MessageEnd();

  MonitorTreeModel* model;
  MonitorTreeItem* current_tree_item;
};

class CapnprotoIterator
{
public:
  CapnprotoIterator(MonitorTreeModel* model_) : tree_builder(model_) {}
  void ProcessMessage(capnp::DynamicStruct::Reader message, const MonitorTreeBuilder::MessageInfo& parent_info = {});

private:
  MonitorTreeBuilder tree_builder;

};
