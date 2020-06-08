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

#include "edit_button_delegate.h"

#include <QPushButton>
#include <QPainter>
#include <QAbstractItemView>
#include <QApplication>
#include <QMouseEvent>
#include <QDoubleSpinBox>
#include <QLineEdit>

#include <q_ecal_play.h>

#include "scenario_model.h"

//////////////////////////////////////////
// Constructor & Destructor
//////////////////////////////////////////

EditButtonDelegate::EditButtonDelegate(QAbstractItemView* parent)
  : QStyledItemDelegate(parent)
  , mouse_down_(false)
  , last_time_button_hover_(false)
{
  qApp->installEventFilter(this);
  parent->viewport()->setAttribute( Qt::WA_Hover ); // We need to repaint the buttons when the mouse hovers over a row
}

EditButtonDelegate::~EditButtonDelegate()
{}

//////////////////////////////////////////
// QStyledItemDelegate overrides
//////////////////////////////////////////

void EditButtonDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QRect button_rect = getButtonRect(option);

  // Get the ItemView of this delegate
  const QAbstractItemView* item_view = static_cast<const QAbstractItemView*>(option.widget);

  // Check whether the row and / or the item is hovered
  QPoint p = item_view->viewport()->mapFromGlobal(QCursor::pos());
  QModelIndex index_at_mouse = item_view->indexAt(p);

  bool cell_hover        = (index_at_mouse == index);
  //bool row_hover         = index_at_mouse.row() == index.row();
  bool button_hover      = button_rect.contains(p);
  bool currently_editing = (index_currently_editing_ == index);

  if (cell_hover && !currently_editing)
  {
    // Save the rect of the current button, as the mouse click event needs that
    const_cast<EditButtonDelegate*>(this)->button_rect_ = button_rect;
  }
  else if (cell_hover)
  {
    const_cast<EditButtonDelegate*>(this)->button_rect_ = QRect(0, 0, 0, 0);
  }

  // Call base painter
  QStyleOptionViewItem option_for_base_painter(option);
  if (cell_hover && !currently_editing)
  {
    //option_for_base_painter.rect.setLeft(button_rect.width());
    option_for_base_painter.rect.setWidth(option_for_base_painter.rect.width() - button_rect.width());
  }
  QStyledItemDelegate::paint(painter, option_for_base_painter, index); // call base

  if (cell_hover && !currently_editing)
  {
    // At least on Ubuntu 16.04 with Qt 5.5 we can only paint elements starting
    // at 0,0. Thus, we create a rect with the same width and height as the
    // target rect and then move the painter back and forth to paint in the
    // given rect.
    painter->save();
    painter->translate(button_rect.topLeft());

    // Button Bevel
    QStyleOptionButton button_bevel_options;
    button_bevel_options.rect        = QRect(0, 0, button_rect.width(), button_rect.height());
    button_bevel_options.state       = QFlags<QStyle::StateFlag>(QStyle::StateFlag::State_Enabled & ~QStyle::StateFlag::State_HasFocus);
    //button_bevel_options.features    = QStyleOptionButton::ButtonFeature::Flat;

    if (button_hover)
      button_bevel_options.state |= (QStyle::StateFlag::State_HasFocus | QStyle::StateFlag::State_MouseOver);

    item_view->style()->drawControl(QStyle::CE_PushButtonBevel, &button_bevel_options, painter);


    // Button Content
    QStyleOptionButton button_label_options;

    QRect button_content_rect = QApplication::style()->subElementRect(QStyle::SE_PushButtonContents, &button_bevel_options);
  
    const int icon_size     = item_view->style()->pixelMetric(QStyle::PM_ButtonIconSize);
  
    painter->translate(button_content_rect.topLeft());

    button_label_options.rect        = QRect(0, 0, button_content_rect.width(), button_content_rect.height());
    button_label_options.text        = "";
    button_label_options.icon        = QIcon(":ecalicons/EDIT");
    button_label_options.iconSize    = QSize(icon_size, icon_size);
    button_label_options.state       = QStyle::StateFlag::State_Enabled;
    //button_label_options.features    = QStyleOptionButton::ButtonFeature::Flat;


    if (button_hover)
      button_label_options.state |= QStyle::StateFlag::State_MouseOver;

    item_view->style()->drawControl(QStyle::CE_PushButtonLabel, &button_label_options, painter);

    painter->restore();
  }
}

QSize EditButtonDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  const QAbstractItemView* item_view = static_cast<const QAbstractItemView*>(option.widget);

  const int icon_size     = item_view->style()->pixelMetric(QStyle::PM_ButtonIconSize);
  const int button_margin = item_view->style()->pixelMetric(QStyle::PM_ButtonMargin);
  const int frame_width   = item_view->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

  // Check how big the string is

  QVariant data = index.data(Qt::ItemDataRole::DisplayRole);
  QString displayed_string;

  if (data.canConvert<QString>())
  {
    displayed_string = data.toString();
  }
  else if (data.canConvert<double>())
  {
    displayed_string = QString::number(data.toDouble());
  }
  QRect text_rect = item_view->fontMetrics().boundingRect(displayed_string);
  
  // Check how big the button is
  int button_size = icon_size + button_margin;

  return QSize(button_size + text_rect.width() + frame_width * 6, std::max(button_size, text_rect.height()));
}

