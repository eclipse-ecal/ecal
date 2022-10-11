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

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>

namespace ftxui {
class ContainerBase : public ComponentBase {
 public:
  ContainerBase(Components children, int* selector)
      : selector_(selector ? selector : &selected_) {
    for (Component& child : children)
      Add(std::move(child));
  }

  // Component override.
  bool OnEvent(Event event) override {
    if (event.is_mouse())
      return OnMouseEvent(event);

    if (!Focused())
      return false;

    if (ActiveChild() && ActiveChild()->OnEvent(event))
      return true;

    return EventHandler(event);
  }

  Component ActiveChild() override {
    if (children_.size() == 0)
      return nullptr;

    return children_[*selector_ % children_.size()];
  }

  void SetActiveChild(ComponentBase* child) override {
    for (size_t i = 0; i < children_.size(); ++i) {
      if (children_[i].get() == child) {
        *selector_ = static_cast<int>(i);
        return;
      }
    }
  }

 protected:
  // Handlers
  virtual bool EventHandler(Event) { return false; }

  virtual bool OnMouseEvent(Event event) {
    return ComponentBase::OnEvent(event);
  }

  int selected_ = 0;
  int* selector_ = nullptr;

  void MoveSelector(int dir) {
    for (int i = *selector_ + dir; i >= 0 && i < (int)children_.size();
         i += dir) {
      if (children_[i]->Focusable()) {
        *selector_ = i;
        return;
      }
    }
  }
  void MoveSelectorWrap(int dir) {
    for (size_t offset = 1; offset < children_.size(); ++offset) {
      size_t i = (*selector_ + offset * dir + children_.size()) % children_.size();
      if (children_[i]->Focusable()) {
        *selector_ = static_cast<int>(i);
        return;
      }
    }
  }
};
}

class FocusManager : public ftxui::ContainerBase
{
 public:
  using ftxui::ContainerBase::ContainerBase;

  ftxui::Element Render() override {
    using namespace ftxui;
    Elements elements;
    for (auto& it : children_)
      elements.push_back(it->Render());
    if (elements.size() == 0)
      return text("Empty container");
    return hbox(std::move(elements));
  }

  bool EventHandler(ftxui::Event event) override {
    using namespace ftxui;
    int old_selected = *selector_;
    if (event == Event::Tab && children_.size())
      MoveSelectorWrap(+1);
    if (event == Event::TabReverse && children_.size())
      MoveSelectorWrap(-1);

    *selector_ = std::max(0, std::min(int(children_.size()) - 1, *selector_));
    return old_selected != *selector_;
  }
};
