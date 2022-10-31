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

#include <stack>

#include <ecal/ecal.h>
#include <ecal/protobuf/ecal_proto_visitor.h>
#include <ecal/protobuf/ecal_proto_decoder.h>

#include <google/protobuf/message.h>

#include "tui/view/component/tree.hpp"
#include "tui/view/component/bytes.hpp"
#include "tui/view/component/decorator.hpp"
#include "tui/style_sheet.hpp"

namespace eCAL::protobuf
{

using namespace ftxui;

class TreeMessageVisitor : public MessageVisitor
{
  const std::shared_ptr<StyleSheet> style;

  std::stack<TreeNode*> nodes;

  std::string GenerateTag(const MessageInfo &info)
  {
    return info.group_name + "." + info.field_name + "@" + std::to_string(info.id);
  }

  void ScalarValue(const MessageInfo &info, const std::string &val)
  {
    auto node = nodes.top();
    node->value = Renderer([name = info.field_name, type = info.type, val, style=style]{
      return hbox(
        text(name + " " + type + ": ") | style->proto_tree.primitive_header,
        text(" " + val) | style->proto_tree.primitive
      );
    });
    node->tag = GenerateTag(info);
    nodes.pop();
  }

  void ArrayValue(const MessageInfo& info, const std::string &val)
  {
    auto node = nodes.top();
    node->value = Renderer([index = info.id, val, style=style]{
      return hbox(
        text("[" + std::to_string(index) + "]: ") | style->proto_tree.array_value,
        text(val) | style->proto_tree.primitive
      );
    });
    node->tag = GenerateTag(info);
    nodes.pop();
  }

  void InitializeChildren(TreeNode *node, size_t size)
  {
    auto &children = node->children;
    children.resize(size);
    std::for_each(node->children.rbegin(), node->children.rend(),
    [&nodes = nodes](auto &child) {
      nodes.push(&child);
    });
  }

protected:
  void ArrayStart(const MessageInfo& info, const google::protobuf::FieldDescriptor::Type& /* type_ */, size_t size)
  {
    auto node = nodes.top();
    node->value = Renderer([info, size, style=style]{
      return text(info.field_name + " " + info.type +"[" + std::to_string(size) + "]:")
        | style->proto_tree.array_header;
    });
    node->tag = GenerateTag(info);
    node->expanded = false;
    InitializeChildren(node, size);
  }

  void ArrayValueIntegral(const MessageInfo& info_, google::protobuf::int32 value_)
  {
    ArrayValue(info_, std::to_string(value_));
  }

  void ArrayValueIntegral(const MessageInfo& info_, google::protobuf::uint32 value_)
  {
    ArrayValue(info_, std::to_string(value_));
  }

  void ArrayValueIntegral(const MessageInfo& info_, google::protobuf::int64 value_)
  {
    ArrayValue(info_, std::to_string(value_));
  }

  void ArrayValueIntegral(const MessageInfo& info_, google::protobuf::uint64 value_)
  {
    ArrayValue(info_, std::to_string(value_));
  }

  void ArrayValueIntegral(const MessageInfo& info_, float value_)
  {
    ArrayValue(info_, std::to_string(value_));
  }

  void ArrayValueIntegral(const MessageInfo& info_, double value_)
  {
    ArrayValue(info_, std::to_string(value_));
  }

  void ArrayValueIntegral(const MessageInfo& info_, bool value_)
  {
    ArrayValue(info_, std::to_string(value_));
  }

  void ArrayValueString(const MessageInfo& info_, const std::string& value_)
  {
    ArrayValue(info_, value_);
  }

  void ArrayValueBytes(const MessageInfo& info_, const std::string& value_)
  {
    ArrayValue(info_, value_);
  }

  void ArrayValueEnum(const MessageInfo& info_, int value_, const std::string& name_)
  {
    ArrayValue(info_, name_ + "(" + std::to_string(value_) + ")");
  }

  void ArrayEnd()
  {
    nodes.pop();
  };

