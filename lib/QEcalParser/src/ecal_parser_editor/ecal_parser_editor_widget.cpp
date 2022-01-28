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

#include <QEcalParser/ecal_parser_editor/ecal_parser_editor_widget.h>

#include "ui_ecal_parser_editor_widget.h"

#include <EcalParser/EcalParser.h>

#include "function_list_model.h"
#include <CustomQt/QMulticolumnSortFilterProxyModel.h>

QEcalParserEditor::QEcalParserEditor(QWidget *parent)
  : QWidget(parent)
  , ui_(new Ui::EcalParserEditor)
{
  ui_->setupUi(this);

  // Function list
  function_list_model_      = new FunctionListModel(this);
  function_list_proy_model_ = new QMulticolumnSortFilterProxyModel(this);

  function_list_proy_model_->setSortCaseSensitivity  (Qt::CaseSensitivity::CaseInsensitive);
  function_list_proy_model_->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  function_list_proy_model_->setFilterKeyColumns     ({ (int)FunctionListModel::Columns::NAME, (int)FunctionListModel::Columns::DESCRIPTION });
  function_list_proy_model_->setSourceModel(function_list_model_);

  ui_->function_list->setModel(function_list_proy_model_);
  ui_->function_list->setFirstColumnSpanned(function_list_proy_model_->mapFromSource(function_list_model_->index(0, 0)).row(), QModelIndex(), true);
  ui_->function_list->sortByColumn(0, Qt::SortOrder::AscendingOrder);

  connect(ui_->function_filter_lineedit, &QLineEdit::textChanged, function_list_proy_model_, &QMulticolumnSortFilterProxyModel::setFilterFixedString);

  // Function help
  connect(ui_->function_list->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QEcalParserEditor::updateFunctionHelp);

  //initial selection
  ui_->function_list->selectionModel()->select(function_list_proy_model_->mapFromSource(function_list_model_->index(0, 0)), QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);

  // Preview
  connect(ui_->entry_textedit,               &QTextEdit::textChanged,   this, &QEcalParserEditor::updatePreview);
  connect(ui_->preview_complete_radiobutton, &QAbstractButton::clicked, this, &QEcalParserEditor::updatePreview);
  connect(ui_->preview_host_radiobutton,     &QAbstractButton::clicked, this, &QEcalParserEditor::updatePreview);
}

QEcalParserEditor::~QEcalParserEditor()
{
  delete ui_;
}

void QEcalParserEditor::updatePreview()
{
  ui_->preview_textedit->setPlainText(QString::fromStdString(EcalParser::Evaluate(ui_->entry_textedit->toPlainText().toStdString(), ui_->preview_complete_radiobutton->isChecked())));
}

void QEcalParserEditor::updateFunctionHelp()
{
  auto proxy_index_list = ui_->function_list->selectionModel()->selectedRows();

  if (proxy_index_list.size() != 1)
  {
    ui_->function_help_textedit->clear();
  }
  else
  {
    QModelIndex selected_source_index = function_list_proy_model_->mapToSource(proxy_index_list.front());
    auto function_pair = function_list_model_->getFunction(selected_source_index.row());

    if (function_pair.second)
      ui_->function_help_textedit->setText(generateFunctionHtmlHelp(function_pair));
    else
      ui_->function_help_textedit->setText(generateGeneralHtmlHelp());
  }
}

void QEcalParserEditor::setText(const QString& text)
{
  ui_->entry_textedit->setPlainText(text);
}

QString QEcalParserEditor::text()
{
  return ui_->entry_textedit->toPlainText();
}

QString QEcalParserEditor::css()
{
  return R"(
#usage {
    color: rgb(0,0,0);
    font-family: "Courier New", Courier, monospace;
    background-color: rgb(220,220,220);
}
#code {
    color: rgb(0,0,0);
    font-family: "Courier New", Courier, monospace;
    background-color: rgb(220,220,220);
}
#example {
    color: rgb(0,0,0);
    font-family: "Courier New", Courier, monospace;
    background-color: rgb(220,220,220);
    margin-left: 10px;
    margin-right: 10px;
    padding: 10px;
})";
}

QString QEcalParserEditor::generateFunctionHtmlHelp(std::pair<QString, EcalParser::Function*> function_pair)
{
  QString html_help = R"(
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html>
<head>
<style type="text/css">
%CSS_STRING%
}
</style>
</head>

<body>
<h1>%FUNCTION_NAME_STRING%</h1>
<p>%SHORT_HELP_STRING%</p>
<h2>Usage</h2>
<p id="usage">%USAGE_STRING%</p>
<p> Example: </p>
<p id="example">%EXAMPLE_STRING%</p>
<p id="example">%EVALUATED_EXAMPLE_STRING%</p>
<h2>Documentation</h2>
%HTML_DOC_STRING%
</body>

