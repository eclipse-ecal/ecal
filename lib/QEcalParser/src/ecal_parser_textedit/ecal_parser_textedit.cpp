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

#include <QEcalParser/ecal_parser_textedit/ecal_parser_textedit.h>

#include <QEcalParser/ecal_parser_editor/ecal_parser_editor_dialog.h>

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QContextMenuEvent>
#include <QToolButton>
#include <QScrollBar>
#include <QStyle>

QEcalParserTextEdit::QEcalParserTextEdit(QWidget *parent)
  : QEcalParserTextEdit("", parent)
{
}

QEcalParserTextEdit::QEcalParserTextEdit(const QString &contents, QWidget *parent)
  : QTextEdit(contents, parent)
{
  Q_INIT_RESOURCE(qecalparser);

  open_dialog_action_ = new QAction(this);
  open_dialog_action_->setIcon(QIcon(":/qecalparser/SHOW_DIALOG"));
  addAction(open_dialog_action_);
  open_dialog_action_->setToolTip(tr("Advanced editor..."));
  open_dialog_action_->setText(tr("Advanced editor..."));

  connect(open_dialog_action_, &QAction::triggered, this, &QEcalParserTextEdit::openDialog);

  overlay_button_ = new QToolButton(this);
  overlay_button_->setStyleSheet(
    R"(
QToolButton
{
  background-color: rgba(255, 255, 255, 80%);
  border: none;
}
)"
  );
  overlay_button_->setDefaultAction(open_dialog_action_);
  overlay_button_->raise();
}

QEcalParserTextEdit::~QEcalParserTextEdit()
{}

void QEcalParserTextEdit::openDialog()
{
  QEcalParserEditorDialog dialog(toPlainText(), this);
  if (dialog.exec() == QDialog::Accepted)
  {
    setText(dialog.text());
  }
}

void QEcalParserTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = createStandardContextMenu();

  menu->addSeparator();
  menu->addAction(open_dialog_action_);
  menu->exec(event->globalPos());

  delete menu;
}

void QEcalParserTextEdit::resizeEvent(QResizeEvent* event)
{
  QTextEdit::resizeEvent(event);
  moveOverlayButton();
}

void QEcalParserTextEdit::showEvent(QShowEvent *event)
{
  QTextEdit::showEvent(event);
  moveOverlayButton();
}

void QEcalParserTextEdit::moveOverlayButton()
{
  int scrollbar_width = 0;
  if (verticalScrollBar()->isVisible())
    scrollbar_width = verticalScrollBar()->width();

  int frame_width = style()->pixelMetric(QStyle::PixelMetric::PM_DefaultFrameWidth, nullptr, this);

  overlay_button_->move(QPoint(width() - overlay_button_->width() - scrollbar_width - frame_width, frame_width));
}
