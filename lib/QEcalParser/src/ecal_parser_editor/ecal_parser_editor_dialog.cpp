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

#include <QEcalParser/ecal_parser_editor/ecal_parser_editor_dialog.h>

#include "ui_ecal_parser_editor_dialog.h"

QEcalParserEditorDialog::QEcalParserEditorDialog(const QString& text, QWidget *parent)
  : QDialog(parent)
  , ui_(new Ui::EcalParserEditorDialog)
{
    ui_->setupUi(this);
    ui_->editor->setText(text);
}

QEcalParserEditorDialog::QEcalParserEditorDialog(QWidget *parent)
  : QEcalParserEditorDialog(QString(), parent)
{}

QEcalParserEditorDialog::~QEcalParserEditorDialog()
{
    delete ui_;
}

QString QEcalParserEditorDialog::text() const
{
  return ui_->editor->text();
}