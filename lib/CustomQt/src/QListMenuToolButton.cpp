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

#include "CustomQt/QListMenuToolButton.h"

#include <QStyleOptionToolButton>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QApplication>
#include <QScreen>

QListMenuToolButton::QListMenuToolButton(QWidget* parent)
  : QToolButton(parent)
  , max_visible_items_(10)
{
  popup_     = new QMainWindow(this, Qt::WindowType::Popup);
  list_view_ = new QListView(popup_);

  list_view_->setUniformItemSizes(true);
  list_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  list_view_->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);

  popup_->setCentralWidget(list_view_);

  connect(list_view_, &QListView::clicked,   this, &QListMenuToolButton::listItemClicked);
  connect(list_view_, &QListView::activated, this, &QListMenuToolButton::listItemClicked);
}

QListMenuToolButton::~QListMenuToolButton()
{}

void QListMenuToolButton::showListMenu()
{
  if (!model())
  {
    return;
  }

  // Height
  if (model()->rowCount() == 0)
  {
    return;
  }

  int row_height    = list_view_->sizeHintForRow(0);
  int visible_items = model()->rowCount();
  int frame_width   = list_view_->frameWidth();

  if (max_visible_items_ > 0)
  {
    visible_items = std::min(max_visible_items_, model()->rowCount());
  }

  int target_height = visible_items * row_height + 2 * frame_width;


  // Width
  int target_width = 200;
  auto list_view_model = list_view_->model();
  for (int i = 0; i < list_view_model->rowCount(); i++)
  {
    target_width = std::max(target_width, list_view_->sizeHintForIndex(list_view_model->index(i, modelColumn())).width());
  }
  target_width += 2 * frame_width;
  if (visible_items < list_view_model->rowCount())
  {
    target_width += qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
  }

  // Position on screen
  int screen_number     = QApplication::desktop()->screenNumber(this);
  QRect screen_geometry = QApplication::screens()[screen_number]->geometry();

  QRect global_button_geometry = QRect(mapToGlobal(QPoint(0, 0)), size());
  QRect popup_geometry         = QRect(global_button_geometry.bottomLeft(), QSize(target_width, target_height));

  if (global_button_geometry.bottomLeft().y() + target_height <= (screen_geometry.y() + screen_geometry.height()))
  {
    // position below
    popup_geometry.moveTo(global_button_geometry.bottomLeft());
  }
  else if(global_button_geometry.bottomLeft().y() - target_height >= screen_geometry.y())
  {
    // position above
    popup_geometry.moveTo(global_button_geometry.topLeft() - QPoint(0, target_height));
  }
  else
  {
    // Start with positioning it below. Deal with re-positioning it later.
    popup_geometry.moveTo(global_button_geometry.bottomLeft());
  }

  // Move the popup to be on screen
  if (popup_geometry.right() > screen_geometry.right())
  {
    popup_geometry.moveRight(screen_geometry.right());
  }
  if (popup_geometry.bottom() > screen_geometry.bottom())
  {
    popup_geometry.moveBottom(screen_geometry.bottom());
  }
  if (popup_geometry.left() < screen_geometry.left())
  {
    popup_geometry.moveLeft(screen_geometry.left());
  }
  if (popup_geometry.top() < screen_geometry.top())
  {
    popup_geometry.moveTop(screen_geometry.top());
  }

  // We are now only sure that the top left edge is on screen. The popup may still be too large.
  if (popup_geometry.right() > screen_geometry.right())
  {
    popup_geometry.setRight(screen_geometry.right());
  }
  if (popup_geometry.bottom() > screen_geometry.bottom())
  {
    popup_geometry.setBottom(screen_geometry.bottom());
  }

  popup_    ->move(popup_geometry.topLeft());
  popup_    ->setFixedSize(popup_geometry.size());
  list_view_->setFixedSize(popup_geometry.size());

  // Scroll to the current selection
  auto selected_indexes = list_view_->selectionModel()->selectedIndexes();
  if (selected_indexes.isEmpty())
  {
    if (list_view_model->rowCount() > 0)
    {
      list_view_->setCurrentIndex(list_view_model->index(0, modelColumn()));
    }
    list_view_->scrollToTop();
  }
  else
  {
    list_view_->setCurrentIndex(selected_indexes.at(0));
    list_view_->scrollTo(selected_indexes.at(0));
  }

  popup_->show();
  list_view_->setFocus();
}

void QListMenuToolButton::mousePressEvent(QMouseEvent *e)
{
  QStyleOptionToolButton style_option_tool_button;
  initStyleOption(&style_option_tool_button);

  if ((e->button() == Qt::LeftButton)
    && (popupMode() == QToolButton::MenuButtonPopup))
  {
    QRect popup_button_rect = style()->subControlRect(QStyle::CC_ToolButton, &style_option_tool_button, QStyle::SC_ToolButtonMenu, this);
    if (popup_button_rect.isValid() && popup_button_rect.contains(e->pos()))
    {
      showListMenu();
      return;
    }
  }

  QToolButton::mousePressEvent(e);
}

void QListMenuToolButton::listItemClicked(const QModelIndex& index)
{
  popup_->hide();
  emit itemSelected(index);
}

void QListMenuToolButton::setModel(QAbstractItemModel* model)
{
  list_view_->setModel(model);
}
void QListMenuToolButton::setModelColumn(int visible_column)
{
  list_view_->setModelColumn(visible_column);
}

void QListMenuToolButton::setMaxVisibleItems(int max_items)
{
  max_visible_items_ = max_items;
}


QAbstractItemModel* QListMenuToolButton::model() const
{
  return list_view_->model();
}

int QListMenuToolButton::modelColumn() const
{
  return list_view_->modelColumn();
}

int QListMenuToolButton::maxVisibleItems() const
{
  return max_visible_items_;
}

void QListMenuToolButton::setSelection(const QModelIndex& selection)
{
  if (selection.isValid())
  {
    list_view_->selectionModel()->select(selection, QItemSelectionModel::SelectionFlag::ClearAndSelect);
  }
  else
  {
    list_view_->selectionModel()->clearSelection();
  }
  emit itemSelected(selection);
}

void QListMenuToolButton::setSelection(int row)
{
  if (row > 0)
  {
    setSelection(list_view_->model()->index(row, modelColumn()));
  }
  else
  {
    setSelection(QModelIndex());
  }
}

void QListMenuToolButton::clearSelection()
{
  setSelection(QModelIndex());
}