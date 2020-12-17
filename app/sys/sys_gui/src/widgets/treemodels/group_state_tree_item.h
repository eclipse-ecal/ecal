/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "CustomQt/QAbstractTreeItem.h"

#include <memory>
#include <ecalsys/ecal_sys.h>

class GroupStateTreeItem :
  public QAbstractTreeItem
{
public:

  enum class Columns : int
  {
    NAME,
  };

  GroupStateTreeItem(std::shared_ptr<TaskGroup::GroupState> group_state);
  ~GroupStateTreeItem();

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  std::shared_ptr<TaskGroup::GroupState> getGroupState();

  int type() const;

private:
  std::shared_ptr<TaskGroup::GroupState> group_state_;
};

