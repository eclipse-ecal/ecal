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

#include <QAbstractItemModel>

#include <EcalParser/EcalParser.h>

#include <vector>
#include <utility>

class FunctionListModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum class Columns : int
  {
    NAME,
    DESCRIPTION,

    COLUMN_COUNT
  };

//////////////////////////////////////////////////////
// Constructor & destructor
//////////////////////////////////////////////////////
public:
  explicit FunctionListModel(QObject *parent = 0);
  ~FunctionListModel();

//////////////////////////////////////////////////////
// Reimplemented from QAbstractItemModel
//////////////////////////////////////////////////////
public:
  QVariant      data       (const QModelIndex &index, int role)                                       const override;
  QVariant      headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole)     const override;
  QModelIndex   index      (int row, int column, const QModelIndex &parent = QModelIndex())           const override;
  QModelIndex   index      (const QString& function_name, int column = 0)                             const;
  QModelIndex   parent     (const QModelIndex &index)                                                 const override;
  int           rowCount   (const QModelIndex &parent = QModelIndex())                                const override;
  int           columnCount(const QModelIndex &parent = QModelIndex())                                const override;


//////////////////////////////////////////////////////
// Auxiliary functions
//////////////////////////////////////////////////////
public:
  std::pair<QString, EcalParser::Function*> getFunction(int row) const;

//////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////
private:
  const std::map<Columns, QString> column_labels_
  {
    { Columns::NAME,                 "Function" } ,
    { Columns::DESCRIPTION,          "Description" } ,
  };

  std::vector<std::pair<QString, EcalParser::Function*>> function_list_;
};
