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

#include "ProtobufTreeModel.h"
#include "TreeItemType.h"
#include "ItemDataRoles.h"

ProtobufTreeModel::ProtobufTreeModel(QObject *parent)
  : QAbstractTreeModel(parent)
  , display_blobs_(false)
{
  setRoot(new ProtobufTreeItem(-1, this));
}

ProtobufTreeModel::~ProtobufTreeModel()
{}

int ProtobufTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant ProtobufTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    return column_labels_.at((Columns)section);
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}

int ProtobufTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch ((TreeItemType)tree_item_type)
  {
  case TreeItemType::Protobuf:
    return item_column_mapping_.at((Columns)(model_column));
  default:
    return QAbstractTreeModel::mapColumnToItem(model_column, tree_item_type);
  }
}

void ProtobufTreeModel::setRoot(QAbstractTreeItem* root)
{
  // This function just checks if the user tries to set an inappropriate root item.
  Q_ASSERT(root->type() == (int)TreeItemType::Protobuf);
  QAbstractTreeModel::setRoot(root);
}

void ProtobufTreeModel::setMessage(const google::protobuf::Message& message)
{
  ((ProtobufTreeItem*)root())->setMessage(message);
}


bool ProtobufTreeModel::displayBlobs() const
{
  return display_blobs_;
}

void ProtobufTreeModel::setDisplayBlobs(bool enabled)
{
  if (display_blobs_ != enabled)
  {
    display_blobs_ = enabled;
    updateAll();
  }
}