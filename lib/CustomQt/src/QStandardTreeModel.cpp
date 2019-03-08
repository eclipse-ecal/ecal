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

#include <CustomQt/QStandardTreeModel.h>

QStandardTreeModel::QStandardTreeModel(QObject *parent)
  : QAbstractTreeModel(parent)
  , column_count_(1)
{}

QStandardTreeModel::~QStandardTreeModel()
{}

int QStandardTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
  return column_count_;
}

void QStandardTreeModel::setColumnCount(int column_count)
{
  column_count_ = column_count;
}

QVariant QStandardTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Orientation::Vertical)
    && header_data_vertical_.contains(section)
    && header_data_vertical_.value(section).contains(role))
  {
    return header_data_vertical_.value(section).value(role);
  }
  else if ((orientation == Qt::Orientation::Horizontal)
    && header_data_horizontal_.contains(section)
    && header_data_horizontal_.value(section).contains(role))
  {
    return header_data_horizontal_.value(section).value(role);
  }

  // Default Fallback
  else if (role == Qt::ItemDataRole::DisplayRole)
  {
    return section;
  }

  return QVariant();
}

bool QStandardTreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
  if (orientation == Qt::Orientation::Horizontal)
  {
    header_data_horizontal_[section][role] = value;
    emit headerDataChanged(orientation, section, section);
    return true;
  }
  else // if (orientation == Qt::Orientation::Vertical)
  {
    header_data_vertical_[section][role] = value;
    emit headerDataChanged(orientation, section, section);
    return true;
  }
}