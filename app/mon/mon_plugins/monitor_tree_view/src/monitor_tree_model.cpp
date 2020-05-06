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

#include "monitor_tree_model.h"
#include "tree_item_type.h"
//#include "ItemDataRoles.h"

MonitorTreeModel::MonitorTreeModel(QObject *parent)
  : QAbstractTreeModel(parent)
  , display_blobs_(false)
{
  setRoot(new MonitorTreeItem(-1, this));
}

MonitorTreeModel::~MonitorTreeModel()
{}

int MonitorTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant MonitorTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    return column_labels_.at((Columns)section);
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}

int MonitorTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::MonitoringContent:
    return item_column_mapping_.at((Columns)(model_column));
  default:
    return QAbstractTreeModel::mapColumnToItem(model_column, tree_item_type);
  }
}

void MonitorTreeModel::setRoot(QAbstractTreeItem* root)
{
  // This function just checks if the user tries to set an inappropriate root item.
  Q_ASSERT(root->type() == (int)TreeItemType::MonitoringContent);
  QAbstractTreeModel::setRoot(root);
}

bool MonitorTreeModel::displayBlobs() const
{
  return display_blobs_;
}

void MonitorTreeModel::setDisplayBlobs(bool enabled)
{
  if (display_blobs_ != enabled)
  {
    display_blobs_ = enabled;
    updateAll();
  }
}