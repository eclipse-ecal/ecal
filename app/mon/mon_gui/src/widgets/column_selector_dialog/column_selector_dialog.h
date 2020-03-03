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

#include <QDialog>
#include "CustomQt/QStableSortFilterProxyModel.h"
#include "ui_column_selector_dialog.h"

#include "widgets/models/column_tree_model.h"
#include "widgets/column_selector_dialog/column_picker_delegate.h"

#include <map>

class ColumnSelectorDialog : public QDialog
{
  Q_OBJECT

public:
  ColumnSelectorDialog(const std::map<int, QString>& column_list, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
  ~ColumnSelectorDialog();

  void setSelectedColumns(const QVector<int>& columns);

  QVector<int> getSelectedColumns() const;

  void setAutoExpandDepth(int dept);

  int getAutoExpandDepth() const;

private slots:
  void addColumn();
  void removeSelectedColumn();
  void removeAllColumns();
  void moveUpSelectedColumn();
  void moveDownSelectedColumn();

private:
  Ui::ColumnSelectorDialog ui_;

  std::map<int, QString> available_columns_;

  ColumnTreeModel* selected_columns_model_;
  ColumnPickerDelegate* column_picker_delegate_;
};
