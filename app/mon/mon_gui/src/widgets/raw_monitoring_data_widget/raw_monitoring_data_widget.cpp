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

#include "raw_monitoring_data_widget.h"

#include <ecal/ecal.h>

#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QColor>
#include <QBrush>

#include <QAction>

RawMonitoringDataWidget::RawMonitoringDataWidget(QWidget *parent)
  : QWidget(parent)
  , protobuf_highlighter_(nullptr)
{
  ui_.setupUi(this);

  search_clear_action_ = new QAction(QIcon(":/ecalicons/CLEAR"), tr("Clear"), this);
  search_down_action_  = new QAction(QIcon(":/ecalicons/HIDE"), tr("Search down"), this);
  search_up_action_    = new QAction(QIcon(":/ecalicons/SHOW"), tr("Search up"), this);

  ui_.search_lineedit->addAction(search_clear_action_, QLineEdit::TrailingPosition);
  ui_.search_lineedit->addAction(search_down_action_, QLineEdit::TrailingPosition);
  ui_.search_lineedit->addAction(search_up_action_, QLineEdit::TrailingPosition);

  chooseCorrectHighlighting();

  connect(ui_.update_button,       &QPushButton::clicked, this, &RawMonitoringDataWidget::updateRawMonitoringData);
  connect(ui_.save_to_file_button, &QPushButton::clicked, this, &RawMonitoringDataWidget::saveToFile);
  connect(ui_.search_lineedit,     &QLineEdit::editingFinished, this, [this](){ this->searchForString(ui_.search_lineedit->text()); });
}

RawMonitoringDataWidget::~RawMonitoringDataWidget() = default;

void RawMonitoringDataWidget::setRawMonitoringData(const eCAL::pb::Monitoring& monitoring_pb)
{
  ui_.raw_monitoring_data_textedit->setPlainText(QString::fromStdString(monitoring_pb.DebugString()));
  ui_.save_to_file_button->setEnabled(true);
}

void RawMonitoringDataWidget::updateRawMonitoringData()
{
  std::string          monitoring_string;
  eCAL::pb::Monitoring monitoring_pb;

  if ((eCAL::Monitoring::GetMonitoring(monitoring_string) != 0) && !monitoring_string.empty() && monitoring_pb.ParseFromString(monitoring_string))
  {
    setRawMonitoringData(monitoring_pb);
  }
  else
  {
    ui_.raw_monitoring_data_textedit->setPlainText("- No monitoring data available -");
    ui_.save_to_file_button->setEnabled(false);
  }
}

void RawMonitoringDataWidget::saveToFile()
{
  const QString filename = QFileDialog::getSaveFileName(this, tr("Save raw monitoring data"), "mon_raw.txt", tr("Text files (*.txt)"));
  
  if (!filename.isEmpty())
  {
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream out(&file);
      out << ui_.raw_monitoring_data_textedit->toPlainText();
    }
    else
    {
      // Display error message
      QMessageBox error_message(
        QMessageBox::Icon::Critical
        , tr("Error")
        , tr("Failed to save raw data to file \"") + filename + "\""
        , QMessageBox::Button::Ok
        , this);
      error_message.exec(); 
    }
  }
}

void RawMonitoringDataWidget::changeEvent(QEvent* event)
{
  QWidget::changeEvent(event);

  if (event->type() == QEvent::Type::PaletteChange)
  {
    chooseCorrectHighlighting();
    event->accept();
  }
}

void RawMonitoringDataWidget::chooseCorrectHighlighting()
{
  // Get background of input fields
  const QColor background = palette().color(QPalette::ColorRole::Base);

  // Check if the bg color is dark or light
  const bool dark_mode = (background.toHsl().lightness() < 128);

  if (protobuf_highlighter_)
    protobuf_highlighter_->deleteLater();

  protobuf_highlighter_ = new ProtobufHighlighter(dark_mode, ui_.raw_monitoring_data_textedit->document());
}

// Function searchForString that gets the tex from the search_lineedit as input and searches the search_string in the raw_monitoring_data_textedit. If found, the search_string is selected and the cursor is moved to the found search_string.
void RawMonitoringDataWidget::searchForString(const QString& search_string)
{

  // Get the old cursor from the textedit
  QTextCursor old_cursor = ui_.raw_monitoring_data_textedit->textCursor();

  // Clear the old extra selections
  ui_.raw_monitoring_data_textedit->moveCursor(QTextCursor::MoveOperation::Start);
  ui_.raw_monitoring_data_textedit->setExtraSelections({});

  // Search the textedit for the search string until the end. All found occurences are set as ExtraSelection.
  QList<QTextEdit::ExtraSelection> extra_selections;
  while (ui_.raw_monitoring_data_textedit->find(search_string))
  {
    QTextEdit::ExtraSelection extra_selection;
    extra_selection.cursor = ui_.raw_monitoring_data_textedit->textCursor();
    extra_selection.format.setBackground(QBrush(QColor(243,168,29, 128)));
    extra_selections << extra_selection;
  }
  ui_.raw_monitoring_data_textedit->setExtraSelections(extra_selections);

  // restore old cursor
  ui_.raw_monitoring_data_textedit->setTextCursor(old_cursor);

  //// Clear old find-result-highlighting
  //ui_.raw_monitoring_data_textedit->setExtraSelections({});

  //if (search_string.isEmpty())
  //{
  //  cursor.clearSelection();
  //  ui_.raw_monitoring_data_textedit->moveCursor(QTextCursor::MoveOperation::Start);
  //  ui_.raw_monitoring_data_textedit->setExtraSelections({});
  //  return;
  //}

  ////ui_.raw_monitoring_data_textedit->setTextCursor(cursor);

  //if (ui_.raw_monitoring_data_textedit->find(search_string))
  //{
  //  //ui_.raw_monitoring_data_textedit->setTextColor(palette().color(QPalette::ColorRole::Text));
  //  ui_.raw_monitoring_data_textedit->setExtraSelections({});
  //}
  //else
  //{
  //  //ui_.raw_monitoring_data_textedit->setTextColor(palette().color(QPalette::ColorRole::BrightText));
  //  QList<QTextEdit::ExtraSelection> extra_selections;
  //  QTextEdit::ExtraSelection extra_selection;
  //  extra_selection.format.setBackground(palette().color(QPalette::ColorRole::Highlight));
  //  extra_selection.format.setForeground(palette().color(QPalette::ColorRole::HighlightedText));
  //  extra_selection.cursor = ui_.raw_monitoring_data_textedit->textCursor();
  //  extra_selections.append(extra_selection);
  //  ui_.raw_monitoring_data_textedit->setExtraSelections(extra_selections);
  //}
}
