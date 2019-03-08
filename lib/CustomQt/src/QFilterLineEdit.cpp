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

#include "CustomQt/QFilterLineEdit.h"

#include <QStyle>
#include <QAction>
#include <QKeyEvent>
#include <QApplication>

QFilterLineEdit::QFilterLineEdit(QWidget *parent)
  : QFilterLineEdit("", parent)
{}

QFilterLineEdit::QFilterLineEdit(const QString &contents, QWidget *parent)
  : QLineEdit(contents, parent)
{
  clear_lineedit_action_ = new QAction(this);
  addAction(clear_lineedit_action_, QLineEdit::ActionPosition::TrailingPosition);
  clear_lineedit_action_->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_LineEditClearButton)));

  connect(this, &QLineEdit::textChanged, [this](const QString& text) { clear_lineedit_action_->setEnabled(!text.isEmpty()); });
  connect(clear_lineedit_action_, &QAction::triggered, [this]() {clear(); });

  clear_lineedit_action_->setEnabled(!contents.isEmpty());
}

QFilterLineEdit::~QFilterLineEdit()
{}

void QFilterLineEdit::setClearIcon(const QIcon& icon)
{
  clear_lineedit_action_->setIcon(icon);
}

void QFilterLineEdit::keyPressEvent(QKeyEvent* key_event)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
  if (key_event->key() == Qt::Key::Key_Escape)
#else // QT_VERSION
  if (key_event->matches(QKeySequence::StandardKey::Cancel))
#endif // QT_VERSION
  {
    clear();
  }
  else
  {
    QLineEdit::keyPressEvent(key_event);
  }
}