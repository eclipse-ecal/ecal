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

QEcalParserTextEdit::QEcalParserTextEdit(QWidget *parent)
  : QEcalParserTextEdit("", parent)
{
  Q_INIT_RESOURCE(qecalparser);
}

QEcalParserTextEdit::QEcalParserTextEdit(const QString &contents, QWidget *parent)
  : QTextEdit(contents, parent)
{
  open_dialog_action = new QAction(this);
  open_dialog_action->setIcon(QIcon(":/qecalparser/SHOW_DIALOG"));
  addAction(open_dialog_action);
  open_dialog_action->setToolTip(tr("Advanced editor..."));
  open_dialog_action->setText(tr("Advanced editor..."));

  connect(open_dialog_action, &QAction::triggered, this, &QEcalParserTextEdit::openDialog);
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
  menu->addAction(open_dialog_action);
  menu->exec(event->globalPos());

  delete menu;
}