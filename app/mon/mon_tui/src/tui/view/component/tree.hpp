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

#include <unordered_map>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>

#include "tui/view/component/decorator.hpp"

namespace ftxui
{

struct TreeNode
{
  friend class TreeBase;

  ftxui::Component value;
  std::vector<TreeNode> children;

  std::string tag = "";
  bool expanded = true;

  bool IsExpandable() const
  {
    return children.size() > 0;
  }

private:
  int selector_ = 0;
};

using TreeNodePtr = std::shared_ptr<TreeNode>;

class TreeBase : public ftxui::ComponentBase
{
  Component Expandable(Component label, Components children, bool &show, int* selector) {
   class Impl : public ComponentBase
   {
   public:
     Impl(Component label, Components children, bool &show, int *selector)
         : show_(show)
     {
       CheckboxOption opt;
       opt.transform = [label](EntryState s)
       {
         auto prefix = text(s.state ? "▾ " : "▸ ");
         auto text = label->Render();
         if (s.focused)
           text |= inverted;
         return hbox({prefix, text});
       };
       Components children_ { Checkbox("", &show_, opt) };
       for(auto child: children)
       {
         children_.push_back(Maybe(Indent(std::move(child)), &show_));
       }
       Add(Container::Vertical(children_, selector));
     }

     bool &show_;
   };

   return Make<Impl>(label, std::move(children), show, selector);
  }

  TreeNodePtr root;
  Component tree = Renderer([]{
    return emptyElement();
  });

  Component BuildTree(TreeNode &node)
  {
    Components children;

    auto tree_value = node.value != nullptr
      ? node.value
      : Renderer([] { return emptyElement(); });

    if(node.IsExpandable())
    {
      for(auto &child: node.children)
      {
        auto tree_item = BuildTree(child);
        children.push_back(tree_item);
      }
      return Expandable(tree_value, std::move(children), node.expanded, &node.selector_);
    }
    return tree_value;
  }

  void SaveTreeState(TreeNode &node, std::unordered_map<std::string, TreeNode*> &set)
  {
    if(node.tag != "")
    {
      set[node.tag] = &node;
    }
    for(auto &child: node.children)
    {
      SaveTreeState(child, set);
    }
  }

  void RestoreTreeState(TreeNode &node, std::function<void(TreeNode&)> restore_func)
  {
    restore_func(node);
    for(auto &child: node.children)
    {
      RestoreTreeState(child, restore_func);
    }
  }

protected:
  bool OnEvent(Event event) override
  {
    //Tree doesn't need wraparound functionality
    if(event == Event::Tab) return false;

    return ComponentBase::OnEvent(event);
  }

public:
  TreeBase() : TreeBase(nullptr)
  { }

  TreeBase(TreeNodePtr root_) : root{root_}
  {
    if(root_) tree = BuildTree(*root_);
    Add(tree);
  }

  void UpdateTree(TreeNodePtr root_)
  {
    tree->Detach();
    if(root)
    {
      std::unordered_map<std::string, TreeNode*> state;
      SaveTreeState(*root, state);

      auto restore_state_func = [&state](TreeNode &node) {
        auto entry = state.find(node.tag);
        if(entry != state.end())
        {
          auto old = entry->second;
          node.expanded = old->expanded;
          node.selector_ = old->selector_;
        }
      };
      RestoreTreeState(*root_, restore_state_func);
      tree = BuildTree(*root_);
    }
    root = root_;
    Add(tree);
  }

  Element Render() override
  {
    return tree->Render();
  }
};

using Tree = std::shared_ptr<TreeBase>;

Tree CreateTree(TreeNodePtr root)
{
  return std::make_shared<TreeBase>(root);
}

Tree CreateTree()
{
  return std::make_shared<TreeBase>();
}

}
