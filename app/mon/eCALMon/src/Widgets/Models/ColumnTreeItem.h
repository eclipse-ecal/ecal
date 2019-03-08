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

class ColumnTreeItem :
  public QAbstractTreeItem
{
public:
  enum class Columns : int
  {
    INDEX,
    NAME,
  };

  ColumnTreeItem(const QString& name = "", int number = -1);

  ~ColumnTreeItem();

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  bool setData(int column, const QVariant& data, Qt::ItemDataRole role = Qt::ItemDataRole::EditRole);

  int type() const;

  Qt::ItemFlags flags(int column) const;

  void setName(const QString& name);
  QString name() const;
  void setNumber(int number);
  int number() const;

private:
  QString name_;
  int number_;
};
