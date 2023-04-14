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

RawMonitoringDataWidget::RawMonitoringDataWidget(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);

  connect(ui_.update_button,       &QPushButton::clicked, this, &RawMonitoringDataWidget::updateRawMonitoringData);
  connect(ui_.save_to_file_button, &QPushButton::clicked, this, &RawMonitoringDataWidget::saveToFile);
}

RawMonitoringDataWidget::~RawMonitoringDataWidget()
{}

void RawMonitoringDataWidget::setRawMonitoringData(const eCAL::pb::Monitoring& monitoring_pb)
{
  ui_.raw_monitoring_data_textedit->setPlainText(QString::fromStdString(monitoring_pb.DebugString()));
  ui_.save_to_file_button->setEnabled(true);
}

void RawMonitoringDataWidget::updateRawMonitoringData()
{
  std::string          monitoring_string;
  eCAL::pb::Monitoring monitoring_pb;

  if (eCAL::Monitoring::GetMonitoring(monitoring_string) && !monitoring_string.empty() && monitoring_pb.ParseFromString(monitoring_string))
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
