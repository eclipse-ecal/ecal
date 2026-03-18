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

#include "raw_monitoring_data_widget.h"

#include <ecal/ecal.h>

#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QColor>
#include <QBrush>
#include <QTimer>

////////////////////////////////////////////
// Constructor & Destructor
////////////////////////////////////////////

RawMonitoringDataWidget::RawMonitoringDataWidget(QWidget *parent)
  : QWidget(parent)
  , ui_{}
  , protobuf_highlighter_(nullptr)
{
  ui_.setupUi(this);

  chooseCorrectHighlighting();

  // Update Button
  connect(ui_.update_button,       &QPushButton::clicked, this, &RawMonitoringDataWidget::updateRawMonitoringData);

  // Save to file button
  connect(ui_.save_to_file_button, &QPushButton::clicked, this, &RawMonitoringDataWidget::saveToFile);

  // Search input
  connect(ui_.search_lineedit, &SearchLineedit::searchNextTriggered,     this, &RawMonitoringDataWidget::searchForward);
  connect(ui_.search_lineedit, &SearchLineedit::searchPreviousTriggered, this, &RawMonitoringDataWidget::searchBackward);
  connect(ui_.search_lineedit, &SearchLineedit::newSearchTriggered,      this, &RawMonitoringDataWidget::updateSearchHighlighting);
}

////////////////////////////////////////////
// Plaintext handling
////////////////////////////////////////////

void RawMonitoringDataWidget::setRawMonitoringData(const eCAL::Monitoring::SMonitoring& monitoring)
{
  // Create a formatted text representation of the monitoring data
  std::string output;
  output += "=== Monitoring Data ===\n\n";
  
  output += "Processes: " + std::to_string(monitoring.processes.size()) + "\n";
  for (const auto& proc : monitoring.processes)
  {
    output += "  - " + proc.process_name + " (PID: " + std::to_string(proc.process_id) + ")\n";
  }
  
  output += "\nPublishers: " + std::to_string(monitoring.publishers.size()) + "\n";
  for (const auto& pub : monitoring.publishers)
  {
    output += "  - " + pub.topic_name + " [" + pub.datatype_information.name + "]\n";
  }
  
  output += "\nSubscribers: " + std::to_string(monitoring.subscribers.size()) + "\n";
  for (const auto& sub : monitoring.subscribers)
  {
    output += "  - " + sub.topic_name + " [" + sub.datatype_information.name + "]\n";
  }
  
  output += "\nServers: " + std::to_string(monitoring.servers.size()) + "\n";
  for (const auto& srv : monitoring.servers)
  {
    output += "  - " + srv.service_name + "\n";
  }
  
  output += "\nClients: " + std::to_string(monitoring.clients.size()) + "\n";
  for (const auto& cli : monitoring.clients)
  {
    output += "  - " + cli.service_name + "\n";
  }
  
  ui_.raw_monitoring_data_textedit->setPlainText(QString::fromStdString(output));
  ui_.save_to_file_button->setEnabled(true);
  ui_.search_lineedit->setEnabled(true);
}