  void ScalarValueIntegral(const MessageInfo& info_, google::protobuf::int32 value_)
  {
    ScalarValue(info_, std::to_string(value_));
  }

  void ScalarValueIntegral(const MessageInfo& info_, google::protobuf::uint32 value_)
  {
    ScalarValue(info_, std::to_string(value_));
  }

  void ScalarValueIntegral(const MessageInfo& info_, google::protobuf::int64 value_)
  {
    ScalarValue(info_, std::to_string(value_));
  }

  void ScalarValueIntegral(const MessageInfo& info_, google::protobuf::uint64 value_)
  {
    ScalarValue(info_, std::to_string(value_));
  }

  void ScalarValueIntegral(const MessageInfo& info_, float value_)
  {
    ScalarValue(info_, std::to_string(value_));
  }

  void ScalarValueIntegral(const MessageInfo& info_, double value_)
  {
    ScalarValue(info_, std::to_string(value_));
  }

  void ScalarValueIntegral(const MessageInfo& info_, bool value_)
  {
    ScalarValue(info_, std::to_string(value_));
  }

  void ScalarValueString(const MessageInfo& info_, const std::string& value_)
  {
    ScalarValue(info_, value_);
  }

  void ScalarValueBytes(const MessageInfo& info, const std::string& val)
  {
    auto node = nodes.top();
    node->value = Renderer([name = info.field_name, type = info.type, val, style=style]{
      return text(name + " bytes[" + std::to_string(val.size()) + "]: ") | style->proto_tree.array_header;
    });
    node->tag = GenerateTag(info);
    node->expanded = false;

    auto &child = node->children.emplace_back();
    child.value = Renderer([val, style=style] {
      return bytes(val) | style->proto_tree.primitive;
    });
    child.tag = "data@" + node->tag;
    nodes.pop();
  }

  void ScalarValueEnum(const MessageInfo& info_, int value_, const std::string& name)
  {
    ScalarValue(info_, name + "(" + std::to_string(value_) + ")");
  }

  void ScalarMessageStart(const MessageInfo& info, const std::vector<std::string>& fields_)
  {
    auto node = nodes.top();
    node->value = Renderer([info, style=style]{
      auto name = info.field_name != "" ? info.field_name + " " : "";
      return text(name + info.type + ":") | style->proto_tree.msg_header;
    });
    node->tag = GenerateTag(info);
    node->expanded = false;
    InitializeChildren(node, fields_.size());
  }

  void ScalarMessageEnd()
  {
    nodes.pop();
  }

  void ArrayMessageStart(const MessageInfo& info, const std::vector<std::string>& fields_)
  {
    auto node = nodes.top();

    node->value = Renderer([info, style=style]{
      auto name = info.field_name != "" ? info.field_name + " " : "";
      return text(info.type + "[" + std::to_string(info.id) + "]:") | style->proto_tree.array_value;
    });
    node->tag = GenerateTag(info);
    node->expanded = false;

    InitializeChildren(node, fields_.size());
  }

  void ArrayMessageEnd()
  {
    nodes.pop();
  }

  bool AcceptMessage(const std::string& /*name*/) { return true; };

public:
  TreeMessageVisitor(TreeNode &root, const std::shared_ptr<StyleSheet> style_)
    : style{style_}
  {
    nodes.push(&root);
  }
};

}

void PopulateProtoTree(ftxui::TreeNode &root, google::protobuf::Message *message, const std::shared_ptr<StyleSheet> style)
{
  auto tree_builder = std::make_shared<eCAL::protobuf::TreeMessageVisitor>(root, style);
  if(message)
  {
    eCAL::protobuf::CProtoDecoder protobuf_decoder;
    protobuf_decoder.SetVisitor(tree_builder);
    protobuf_decoder.ProcProtoMsg(*message);
  }
}

ftxui::TreeNodePtr ProtoTree(google::protobuf::Message *message, const std::shared_ptr<StyleSheet> style)
{
  using namespace ftxui;
  auto root = std::make_shared<TreeNode>();
  PopulateProtoTree(*root, message, style);
  return root;
}
