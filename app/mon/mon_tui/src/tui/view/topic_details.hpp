/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "ftxui/dom/elements.hpp"
#include "utils/string.hpp"

#include "tui/view/view.hpp"
#include "tui/view/component/decorator.hpp"
#include "tui/view/message_visualization/factory.hpp"

#include "tui/viewmodel/topics.hpp"

class TopicDetailsView : public View
{
  std::shared_ptr<TopicsViewModel> view_model;

  std::string prev_topic;
  ftxui::Component vizualization_view = ftxui::Renderer([]{ return ftxui::emptyElement(); });
  ftxui::Component vizualization = ftxui::Renderer([this] {
    return Vizualization()->Render();
  });

  ftxui::Element Info()
  {
    using namespace ftxui;
    auto details = view_model->SelectedItem();
    if(details == nullptr) return emptyElement();

    Elements attributes;
    for(auto &a: details->attributes)
    {
      attributes.push_back(text(a.first + ": " + a.second));
    }

    return vbox (
      separatorEmpty(),
      text("Process path: " + details->process_name),
      separatorEmpty(),
      text("Local connections: " + std::to_string(details->local_connections_count)),
      separatorEmpty(),
      text("External connections: " + std::to_string(details->external_connections_count)),
      separatorEmpty(),
      text("Attributes:"),
      vbox(attributes)
    );
  }

  ftxui::Component Vizualization()
  {
    using namespace ftxui;
    auto topic = view_model->SelectedItem();
    if(topic == nullptr)
    {
      prev_topic = "";
      vizualization_view->Detach();
      vizualization_view = ftxui::Renderer([]{ return ftxui::emptyElement(); });
    }

    const auto topic_name = topic->name;
    const auto tencoding = topic->encoding;
    const auto ttype = topic->type;
    if(topic_name != prev_topic)
    {
      prev_topic = topic_name;
      vizualization_view->Detach();
      vizualization_view = CreateVisualizationView(view_factory, view_model->view_model_factory, topic_name, tencoding, ttype);
      Add(vizualization_view);
    }
    return vizualization_view;
  }

public:
  TopicDetailsView(std::shared_ptr<TopicsViewModel> vm)
    : View(), view_model{vm} {}

  class ViewModel &ViewModel() override
  {
    return *view_model;
  }

  ftxui::Element Render() override
  {
    using namespace ftxui;

    return vbox(
      Info()
        | indent
        | indentRight,
      separatorEmpty(),
      separator(),
      vizualization->Render()
        | indent
        | indentRight
    );
  }
};