void RawMonitoringDataWidget::updateRawMonitoringData()
{
  eCAL::Monitoring::SMonitoring monitoring;

  if (eCAL::Monitoring::GetMonitoring(monitoring))
  {
    setRawMonitoringData(monitoring);
  }
  else
  {
    ui_.raw_monitoring_data_textedit->setPlainText("- No monitoring data available -");
    ui_.save_to_file_button->setEnabled(false);
    ui_.search_lineedit->setEnabled(false);
  }

  ui_.search_lineedit->clear();
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

void RawMonitoringDataWidget::chooseCorrectHighlighting()
{
  // Get background of input fields
  const QColor background = palette().color(QPalette::ColorRole::Base);

  // Check if the bg color is dark or light
  const bool dark_mode = (background.toHsl().lightness() < 128);

  if (protobuf_highlighter_ != nullptr)
    protobuf_highlighter_->deleteLater();

  protobuf_highlighter_ = new ProtobufHighlighter(dark_mode, ui_.raw_monitoring_data_textedit->document());
}

////////////////////////////////////////////
// Search handling
////////////////////////////////////////////

// Function updateSearchHighlighting that gets the tex from the search_lineedit as input and searches the search_string in the raw_monitoring_data_textedit. If found, the search_string is selected and the cursor is moved to the found search_string.
void RawMonitoringDataWidget::updateSearchHighlighting(const QString& search_string)
{
  // Un-select the text from the raw_monitoring_data_textedit
  QTextCursor cursor = ui_.raw_monitoring_data_textedit->textCursor();
  cursor.setPosition(cursor.anchor(), QTextCursor::MoveMode::KeepAnchor);
  ui_.raw_monitoring_data_textedit->setTextCursor(cursor);
  
  if (search_string.isEmpty())
  {
    // If search string is empty, clear the find-highlighting and do nothing
    ui_.raw_monitoring_data_textedit->setExtraSelections({});
    ui_.occurences_label->setText("0 occurences");
    ui_.occurences_label->setEnabled(false);
    return;
  }
  else
  {
    // search in the textedit for all positions of the search string
    QList<QTextEdit::ExtraSelection> extra_selections;
    {
      int index = ui_.raw_monitoring_data_textedit->toPlainText().indexOf(search_string, 0, Qt::CaseSensitivity::CaseInsensitive);
      while (index != -1)
      {
        QTextEdit::ExtraSelection extra_selection;
        extra_selection.cursor = ui_.raw_monitoring_data_textedit->textCursor();
        extra_selection.cursor.setPosition(index);
        extra_selection.cursor.setPosition(index + search_string.length(), QTextCursor::MoveMode::KeepAnchor);
        extra_selection.format.setBackground(QBrush(QColor(243, 168, 29, 128)));
        extra_selections << extra_selection;

        index = ui_.raw_monitoring_data_textedit->toPlainText().indexOf(search_string, index + search_string.length(), Qt::CaseSensitivity::CaseInsensitive);
      }
    }

    ui_.raw_monitoring_data_textedit->setExtraSelections(extra_selections);

    // Set the number of occurences in the label
    ui_.occurences_label->setText(QString::number(extra_selections.size()) + " occurence" + (extra_selections.size() != 1 ? "s" : ""));
    ui_.occurences_label->setEnabled(true);
  }
}

void RawMonitoringDataWidget::searchForward(const QString& search_string)
{
  if (search_string.isEmpty())
  {
    // if search string is empty, do nothing
    return;
  }
  else
  {
    const bool found = ui_.raw_monitoring_data_textedit->find(search_string);
    if (!found)
    {
      // if not found, search from the beginning
      const QTextCursor cursor = ui_.raw_monitoring_data_textedit->document()->find(search_string, 0);
      if (!cursor.isNull())
      {
        ui_.raw_monitoring_data_textedit->setTextCursor(cursor);
      }
      else
      {
        // If there are no occurences, flash the label and the search lineedit
        flashSearchBar();
      }
    }
  }
}

void RawMonitoringDataWidget::searchBackward(const QString& search_string)
{
  if (search_string.isEmpty())
  {
    // if search string is empty, do nothing
    return;
  }
  else
  {
    const bool found = ui_.raw_monitoring_data_textedit->find(search_string, QTextDocument::FindFlag::FindBackward);

    if (!found)
    {
      // if not found, search from the end
      const int end_position = ui_.raw_monitoring_data_textedit->document()->characterCount();
      const QTextCursor cursor = ui_.raw_monitoring_data_textedit->document()->find(search_string, end_position, QTextDocument::FindFlag::FindBackward);
      if (!cursor.isNull())
      {
        ui_.raw_monitoring_data_textedit->setTextCursor(cursor);
      }
      else
      {
        // If there are no occurences, flash the label and the search lineedit
        flashSearchBar();
      }
    }
  }
}

void RawMonitoringDataWidget::flashSearchBar()
{
  ui_.occurences_label->setStyleSheet("QLabel { color: red; }");
  ui_.search_lineedit->setStyleSheet("QLineEdit { background-color: red; }");
  QTimer::singleShot(100, this, [this]()
    {
      ui_.occurences_label->setStyleSheet("");
      ui_.search_lineedit->setStyleSheet("");
    });
  QTimer::singleShot(200, this, [this]()
    {
      ui_.occurences_label->setStyleSheet("QLabel { color: red; }");
      ui_.search_lineedit->setStyleSheet("QLineEdit { background-color: red; }");
    });
  QTimer::singleShot(300, this, [this]()
    {
      ui_.occurences_label->setStyleSheet("");
      ui_.search_lineedit->setStyleSheet("");
    });
  QTimer::singleShot(400, this, [this]()
    {
      ui_.occurences_label->setStyleSheet("QLabel { color: red; }");
      ui_.search_lineedit->setStyleSheet("QLineEdit { background-color: red; }");
    });
  QTimer::singleShot(500, this, [this]()
    {
      ui_.occurences_label->setStyleSheet("");
      ui_.search_lineedit->setStyleSheet("");
    });
}

////////////////////////////////////////////
// QWidget overrides (events)
////////////////////////////////////////////

void RawMonitoringDataWidget::keyPressEvent(QKeyEvent* key_event)
{
  if (key_event->matches(QKeySequence::StandardKey::Find))
  {
    ui_.search_lineedit->setFocus();
    ui_.search_lineedit->selectAll();
  }
  else if (key_event->matches(QKeySequence::StandardKey::FindNext))
  {
    ui_.search_lineedit->searchNext();
  }
  else if (key_event->matches(QKeySequence::StandardKey::FindPrevious))
  {
    ui_.search_lineedit->searchPrevious();
  }
  else
  {
    QWidget::keyPressEvent(key_event);
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
