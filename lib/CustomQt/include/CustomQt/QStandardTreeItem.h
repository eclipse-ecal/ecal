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

#include <CustomQt/QAbstractTreeItem.h>

#include <QMap>

/**
 * @brief The QStandardTreeItem is a default implementation of the @see{QAbstractTreeItem}. This class is ready-to-use.
 *
 * This class provides an implementation for all required functions of the
 * @see{QAbstractTreeItem}. Setting data is done by the @see{setData} function.
 * A corresponding Tree Model implementation is provided by the
 * @see{QStandardTreeModel}.
 */
class QStandardTreeItem : public QAbstractTreeItem
{
public:
  QStandardTreeItem();
  ~QStandardTreeItem();

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const override;

  bool setData(int column, const QVariant& data, Qt::ItemDataRole role = Qt::ItemDataRole::EditRole) override;

private:
  QMap<int, QMap<Qt::ItemDataRole, QVariant>> data_;                            /**< QMap<Column, QMap<Role, QVariant>>. This map holds all data set by the user*/
};

