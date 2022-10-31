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

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

#include "tui/view/component/scroller.hpp"

namespace ftxui
{

Element indent(Element child)
{
  return hbox({
      text("  "),
      child
  });
}

Element indentRight(Element child)
{
  return hbox({
      child,
      text("  ")
  });
}

class IndentBase : public ComponentBase
{
  Component child;
public:
  IndentBase(Component child_) : child{child_}
  {
    Add(child);
  }

  Element Render() override
  {
    return hbox({
      text("  "),
      child->Render()
    });
  }
};

Component Indent(Component child)
{
  return std::make_shared<IndentBase>(child);
}

}
