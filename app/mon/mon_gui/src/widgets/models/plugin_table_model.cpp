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

#include "plugin_table_model.h"

#include <QMessageBox>

PluginTableModel::PluginTableModel(QObject *parent) :
  QAbstractItemModel(parent)
{}

int PluginTableModel::rowCount(const QModelIndex &parent) const// override
{
  if (parent.isValid())
  {
    return 0;
  }

  return PluginManager::getInstance()->getAvailableIIDs().count();
}

int PluginTableModel::columnCount(const QModelIndex &parent) const// override
{
  if (parent.isValid())
  {
    return 0;
  }
  return static_cast<int>(Columns::COLUMN_COUNT);
}

static QString GetTopicDisplayString(const PluginWrapper::PluginData::MetaData::Topic& topic)
{
  auto name = (!topic.name.isEmpty()) ? topic.name : QString("*");
  auto type = (!topic.type.isEmpty()) ? topic.type : QString("*");
  auto format = (!topic.format.isEmpty()) ? topic.format : QString("*");

  return name + "(" + format + ":" + type + ")";
}

QVariant PluginTableModel::data(const QModelIndex &index, int role) const// override
{
  if (index.isValid())
  {
    auto iid = PluginManager::getInstance()->getAvailableIIDs().at(index.row());
    PluginWrapper::PluginData plugin_data = PluginManager::getInstance()->getPluginData(iid);

    Columns column = static_cast<Columns>(index.column());

    if (role == Qt::ItemDataRole::DisplayRole)
    {
      switch (column)
      {
      case Columns::NAME:
        return plugin_data.meta_data.name;
      case Columns::VERSION:
        return plugin_data.meta_data.version;
      case Columns::AUTHOR:
        return plugin_data.meta_data.author;
      case Columns::IID:
        return plugin_data.iid;
      case Columns::PRIORITY:
        return plugin_data.meta_data.priority;
      case Columns::PATH:
        return plugin_data.path;
      case Columns::TOPICS:
      {
        QString topics_str;
        for (const auto& topic : plugin_data.meta_data.topics)
          topics_str += "\"" + GetTopicDisplayString(topic) + "\"  ";
        return topics_str;
      }
      default:
        return QVariant();
      }
    }
    if (role == Qt::ItemDataRole::CheckStateRole)
    {
      if (column == Columns::NAME)
        return PluginManager::getInstance()->isActive(iid) ? Qt::Checked : Qt::Unchecked;
    }
  }

  return QVariant();
}

bool PluginTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (index.isValid())
  {
    auto iid = PluginManager::getInstance()->getAvailableIIDs().at(index.row());

    Columns column = static_cast<Columns>(index.column());

    if (role == Qt::CheckStateRole)
    {
      if (column == Columns::NAME)
      {
        if (value == Qt::Checked)
        {
          PluginManager::getInstance()->setActive(iid, true);
        }
        else
        {
          PluginManager::getInstance()->setActive(iid, false);
        }
        return true;
      }
    }
  }
  return false;
}

Qt::ItemFlags PluginTableModel::flags(const QModelIndex & index) const
{
  if (index.isValid())
  {
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemFlag::ItemIsUserCheckable;
  }
  return QAbstractItemModel::flags(index);
}

QModelIndex PluginTableModel::index(int row, int column, const QModelIndex & /*parent*/) const
{
  return createIndex(row, column, nullptr);
}

QVariant PluginTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Orientation::Horizontal) && (role == Qt::ItemDataRole::DisplayRole))
  {
    return header_data[(Columns)section];
  }
  return QVariant();
}

QModelIndex PluginTableModel::parent(const QModelIndex & /*index*/) const
{
  return QModelIndex();
}

void PluginTableModel::rediscover()
{
  beginResetModel();
  PluginManager::getInstance()->discover();
  endResetModel();
}
