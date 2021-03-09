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

#include "CustomQt/QAbstractTreeItem.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/topic.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class TopicTreeItem :
  public QAbstractTreeItem
{
public:

  enum class Columns : int
  {
    RCLOCK,
    HNAME,
    PID,
    PNAME,
    UNAME,
    TID,
    TNAME,
    DIRECTION,
    TTYPE,
    TDESC,
    TQOS,
    TLAYER,
    TSIZE,
    CONNECTIONS_LOC,
    CONNECTIONS_EXT,
    MESSAGE_DROPS,
    DCLOCK,
    DFREQ,
  };

  TopicTreeItem();
  TopicTreeItem(const eCAL::pb::Topic& topic);

  ~TopicTreeItem();

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  int type() const;

  void update(const eCAL::pb::Topic& topic);

  eCAL::pb::Topic topicPb();

  std::string topicId() const;

private:
  eCAL::pb::Topic topic_;

  static QString toFrequencyString(long long freq);
};

