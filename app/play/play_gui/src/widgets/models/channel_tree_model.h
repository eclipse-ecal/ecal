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

#include "CustomQt/QAbstractTreeModel.h"

#include "channel_tree_item.h"

class ChannelTreeModel : public QAbstractTreeModel
{
  Q_OBJECT

public:

  enum class Columns : int
  {
    ENABLED,
    CHANNEL_NAME,

    EXPECTED_FRAMES,
    EXISTING_FRAMES,
    LOST_FRAMES,
    RELATIVE_LOSS,

    MESSAGE_COUNTER,

    COLUMN_COUNT
  };


  ChannelTreeModel(QObject *parent = nullptr);
  ~ChannelTreeModel();

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role) const override;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex &index) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  void reload();

  void updateMessageCounters();

  void setAllChecked(bool checked);
  void invertCheckedState();

  void setEditEnabled(bool enabled);
  bool editEnabled() const;

protected:
  int mapColumnToItem(int model_column, int tree_item_type) const override;

private:
  std::map<Columns, QString> columnLabels =
  { 
    { Columns::ENABLED,         ""} ,
    { Columns::CHANNEL_NAME,    "Channel" } ,
    { Columns::MESSAGE_COUNTER, "Published messages" } ,
    { Columns::EXPECTED_FRAMES, "Expected frames" } ,
    { Columns::EXISTING_FRAMES, "Total frames" } ,
    { Columns::LOST_FRAMES,     "Missing frames" } ,
    { Columns::RELATIVE_LOSS,   "Rel. missing" } ,
  };

  std::map<Columns, int> task_tree_item_column_mapping =
  {
    { Columns::ENABLED,         (int)ChannelTreeItem::Columns::ENABLED },
    { Columns::CHANNEL_NAME,    (int)ChannelTreeItem::Columns::SOURCE_CHANNEL_NAME },
    { Columns::MESSAGE_COUNTER, -1 },
    { Columns::EXPECTED_FRAMES, (int)ChannelTreeItem::Columns::EXPECTED_FRAMES },
    { Columns::EXISTING_FRAMES, (int)ChannelTreeItem::Columns::EXISTING_FRAMES },
    { Columns::LOST_FRAMES,     (int)ChannelTreeItem::Columns::LOST_FRAMES },
    { Columns::RELATIVE_LOSS,   (int)ChannelTreeItem::Columns::RELATIVE_LOSS },

  };

  bool edit_enabled_;

  std::map<std::string, long long> message_counters_;
};

