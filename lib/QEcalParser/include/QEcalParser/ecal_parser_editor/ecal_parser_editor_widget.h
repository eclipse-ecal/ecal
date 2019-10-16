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

#include <QWidget>

namespace Ui {
  class EcalParserEditor;
}
namespace EcalParser
{
  class Function;
}
class FunctionListModel;
class QMulticolumnSortFilterProxyModel;

class QEcalParserEditor : public QWidget
{
  Q_OBJECT

public:
  QEcalParserEditor(QWidget *parent = Q_NULLPTR);
  ~QEcalParserEditor();

  void setText(const QString& text);
  QString text();

private slots:
  void updatePreview();
  void updateFunctionHelp();

private:
  static QString css();
  static QString generateFunctionHtmlHelp(std::pair<QString, EcalParser::Function*> function_pair);
  static QString generateGeneralHtmlHelp();

private:
  Ui::EcalParserEditor*             ui_;
  FunctionListModel*                function_list_model_;
  QMulticolumnSortFilterProxyModel* function_list_proy_model_;
};
