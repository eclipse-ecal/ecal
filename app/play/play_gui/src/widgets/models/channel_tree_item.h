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

class ChannelTreeItem : public QAbstractTreeItem
{
public:

  enum class Columns : int
  {
    ENABLED,

    SOURCE_CHANNEL_NAME,
    TARGET_CHANNEL_NAME,

    EXPECTED_FRAMES,
    EXISTING_FRAMES,
    LOST_FRAMES,
    RELATIVE_LOSS
  };

  ChannelTreeItem(const QString& source_name);
  ChannelTreeItem(const QString& source_name, long long expected_frames, long long existing_frames);

  ~ChannelTreeItem();

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const override;

  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  bool setData(int column, const QVariant& data, Qt::ItemDataRole role = Qt::ItemDataRole::EditRole) override;

  Qt::ItemFlags flags(int column) const override;

  int type() const override;

  // Getter
  bool enabled() const;
  QString sourceChannelName() const;
  QString targetChannelName() const;

  // Setter
  void setEnabled(bool enabled);
  void setTargetChannelName(const QString& target_channel_name);

private:
  bool enabled_;

  QString source_name_;
  QString target_name_;

  long long expected_frames_;
  long long existing_frames_;
  
  bool isContinuityValid() const;
};

