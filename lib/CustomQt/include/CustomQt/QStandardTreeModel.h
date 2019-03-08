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

#include <CustomQt/QAbstractTreeModel.h>

#include <QMap>

/**
 * @brief The QStandardTreeModel is a default implementation of the @see{QAbstractTreeModel}. This class is ready-to-use.
 *
 * This class provides an implementation for all required functions of the
 * @see{QAbstractTreeModel}. Setting the header is done by the
 * @see{setHeaderData} function.
 * A corresponding Tree Item implementation is provided by the
 * @see{QStandardTreeItem}.
 */
class QStandardTreeModel : public QAbstractTreeModel
{
  Q_OBJECT

public:
  /**
   * @brief Creates a Tree Model having one column
   * @param parent
   */
  QStandardTreeModel(QObject *parent = nullptr);

  ~QStandardTreeModel();

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * @brief Sets how many columns this model has
   * @param column_count The number of columns
   */
  void setColumnCount(int column_count);

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

private:
  int column_count_;                                                            /**< How many columns this model has */

  QMap<int, QMap<int, QVariant>> header_data_horizontal_;                       /**< QMap<Column, QMap<Role, QVariant>>. This map holds all horizontal header data set by the user*/
  QMap<int, QMap<int, QVariant>> header_data_vertical_;                         /**< QMap<Column, QMap<Role, QVariant>>. This map holds all vertical header data set by the user*/
}; 
