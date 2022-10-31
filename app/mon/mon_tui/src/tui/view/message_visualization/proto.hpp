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

#include <memory>

#include "tui/view/component/tree.hpp"
#include "tui/view/message_visualization/messaage_vizualization.hpp"
#include "tui/view/message_visualization/proto_tree.hpp"

#include "tui/viewmodel/message_visualization/proto.hpp"

class ProtoMessageVisualizationView : public MessageVisualizationView
{
  std::shared_ptr<ProtoMessageVisualizationViewModel> view_model;
  ftxui::Tree tree = ftxui::CreateTree();

  ftxui::Component proto_tree = ftxui::Renderer([this, render_timestamp = 0](){
    using namespace ftxui;
    if(view_model->message_timestamp != render_timestamp)
    {
      auto message = view_model->message();
      auto root = ProtoTree(message.message, style);
      tree->UpdateTree(root);
    }
    return tree->Render();
  });

protected:
  virtual ftxui::Component DataView() override
  {
    return proto_tree;
  }

public:
  ProtoMessageVisualizationView(std::shared_ptr<ProtoMessageVisualizationViewModel> vm)
    : MessageVisualizationView(vm), view_model{vm} { }

  void Init() override
  {
    Add(tree);
  }
};
