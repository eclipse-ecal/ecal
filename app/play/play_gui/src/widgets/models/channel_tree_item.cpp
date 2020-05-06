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

#include "channel_tree_item.h"

#include <QColor>
#include <QFont>

#include "tree_item_type.h"
#include "item_data_roles.h"

ChannelTreeItem::ChannelTreeItem(const QString& source_name)
  : QAbstractTreeItem()
  , enabled_(true)
  , source_name_(source_name)
  , target_name_(source_name)
  , expected_frames_(-1)
  , existing_frames_(0)
{}

ChannelTreeItem::ChannelTreeItem(const QString& source_name, long long expected_frames, long long existing_frames)
  : QAbstractTreeItem()
  , enabled_(true)
  , source_name_(source_name)
  , target_name_(source_name)
  , expected_frames_(expected_frames)
  , existing_frames_(existing_frames)
{}


ChannelTreeItem::~ChannelTreeItem()
{}

QVariant ChannelTreeItem::data(int column, Qt::ItemDataRole role) const
{
  return data((Columns)column, role);
}

QVariant ChannelTreeItem::data(Columns column, Qt::ItemDataRole role) const
{
  if (role == (Qt::ItemDataRole)ItemDataRoles::RawDataRole) //-V1016 //-V547
  {
    switch (column)
    {
    case ChannelTreeItem::Columns::ENABLED:
      return enabled_;
    case ChannelTreeItem::Columns::SOURCE_CHANNEL_NAME:
      return source_name_;
    case ChannelTreeItem::Columns::TARGET_CHANNEL_NAME:
      return target_name_;
    case ChannelTreeItem::Columns::EXPECTED_FRAMES:
      return expected_frames_;
    case ChannelTreeItem::Columns::EXISTING_FRAMES:
      return existing_frames_;
    case ChannelTreeItem::Columns::LOST_FRAMES:
      return isContinuityValid() ? (expected_frames_ - existing_frames_) : -1;
    case ChannelTreeItem::Columns::RELATIVE_LOSS:
      return (expected_frames_ == 0) ? 0.0 : (double)(expected_frames_ - existing_frames_) / (double)expected_frames_;
    default:
      return QVariant();
    }
  }

  else if (role == Qt::ItemDataRole::DisplayRole)
  {
    if (column == ChannelTreeItem::Columns::ENABLED)
    {
      return QVariant();
    }
    else if ((column == ChannelTreeItem::Columns::EXPECTED_FRAMES)
      || (column == ChannelTreeItem::Columns::LOST_FRAMES))
    {
      if (isContinuityValid())
      {
        return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
      }
      else
      {
        return "Undetectable";
      }
    }
    else if (column == ChannelTreeItem::Columns::RELATIVE_LOSS)
    {
      if (isContinuityValid())
      {
        if ((expected_frames_ - existing_frames_) == 0)
        {
          return "0 %";
        }
        else
        {
          return QString::number(data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toDouble() * 100.0, 'f', 2) + " %"; //-V1016
        }
      }
      else
      {
        return "Undetectable";
      }
    }
    else
    {
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
  }


  else if (role == Qt::ItemDataRole::EditRole)
  {
    if (column == Columns::TARGET_CHANNEL_NAME)
    {
      return target_name_;
    }
  }

  else if (role == Qt::ItemDataRole::CheckStateRole)
  {
    if (column == Columns::ENABLED)
    {
      return enabled_ ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    }
  }

  else if (role == (Qt::ItemDataRole)ItemDataRoles::FilterRole) //-V1016 //-V547
  {
    return data(column, Qt::ItemDataRole::DisplayRole);
  }
  else if (role == (Qt::ItemDataRole)ItemDataRoles::SortRole) //-V1016 //-V547
  {
    switch (column)
    {
    case ChannelTreeItem::Columns::LOST_FRAMES:
      return isContinuityValid() ? data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole) : -1LL; //-V1016
    case ChannelTreeItem::Columns::RELATIVE_LOSS:
      return isContinuityValid() ? data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole) : -1.0; //-V1016
    default:
      return data(column, (Qt::ItemDataRole)ItemDataRoles::RawDataRole); //-V1016
    }
  }

  else if (role == Qt::ItemDataRole::TextAlignmentRole)
  {
    if ((column == ChannelTreeItem::Columns::EXPECTED_FRAMES)
      || (column == ChannelTreeItem::Columns::EXISTING_FRAMES)
      || (column == ChannelTreeItem::Columns::LOST_FRAMES)
      || (column == ChannelTreeItem::Columns::RELATIVE_LOSS))
    {
      return Qt::AlignmentFlag::AlignRight;
    }
    else
    {
      return QVariant();
    }
  }

  else if (role == Qt::ItemDataRole::ForegroundRole)
  {
    if (!isContinuityValid())
    {
      if ((column == ChannelTreeItem::Columns::EXPECTED_FRAMES)
        || (column == ChannelTreeItem::Columns::LOST_FRAMES)
        || (column == ChannelTreeItem::Columns::RELATIVE_LOSS))
      {
        return QColor(127, 127, 127);
      }
    }
    else if (expected_frames_ - existing_frames_ > 0)
    {
      if ((column == ChannelTreeItem::Columns::LOST_FRAMES)
        || (column == ChannelTreeItem::Columns::RELATIVE_LOSS))
      {
        return QColor(180, 25, 25);
      }
    }
    else
    {
      return QVariant();
    }
  }


  return QVariant::Invalid;
}

bool ChannelTreeItem::setData(int column, const QVariant& data, Qt::ItemDataRole role)
{
  if (role == Qt::ItemDataRole::EditRole)
  {
    if (column == (int)Columns::TARGET_CHANNEL_NAME)
    {
      target_name_ = data.toString();
      if (target_name_.isEmpty())
      {
        target_name_ = source_name_;
      }
      return true;
    }
  }
  else if (role == Qt::ItemDataRole::CheckStateRole)
  {
    if (column == (int)Columns::ENABLED)
    {
      enabled_ = data.toBool();
      return true;
    }
  }

  return QAbstractTreeItem::setData(column, data, role);
}

Qt::ItemFlags ChannelTreeItem::flags(int column) const
{
  switch (column)
  {
  case (int)Columns::ENABLED:
    return QAbstractTreeItem::flags(column) | Qt::ItemFlag::ItemIsUserCheckable;
  case (int)Columns::TARGET_CHANNEL_NAME:
    return QAbstractTreeItem::flags(column) | Qt::ItemFlag::ItemIsEditable;
  default:
    return QAbstractTreeItem::flags(column);
  }
}


int ChannelTreeItem::type() const
{
  return (int)TreeItemType::Channel;
}

bool ChannelTreeItem::isContinuityValid() const
{
  return (expected_frames_ >= 0);
}



bool ChannelTreeItem::enabled() const
{
  return enabled_;
}

QString ChannelTreeItem::sourceChannelName() const
{
  return source_name_;
}

QString ChannelTreeItem::targetChannelName() const
{
  return target_name_;
}

void ChannelTreeItem::setEnabled(bool enabled)
{
  enabled_ = enabled;
}

void ChannelTreeItem::setTargetChannelName(const QString& target_channel_name)
{
  target_name_ = target_channel_name;
}
