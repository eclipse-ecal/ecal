/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "severity_picker_delegate.h"

#include <QComboBox>

#include "widgets/treemodels/group_state_min_task_state_tree_model.h"

#include "widgets/severity_model/severity_level_model.h"
#include "widgets/severity_model/severity_model.h"

SeverityPickerDelegate::SeverityPickerDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *SeverityPickerDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex& index) const
{
  QComboBox *editor = new QComboBox(parent);
  editor->setToolTip(tooltip);

  GroupStateMinTaskStateTreeModel::Columns column = (GroupStateMinTaskStateTreeModel::Columns)(index.column());

  switch (column)
  {
  case GroupStateMinTaskStateTreeModel::Columns::SEVERITY:
    editor->setModel(new SeverityModel(editor));
    break;
  case GroupStateMinTaskStateTreeModel::Columns::SEVERITY_LEVEL:
    editor->setModel(new SeverityLevelModel(editor));
    break;
  default:
    break;
  }

  connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SeverityPickerDelegate::saveData);

  return editor;
}

void SeverityPickerDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  QComboBox *combobox = static_cast<QComboBox*>(editor);

  combobox->blockSignals(true);

  int current_int = index.model()->data(index, Qt::EditRole).toInt();

  if (index.column() == (int)GroupStateMinTaskStateTreeModel::Columns::SEVERITY)
  {
    eCAL_Process_eSeverity severity = (eCAL_Process_eSeverity)(current_int);
    SeverityModel* severity_model = static_cast<SeverityModel*>(combobox->model());
    combobox->setCurrentIndex(severity_model->getRow(severity));
  }
  else if (index.column() == (int)GroupStateMinTaskStateTreeModel::Columns::SEVERITY_LEVEL)
  {
    eCAL_Process_eSeverity_Level severity_level = (eCAL_Process_eSeverity_Level)(current_int);
    SeverityLevelModel* severity_level_model = static_cast<SeverityLevelModel*>(combobox->model());
    combobox->setCurrentIndex(severity_level_model->getRow(severity_level));
  }

  combobox->blockSignals(false);
}

void SeverityPickerDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  QComboBox *combobox = static_cast<QComboBox*>(editor);

  if (index.column() == (int)GroupStateMinTaskStateTreeModel::Columns::SEVERITY)
  {
    SeverityModel* severity_model = static_cast<SeverityModel*>(combobox->model());
    eCAL_Process_eSeverity severity = severity_model->getSeverity(combobox->currentIndex());
    model->setData(index, (int)severity, Qt::EditRole);
  }
  else if (index.column() == (int)GroupStateMinTaskStateTreeModel::Columns::SEVERITY_LEVEL)
  {
    SeverityLevelModel* severity_level_model = static_cast<SeverityLevelModel*>(combobox->model());
    eCAL_Process_eSeverity_Level severity_level = severity_level_model->getSeverityLevel(combobox->currentIndex());
    model->setData(index, (int)severity_level, Qt::EditRole);
  }
}

void SeverityPickerDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

void SeverityPickerDelegate::saveData()
{
  QComboBox *combobox = static_cast<QComboBox*>(sender());
  emit commitData(combobox);
}