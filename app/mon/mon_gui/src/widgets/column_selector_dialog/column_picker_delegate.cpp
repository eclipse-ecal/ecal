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

#include "column_picker_delegate.h"

#include <QComboBox>
#include <QStandardItemModel>

ColumnPickerDelegate::ColumnPickerDelegate(const std::map<int, QString>& column_map, QObject *parent)
  : QStyledItemDelegate(parent)
  , column_map_(column_map)
{
}

QWidget *ColumnPickerDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex& /*index*/) const
{
  QComboBox *editor = new QComboBox(parent);

  QStandardItemModel* item_model = new QStandardItemModel(editor);

  for (auto& column_pair : column_map_)
  {
    // an std::map is always ordered by key, so we are guaranteed to add the items in the correct order
    QStandardItem* item = new QStandardItem();
    item->setData(column_pair.second, Qt::ItemDataRole::DisplayRole);
    item->setData(column_pair.first,  Qt::ItemDataRole::UserRole);

    item_model->appendRow(item);
  }

  editor->setModel(item_model);

  return editor;
}

void ColumnPickerDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  QComboBox *combobox = static_cast<QComboBox*>(editor);

  int current_column_index = index.model()->data(index, Qt::EditRole).toInt();

  if (current_column_index < 0)
  {
    combobox->setCurrentIndex(-1);
    return;
  }

  QStandardItemModel* item_model = static_cast<QStandardItemModel*>(combobox->model());
  for (int row = 0; row < item_model->rowCount(); row++)
  {
    if (item_model->data(item_model->index(row, 0), Qt::ItemDataRole::UserRole).toInt() == current_column_index)
    {
      combobox->setCurrentIndex(row);
      break;
    }
  }
}

void ColumnPickerDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  QComboBox *combobox = static_cast<QComboBox*>(editor);

  QModelIndex selected_index = combobox->model()->index(combobox->currentIndex(), 0);

  if (selected_index.isValid())
  {
    int selected_column_number = combobox->model()->data(selected_index, Qt::ItemDataRole::UserRole).toInt();
    if (selected_column_number >= 0)
    {
      model->setData(index, selected_column_number, Qt::ItemDataRole::EditRole);
      model->setData(index, column_map_.at(selected_column_number), Qt::ItemDataRole::DisplayRole);
    }
  }
}

void ColumnPickerDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}