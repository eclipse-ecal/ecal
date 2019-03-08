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

#include "CustomQt/QCheckboxHeaderView.h"

#include <QCheckBox>

QCheckboxHeaderView::QCheckboxHeaderView(QWidget* parent)
  :QHeaderView(Qt::Orientation::Horizontal, parent)
{
  setDefaultAlignment(Qt::AlignLeft);
  setSectionsMovable(true);

  // We create a checkbox just to get it's size hint
  QCheckBox dummy_checkbox;
  default_checkbox_size = dummy_checkbox.sizeHint();
}


QCheckboxHeaderView::~QCheckboxHeaderView()
{
}


void QCheckboxHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
  painter->save();
  QHeaderView::paintSection(painter, rect, logicalIndex);
  painter->restore();
  if (isCheckable(logicalIndex))
  {
    QStyleOptionButton option;

    // Calculate the actual rect where we want the checkbox to be painted.
    int top_margin = (rect.height() - default_checkbox_size.height()) / 2;
    int left_margin = (rect.width() - default_checkbox_size.width()) / 2;
    if (left_margin > top_margin)
      left_margin = top_margin;

    QRect paint_rect = QRect(rect.x() + left_margin, rect.y() + top_margin, default_checkbox_size.width(), default_checkbox_size.height());
    option.rect = paint_rect;

    if (checkState(logicalIndex) == Qt::CheckState::Checked)
      option.state = QStyle::State_On;
    else if (checkState(logicalIndex) == Qt::CheckState::Unchecked)
      option.state = QStyle::State_Off;
    else if (checkState(logicalIndex) == Qt::CheckState::PartiallyChecked)
      option.state = QStyle::State_NoChange;

    if (isCheckboxEnabled(logicalIndex))
      option.state |= QStyle::State_Enabled;

    this->style()->drawControl(QStyle::CE_CheckBox, &option, painter);
  }
}

void QCheckboxHeaderView::mousePressEvent(QMouseEvent *event)
{
  int section = logicalIndexAt(event->pos());
  bool clicked_checkbox = false;

  if (isCheckable(section))
  {
    // Check if the user clicked on the checkbox or the area around it

    int section_x = 0;
    int section_y = 1;
    int section_width = 0;
    int section_height = 0;

    section_x = sectionViewportPosition(section);

    section_height = height();
    section_width = sectionSize(section);

    // calculate the rect of the checkbox just like before
    int top_margin = (section_height - default_checkbox_size.height()) / 2;
    int left_margin = (section_width - default_checkbox_size.width()) / 2;
    if (left_margin > top_margin)
      left_margin = top_margin;

    QRect paint_rect = QRect(section_x + left_margin, section_y + top_margin, default_checkbox_size.width(), default_checkbox_size.height());

    if (paint_rect.contains(event->pos()))
    {
      clicked_checkbox = true;
      if (isCheckboxEnabled(section))
      {
        setChecked(section, !(checkState(section) == Qt::CheckState::Checked));
        emit clicked(section, isChecked(section));
      }
    }
  }

  if (!clicked_checkbox)
  {
    QHeaderView::mousePressEvent(event);
  }
}





////////////////////////////////////////////////////////////////////////////////
//// API for the checkboxes                                                 ////
////////////////////////////////////////////////////////////////////////////////

void QCheckboxHeaderView::setCheckable(int column, bool checkable)
{
  if (checkbox_data_.count(column) == 1)
  {
    // The column already exists
    auto checkbox_data = checkbox_data_[column];
    std::get<0>(checkbox_data) = checkable;
    checkbox_data_[column] = checkbox_data;
  }
  else
  {
    // create a new column with default data
    auto checkbox_data = std::make_tuple(checkable, true, Qt::CheckState::Unchecked);
    checkbox_data_[column] = checkbox_data;
  }
  viewport()->update();
}

void QCheckboxHeaderView::setCheckboxEnabled(int column, bool enabled)
{
  if (checkbox_data_.count(column) == 1)
  {
    // The column already exists
    auto checkbox_data = checkbox_data_[column];
    std::get<1>(checkbox_data) = enabled;
    checkbox_data_[column] = checkbox_data;
  }
  else
  {
    // create a new column with default data
    auto checkbox_data = std::make_tuple(false, enabled, Qt::CheckState::Unchecked);
    checkbox_data_[column] = checkbox_data;
  }
  viewport()->update();
}

void QCheckboxHeaderView::setChecked(int column, bool checked)
{
  setCheckState(column, checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
}

void QCheckboxHeaderView::setCheckState(int column, Qt::CheckState checked)
{
  if (checkbox_data_.count(column) == 1)
  {
    // The column already exists
    auto checkbox_data = checkbox_data_[column];
    std::get<2>(checkbox_data) = checked;
    checkbox_data_[column] = checkbox_data;
  }
  else
  {
    // create a new column with default data
    auto checkbox_data = std::make_tuple(false, true, checked);
    checkbox_data_[column] = checkbox_data;
  }
  viewport()->update();
  emit stateChanged(column, checked);
}

bool QCheckboxHeaderView::isCheckable(int column) const
{
  if (checkbox_data_.count(column) == 1)
  {
    return std::get<0>(checkbox_data_.at(column));
  }
  else
  {
    return false;
  }
}

bool QCheckboxHeaderView::isCheckboxEnabled(int column) const
{
  if (checkbox_data_.count(column) == 1)
  {
    return std::get<1>(checkbox_data_.at(column));
  }
  else
  {
    return false;
  }
}

bool QCheckboxHeaderView::isChecked(int column) const
{
  if (checkbox_data_.count(column) == 1)
  {
    return (std::get<2>(checkbox_data_.at(column)) == Qt::CheckState::Checked);
  }
  else
  {
    return false;
  }
}

Qt::CheckState QCheckboxHeaderView::checkState(int column) const
{
  if (checkbox_data_.count(column) == 1)
  {
    return std::get<2>(checkbox_data_.at(column));
  }
  else
  {
    return Qt::CheckState::Unchecked;
  }
}