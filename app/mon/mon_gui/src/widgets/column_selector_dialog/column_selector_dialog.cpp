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

#include "column_selector_dialog.h"

#include <QPushButton>
#include <QTimer>

#include "widgets/models/tree_item_type.h"

ColumnSelectorDialog::ColumnSelectorDialog(const std::map<int, QString>& column_list, QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f)
  , available_columns_(column_list)
{
  ui_.setupUi(this);

  ui_.autoexpand_spinbox->setSpecialValueText("None");

  // We want the buttons to be square, so we measure one button and resize the
  // button widget. We have to do that _after_ this object has been constructed,
  // therefore the QTimer.
  QTimer::singleShot(1, 
      [this]()
      {
        int button_height = ui_.add_button->size().height();
        ui_.button_widget->setMaximumWidth(button_height);
        ui_.add_button->setMaximumWidth(button_height);
        ui_.remove_button->setMaximumWidth(button_height);
      });

  selected_columns_model_  = new ColumnTreeModel(this);
  ui_.column_treeview->setModel(selected_columns_model_);
  ui_.column_treeview->resizeColumnToContents(0);

  // Force all columns
  QVector<int> forced_columns;
  for (int i = 0; i < selected_columns_model_->columnCount(); i++)
  {
    forced_columns.push_back(i);
  }
  ui_.column_treeview->setForcedColumns(forced_columns);

  // Create a delegate for drawing the Comboboxes
  column_picker_delegate_ = new ColumnPickerDelegate(available_columns_, this);
  ui_.column_treeview->setItemDelegateForColumn((int)ColumnTreeModel::Columns::COLUMN_NAME, column_picker_delegate_);

  connect(ui_.add_button,       &QPushButton::clicked, [this]() {addColumn();              });
  connect(ui_.remove_button,    &QPushButton::clicked, [this]() {removeSelectedColumn();   });
  connect(ui_.clear_button,     &QPushButton::clicked, [this]() {removeAllColumns();       });

  connect(ui_.move_up_button,   &QPushButton::clicked, [this]() {moveUpSelectedColumn();   });
  connect(ui_.move_down_button, &QPushButton::clicked, [this]() {moveDownSelectedColumn(); });

  connect(ui_.column_treeview,  &QAdvancedTreeView::keySequenceDeletePressed, [this]() {removeSelectedColumn(); });

  //TODO: Also implement Drag&Drop
}

ColumnSelectorDialog::~ColumnSelectorDialog()
{
}

void ColumnSelectorDialog::setSelectedColumns(const QVector<int>& columns)
{
  selected_columns_model_->removeAllChildren();

  for (int column_number : columns)
  {
    QString column_name;
    for (auto& column_pair : available_columns_)
    {
      if (column_pair.first == column_number)
      {
        column_name = column_pair.second;
      }
    }

    ColumnTreeItem* column_tree_item = new ColumnTreeItem(column_name, column_number);
    selected_columns_model_->insertItem(column_tree_item);
  }
}

QVector<int> ColumnSelectorDialog::getSelectedColumns() const
{
  QVector<int> selected_columns;
  for (int row = 0; row < selected_columns_model_->rowCount(); row++)
  {
    QAbstractTreeItem* tree_item = (QAbstractTreeItem*) selected_columns_model_->index(row, 0).internalPointer();
    Q_ASSERT(tree_item->type() == (int)TreeItemType::Column);
    int column_number = ((ColumnTreeItem*)tree_item)->number();

    if (column_number >= 0)
    {
      selected_columns.push_back(column_number);
    }
  }

  return selected_columns;
}

void ColumnSelectorDialog::setAutoExpandDepth(int depth)
{
  ui_.autoexpand_spinbox->setValue(depth + 1);
}

int ColumnSelectorDialog::getAutoExpandDepth() const
{
  return ui_.autoexpand_spinbox->value() - 1;
}

void ColumnSelectorDialog::addColumn()
{
  ColumnTreeItem* column_tree_item = new ColumnTreeItem();
  selected_columns_model_->insertItem(column_tree_item);
  ui_.column_treeview->scrollToBottom();
}

void ColumnSelectorDialog::removeSelectedColumn()
{
  auto selected_indices = ui_.column_treeview->selectionModel()->selectedRows();
  if (selected_indices.size() > 0)
  {
    for (auto& index : selected_indices)
    {
      if (index.internalPointer())
      {
        selected_columns_model_->removeItem((QAbstractTreeItem*)(index.internalPointer()));
      }
    }
  }
}

void ColumnSelectorDialog::removeAllColumns()
{
  selected_columns_model_->removeAllChildren();
}

void ColumnSelectorDialog::moveUpSelectedColumn()
{
  auto selected_indices = ui_.column_treeview->selectionModel()->selectedRows();
  if (selected_indices.size() > 0)
  {
    QModelIndex index = selected_indices[0];
    if (index.internalPointer() && (index.row() >= 1))
    {
      selected_columns_model_->moveItem(index.row(), index.row() - 1);
    }
  }
}

void ColumnSelectorDialog::moveDownSelectedColumn()
{
  auto selected_indices = ui_.column_treeview->selectionModel()->selectedRows();
  if (selected_indices.size() > 0)
  {
    QModelIndex index = selected_indices[0];
    if (index.internalPointer() && (index.row() < (selected_columns_model_->rowCount() - 1)))
    {
      selected_columns_model_->moveItem(index.row(), index.row() + 1);
    }
  }
}
