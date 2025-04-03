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

#include "CustomQt/QAbstractTreeItem.h"
#include <QFont>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/topic.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class TopicTreeItem :
  public QAbstractTreeItem
{
public:

  enum class Columns : int
  {
    REGISTRATION_CLOCK,
    HOST_NAME,
    SHM_TRANSPORT_DOMAIN,
    PROCESS_ID,
    PROCESS_NAME,
    UNIT_NAME,
    TOPIC_ID,
    TOPIC_NAME,
    DIRECTION,
    TENCODING,
    TTYPE,
    TDESC,
    TRANSPORT_LAYER,
    TOPIC_SIZE,
    CONNECTIONS_LOCAL,
    CONNECTIONS_EXTERNAL,
    MESSAGE_DROPS,
    DATA_CLOCK,
    DFREQ,
  };

  TopicTreeItem() = default;
  TopicTreeItem(const eCAL::pb::Topic& topic);

  ~TopicTreeItem() override = default;

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  bool setFont(const QFont& font);

  int type() const;

  void update(const eCAL::pb::Topic& topic);

  eCAL::pb::Topic topicPb();

  std::string topicId() const;

private:
  eCAL::pb::Topic topic_;
  QFont itemfont;
  static QString toFrequencyString(long long freq);
};
