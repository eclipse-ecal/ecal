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

#pragma once

#include <QDialog>

namespace Ui {
  class EcalParserEditorDialog;
}

class QEcalParserEditorWidget;

class QEcalParserEditorDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QEcalParserEditorDialog(const QString& text, QWidget *parent = 0);
  explicit QEcalParserEditorDialog(QWidget *parent = 0);

  ~QEcalParserEditorDialog();

  QString text() const;

private:
  Ui::EcalParserEditorDialog *ui_;
};