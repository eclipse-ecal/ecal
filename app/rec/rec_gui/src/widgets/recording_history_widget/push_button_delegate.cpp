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

#include "push_button_delegate.h"

#include <QPushButton>
#include <QPainter>
#include <QAbstractItemView>
#include <QApplication>

#include "models/item_data_roles.h"
#include "job_history_model.h"
#include <qecalrec.h>

PushButtonDelegate::PushButtonDelegate(const QIcon& icon, const QString& text, const std::function<bool(const QModelIndex& index)>& is_enabled_function, QObject* parent)
  : QStyledItemDelegate(parent)
  , icon_(icon)
  , text_(text)
  , is_enabled_function_(is_enabled_function)
{}

PushButtonDelegate::PushButtonDelegate(const QString& text, const std::function<bool(const QModelIndex& index)>& is_enabled_function, QObject* parent)
  : PushButtonDelegate(QIcon(), text, is_enabled_function, parent)
{}

void PushButtonDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyledItemDelegate::paint(painter, option, index); // call base

  // Only paint buttons on the job-items
  if (index.parent().isValid())
    return;

  // Get the ItemView of this delegate
  const QAbstractItemView* item_view = static_cast<const QAbstractItemView*>(option.widget);

  // Check whether the row and / or the item is hovered
  QPoint p = item_view->viewport()->mapFromGlobal(QCursor::pos());
  bool cell_hover = (item_view->indexAt(p) == index);

  bool button_enabled = is_enabled_function_(index);

  // At least on Ubuntu 16.04 with Qt 5.5 we can only paint elements starting
  // at 0,0. Thus, we create a rect with the same width and height as the
  // target rect and then move the painter back and forth to paint in the
  // given rect.
  painter->save();
  painter->translate(option.rect.topLeft());

  // Button Bevel
  QStyleOptionButton button_bevel_options;
  button_bevel_options.rect        = QRect(0, 0, option.rect.width(), option.rect.height());
  button_bevel_options.state       = QFlags<QStyle::StateFlag>(QStyle::StateFlag::State_Enabled & ~QStyle::StateFlag::State_HasFocus);

  if (!button_enabled)
    button_bevel_options.state &= ~QStyle::StateFlag::State_Enabled;

  if (cell_hover)
    button_bevel_options.state |= (QStyle::StateFlag::State_HasFocus | QStyle::StateFlag::State_MouseOver);

  item_view->style()->drawControl(QStyle::CE_PushButtonBevel, &button_bevel_options, painter);


  // Button Content
  QStyleOptionButton button_label_options;

  QRect button_content_rect = QApplication::style()->subElementRect(QStyle::SE_PushButtonContents, &button_bevel_options);
  
  const int icon_size     = item_view->style()->pixelMetric(QStyle::PM_ButtonIconSize);

  const int space_left_for_text = button_content_rect.width() - 8 - icon_size;
  QString button_text = item_view->fontMetrics().elidedText(text_, Qt::TextElideMode::ElideRight, space_left_for_text);
  
  painter->translate(button_content_rect.topLeft());

  button_label_options.rect        = QRect(0, 0, button_content_rect.width(), button_content_rect.height());
  button_label_options.text        = button_text;
  button_label_options.icon        = icon_;
  button_label_options.iconSize    = QSize(icon_size, icon_size);
  button_label_options.state       = QStyle::StateFlag::State_Enabled;

  if (!button_enabled)
    button_label_options.state &= ~QStyle::StateFlag::State_Enabled;
  if (cell_hover)
    button_label_options.state |= QStyle::StateFlag::State_MouseOver;

  item_view->style()->drawControl(QStyle::CE_PushButtonLabel, &button_label_options, painter);

  painter->restore();
}

QSize PushButtonDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
  const QAbstractItemView* item_view = static_cast<const QAbstractItemView*>(option.widget);

  const int icon_size     = item_view->style()->pixelMetric(QStyle::PM_ButtonIconSize);
  const int button_margin = item_view->style()->pixelMetric(QStyle::PM_ButtonMargin);
  const int frame_width   = item_view->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

  int width;
  if (icon_.isNull())
  {
    width = (4 * frame_width) + item_view->fontMetrics().boundingRect(text_).width() + button_margin;
  }
  else
  {
    width = (4 * frame_width) + icon_size + 8 + item_view->fontMetrics().boundingRect(text_).width() + button_margin;
  }
  return QSize(width, icon_size + 4 * frame_width);
}