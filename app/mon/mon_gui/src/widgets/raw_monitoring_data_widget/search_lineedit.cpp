/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2023 Continental Corporation
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

#include "search_lineedit.h"

#include <QStyle>
#include <QAction>
#include <QKeyEvent>
#include <QApplication>

/////////////////////////////////////////
// Constructors & Destructor
/////////////////////////////////////////

SearchLineedit::SearchLineedit(QWidget *parent)
  : SearchLineedit("", parent)
{}

SearchLineedit::SearchLineedit(const QString &contents, QWidget *parent)
  : QLineEdit(contents, parent)
  , clear_lineedit_action_(nullptr)
  , search_next_action    (nullptr)
  , search_previous_action(nullptr)
{
  // Clear lineedit action
  clear_lineedit_action_ = new QAction(tr("Clear"), this);
  addAction(clear_lineedit_action_, QLineEdit::ActionPosition::TrailingPosition);
  clear_lineedit_action_->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_LineEditClearButton)));
  connect(clear_lineedit_action_, &QAction::triggered, this, [this]() {clear(); });
  clear_lineedit_action_->setEnabled(!contents.isEmpty());

  connect(this, &QLineEdit::textChanged
    , [this](const QString& text)
            {
              clear_lineedit_action_->setEnabled(!text.isEmpty());
              search_next_action    ->setEnabled(!text.isEmpty());
              search_previous_action->setEnabled(!text.isEmpty());

              if (text.isEmpty())
              {
                // If the text is empty, we trigger a "new search", so the highlighting is cleared.
                last_search_text_ = "";
                emit newSearchTriggered("");
              }
            });

  // Find next / Find previous actions
  search_next_action     = new QAction(tr("Find next"), this);
  search_previous_action = new QAction(tr("Find previous"), this);

  connect(search_next_action,     &QAction::triggered, this, [this]() { searchNext(); });
  connect(search_previous_action, &QAction::triggered, this, [this]() { searchPrevious(); });

  addAction(search_previous_action, QLineEdit::TrailingPosition);
  addAction(search_next_action,     QLineEdit::TrailingPosition);

  chooseThemeIcons();
}

/////////////////////////////////////////
// Icons
/////////////////////////////////////////

void SearchLineedit::setClearIcon(const QIcon& icon)
{
  clear_lineedit_action_->setIcon(icon);
}

void SearchLineedit::chooseThemeIcons()
{
  // Get background of input fields
  const QColor background = palette().color(QPalette::ColorRole::Base);

  // Check if the bg color is dark or light
  const bool dark_mode = (background.toHsl().lightness() < 128);

  //TODO: change icon of clear_lineedit_action_
  search_next_action    ->setIcon(QIcon(QString(":/ecalicons/") + (dark_mode ? "darkmode/" : "") + "ARROW_DOWN"));
  search_previous_action->setIcon(QIcon(QString(":/ecalicons/") + (dark_mode ? "darkmode/" : "") + "ARROW_UP"));
}

/////////////////////////////////////////
// Signals and slots for search functionality
/////////////////////////////////////////

void SearchLineedit::searchNext()
{
  if (last_search_text_ != text())
  {
    last_search_text_ = text();
    emit newSearchTriggered(text());
  }

  emit searchNextTriggered(text());
}

void SearchLineedit::searchPrevious()
{
  if (last_search_text_ != text())
  {
    last_search_text_ = text();
    emit newSearchTriggered(text());
  }

  emit searchPreviousTriggered(text());
}

/////////////////////////////////////////
// Qt Events override
/////////////////////////////////////////

void SearchLineedit::keyPressEvent(QKeyEvent* key_event)
{
  if (key_event->matches(QKeySequence::StandardKey::Cancel))
  {
    clear();
  }
  else if (key_event->matches(QKeySequence::StandardKey::FindNext)
          || key_event->key() == Qt::Key_Enter
          || key_event->key() == Qt::Key_Return)
  {
    searchNext();
  }
  else if (key_event->matches(QKeySequence::StandardKey::FindPrevious))
  {
    searchPrevious();
  }
  else
  {
    QLineEdit::keyPressEvent(key_event);
  }
}

void SearchLineedit::changeEvent(QEvent* event)
{
  QLineEdit::changeEvent(event);

  if (event->type() == QEvent::Type::PaletteChange)
  {
    chooseThemeIcons();
    event->accept();
  }
}


