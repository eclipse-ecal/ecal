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

#include <algorithm>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <memory>
#include <utility>

#include "ftxui/component/component.hpp"
#include "ftxui/component/mouse.hpp"
#include "ftxui/dom/deprecated.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/node.hpp"
#include "ftxui/dom/requirement.hpp"
#include "ftxui/screen/box.hpp"

namespace ftxui
{
class ScrollerBase : public ComponentBase
{
 public:
  ScrollerBase(Component child, int &selected) : selected_{selected} { Add(child); }
  ScrollerBase(Component child) : selected_{selected_dummy} { Add(child); }

 private:
  Element Render() final
  {
    auto focused = Focused() ? focus : ftxui::select;
    auto style = Focused() ? inverted : nothing;

    Element background = ComponentBase::Render();
    background->ComputeRequirement();
    size_ = background->requirement().min_y;
    auto el =  dbox({
               std::move(background),
               vbox({
                   text(L"") | size(HEIGHT, EQUAL, selected_),
                   text(L"") | style | focused,
               }),
           }) |
           vscroll_indicator | yframe | yflex | reflect(box_);
    return el;
  }

  bool OnEvent(Event event) final
  {
    if (event.is_mouse() && box_.Contain(event.mouse().x, event.mouse().y))
      TakeFocus();

    int selected_old = selected_;
    if (event == Event::ArrowUp || event == Event::Character('k') ||
        (event.is_mouse() && event.mouse().button == Mouse::WheelUp))
    {
      selected_--;
    }
    if ((event == Event::ArrowDown || event == Event::Character('j') ||
         (event.is_mouse() && event.mouse().button == Mouse::WheelDown)))
    {
      selected_++;
    }

    if(event.mouse().button == Mouse::Left &&
        event.mouse().motion == Mouse::Pressed &&
        box_.Contain(event.mouse().x, event.mouse().y))
    {
    }

    if (event == Event::PageDown)
      selected_ += box_.y_max - box_.y_min;
    if (event == Event::PageUp)
      selected_ -= box_.y_max - box_.y_min;
    if (event == Event::Home)
      selected_ = 0;
    if (event == Event::End)
      selected_ = size_;

    selected_ = std::max(0, std::min(size_ - 1, selected_));
    return selected_old != selected_;
  }

  bool Focusable() const final { return true; }

  int &selected_;
  int selected_dummy = 0;
  int size_ = 0;
  Box box_;
};

Component Scroller(Component child, int &selected)
{
  return Make<ScrollerBase>(std::move(child), selected);
}

Component Scroller(Component child)
{
  return Make<ScrollerBase>(std::move(child));
}

}