QWidget* EditButtonDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex &index) const
{
  const ScenarioModel::Column column = (ScenarioModel::Column)index.column();
  QWidget* editor = nullptr;

  if (column == ScenarioModel::Column::SCENARIO_NAME)
  {
    editor = new QLineEdit(parent);
  }
  else if (column == ScenarioModel::Column::SCENARIO_TIMESTAMP)
  {
    QDoubleSpinBox* spin_box = new QDoubleSpinBox(parent);
    spin_box->setAlignment(Qt::AlignRight);
    spin_box->setMinimum(std::numeric_limits<double>().min());
    spin_box->setMaximum(std::numeric_limits<double>().max());
    spin_box->setSuffix(" sec");
    spin_box->setDecimals(3);

    editor = spin_box;
  }

  if (editor != nullptr)
  {
    const_cast<EditButtonDelegate*>(this)->index_currently_editing_ = index;
    connect(editor, &QWidget::destroyed, this
            , [this, index]()
              {
                // Reset the index that is currently being edited
                if (index_currently_editing_ == index)
                  const_cast<EditButtonDelegate*>(this)->index_currently_editing_ = QModelIndex();
              });
  }

  return editor;
}

void EditButtonDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  const ScenarioModel::Column column = (ScenarioModel::Column)index.column();

  if (column == ScenarioModel::Column::SCENARIO_NAME)
  {
    QLineEdit* line_edit = static_cast<QLineEdit*>(editor);
    line_edit->setText(index.data(Qt::ItemDataRole::EditRole).toString());
  }
  else if (column == ScenarioModel::Column::SCENARIO_TIMESTAMP)
  {
    QDoubleSpinBox* spin_box =static_cast<QDoubleSpinBox*>(editor);

    auto measurement_boundaries = QEcalPlay::instance()->measurementBoundaries();

    double min_rel_seconds = 0.0;
    double max_rel_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(measurement_boundaries.second - measurement_boundaries.first).count();

    double current_value = index.data(Qt::ItemDataRole::EditRole).toDouble();

    spin_box->setMinimum(std::min(min_rel_seconds, current_value));
    spin_box->setMaximum(std::max(max_rel_seconds, current_value));

    spin_box->setValue(current_value);
  }
}

void EditButtonDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  const ScenarioModel::Column column = (ScenarioModel::Column)index.column();

  if (column == ScenarioModel::Column::SCENARIO_NAME)
  {
    QLineEdit* line_edit = static_cast<QLineEdit*>(editor);
    line_edit->setGeometry(option.rect);
  }
  else if (column == ScenarioModel::Column::SCENARIO_TIMESTAMP)
  {
    QDoubleSpinBox* spin_box =static_cast<QDoubleSpinBox*>(editor);
    spin_box->setGeometry(option.rect);
  }
}

void EditButtonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  const ScenarioModel::Column column = (ScenarioModel::Column)index.column();

  if (column == ScenarioModel::Column::SCENARIO_NAME)
  {
    QLineEdit* line_edit = static_cast<QLineEdit*>(editor);
    model->setData(index, line_edit->text());
  }
  else if (column == ScenarioModel::Column::SCENARIO_TIMESTAMP)
  {
    QDoubleSpinBox* spin_box =static_cast<QDoubleSpinBox*>(editor);
    model->setData(index, spin_box->value());
  }
}

bool EditButtonDelegate::eventFilter(QObject* obj, QEvent *event)
{
  if (event->type() == QEvent::MouseMove)
  {    
    const QAbstractItemView* item_view = static_cast<const QAbstractItemView*>(parent());
    QPoint p = item_view->viewport()->mapFromGlobal(QCursor::pos());

    QModelIndex current_index      = item_view->indexAt(p);
    bool        currently_hovering = button_rect_.contains(p);

    if ((static_cast<QMouseEvent*>(event)->buttons() & Qt::MouseButton::LeftButton) == 0)
    {
      mouse_down_ = false;
    }

    if (last_time_hovered_index_.isValid()
      && (last_time_hovered_index_ == current_index)
      && (last_time_button_hover_ != currently_hovering)
      && (current_index != index_currently_editing_))
    {
      emit buttonStateChanged(current_index);
    }

    last_time_hovered_index_ = current_index;
    last_time_button_hover_  = currently_hovering;
  }
  else if (event->type() == QEvent::MouseButtonPress)
  {
    if (static_cast<QMouseEvent*>(event)->button() == Qt::MouseButton::LeftButton)
    {
      const QAbstractItemView* item_view = static_cast<const QAbstractItemView*>(parent());

      QPoint mouse_pos = static_cast<QMouseEvent*>(event)->globalPos();
      QPoint p = item_view->viewport()->mapFromGlobal(mouse_pos);

//      QModelIndex current_index = item_view->indexAt(p);

      if(button_rect_.contains(p))
      {
        mouse_down_ = true;
        return true;
      }
    }
  }
  else if (event->type() == QEvent::MouseButtonRelease)
  {
    if (mouse_down_
      && (static_cast<QMouseEvent*>(event)->button() == Qt::MouseButton::LeftButton))
    {
      const QAbstractItemView* item_view = static_cast<const QAbstractItemView*>(parent());
      QPoint p = item_view->viewport()->mapFromGlobal(QCursor::pos());
      QModelIndex current_index = item_view->indexAt(p);

      mouse_down_ = false;

      if(button_rect_.contains(p))
      {
        emit buttonClicked(current_index);
        return true;
      }
    }
  }

  return QStyledItemDelegate::eventFilter(obj, event);
}

//////////////////////////////////////////
// API of EditButtonDelegate
//////////////////////////////////////////

QRect EditButtonDelegate::getButtonRect(const QStyleOptionViewItem &option) const
{
  QRect button_rect;
  button_rect.setTop(option.rect.top());
  button_rect.setLeft(option.rect.left() + option.rect.width() - option.rect.height());
  button_rect.setWidth(option.rect.height());
  button_rect.setHeight(option.rect.height());

  return button_rect;
}