</html>
)";

  QString usage_string      = "$HOST{" + function_pair.first + (function_pair.second->ParameterUsage().empty() ? "" : " ")    + QString::fromStdString(function_pair.second->ParameterUsage())    + "}";
  QString example_string    = "$HOST{" + function_pair.first + (function_pair.second->ParameterExample().empty() ? "" : " ") + QString::fromStdString(function_pair.second->ParameterExample()) + "}";
  QString evaluated_example = QString::fromStdString(EcalParser::Evaluate(example_string.toStdString(), true));

  html_help.replace("%CSS_STRING%",               css());
  html_help.replace("%FUNCTION_NAME_STRING%",     function_pair.first);
  html_help.replace("%SHORT_HELP_STRING%",        QString::fromStdString(function_pair.second->Description()).toHtmlEscaped());
  html_help.replace("%USAGE_STRING%",             usage_string.toHtmlEscaped());
  html_help.replace("%EXAMPLE_STRING%",           example_string.toHtmlEscaped());
  html_help.replace("%EVALUATED_EXAMPLE_STRING%", evaluated_example.toHtmlEscaped());
  html_help.replace("%HTML_DOC_STRING%",          QString::fromStdString(function_pair.second->HtmlDocumentation()));
  
  return html_help;
}

QString QEcalParserEditor::generateGeneralHtmlHelp()
{
  QString html_help = R"(
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html>
<head>
<style type="text/css">
%CSS_STRING%
</style>
</head>

<body>

<h1>eCAL Syntax</h1>
<p> Some inputs in eCAL Applications can be provided in a special syntax. The syntax is parsed and evaluated before or after sending the input to the client application. </p>

<h2>Syntax</h2>
<p> Several functions exist. Some of them accept additional parameters. The format is: </p>
<p id="example">$HOST{FUNCTION Parameters}</p>
<p>
The <b>escape character</b> is the back-tick (<span id="code">`</span>). The dollar sign, braces and back-tick have to be escaped , if they are supposed to appear in the output (<span id="code">`$</span>, <span id="code">`{</span>, <span id="code">`}</span>, <span id="code">``</span>).
</p>

<h2>Host / Target evaluation</h2>
<p>
eCAL is often used to let multiple machines communicate with each other. The eCAL Syntax can therefore differentiate between the current machine (Host) and the client machine (Target), which receives the input.
</p>
<p>
A function that shall be evaluated on the <b>host</b> (i.e. be evaluated before sending the string to the client application) starts with <span id="code">$HOST</span>:
</p>
<p id="example">
$HOST{FUNCTION Parameters}
</p>
<p>
A function that shall be evaluated by the <b>target</b>, e.g. because it uses a function that evaluates differently on the client, starts with <span id="code">$TARGET</span>:
</p>
<p id="example">
$TARGET{FUNCTION Parameters}
</p>

<p>
Consider the following example: You want to record locally on 2 machines (one is a Windows, the other one a Linux machine). Therefore, you have created an environment variable <span id="code">MEAS_ROOT</span> on both machines:
</p>
<p>
<ul>
  <li>Machine1: <span id="code">MEAS_ROOT=C:\measurement</span></li>
  <li>Machine2: <span id="code">MEAS_ROOT=/measurement</span></li>
</ul>
</p>
<p>
You also want the measurement to be in a directory with the current timestamp. You cannot assume that both machines will evaluate the <span id="code">TIME</span> function exactly the same, so you want that to be evaluated once by the host machine.
</p>
<p>
Your solution could look like this:
</p>
<p id="example">
$TARGET{ENV MEAS_ROOT}/$HOST{TIME}
</p>

<p>
This will first evaluate to:
</p>
<p id="example">
$TARGET{ENV MEAS_ROOT}/%CURRENT_TIME_STRING%
</p>

<p>
This string will then be passed to the client applications and evaluate differently on both machines:
</p>
<p>
<ul>
  <li>Machine1: <span id="code">C:\measurement/%CURRENT_TIME_STRING%</span></li>
  <li>Machine2: <span id="code">/measurement/%CURRENT_TIME_STRING%</span></li>
</ul>
</p>

</body>

</html>
)";
  html_help.replace("%CSS_STRING%", css());
  html_help.replace("%CURRENT_TIME_STRING%", QString::fromStdString(EcalParser::Evaluate("$HOST{TIME}", true)).toHtmlEscaped());

  return html_help;
}