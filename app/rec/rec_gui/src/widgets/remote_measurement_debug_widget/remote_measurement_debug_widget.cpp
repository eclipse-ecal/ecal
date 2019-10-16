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

#include "remote_measurement_debug_widget.h"

#include <QTimer>

#include <qecalrec.h>

#include <widgets/wait_for_local_recorder_dialog/wait_for_local_recorder_dialog.h>

RemoteMeasurementDebugWidget::RemoteMeasurementDebugWidget(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);

  update_timer_ = new QTimer(this);
  connect(update_timer_, &QTimer::timeout, this, &RemoteMeasurementDebugWidget::updateTextView);
  update_timer_->start(100);

  connect(ui_.add_button, &QAbstractButton::clicked, this,
      [this]()
      {
        const std::string hostname = ui_.hostname_lineedit->text().toStdString();
        QEcalRec::instance()->addRecorderInstance(hostname, {});
      });

  connect(ui_.activate_connections_button, &QAbstractButton::clicked, this,
      [this]()
      {
        QEcalRec::instance()->setClientConnectionsEnabled(true);
      });

  connect(ui_.deactivate_connections_button, &QAbstractButton::clicked, this,
      [this]()
      {
        QEcalRec::instance()->setClientConnectionsEnabled(false);
      });

  connect(ui_.initiate_connection_shutdown_button, &QAbstractButton::clicked, this,
      [this]()
      {
        QEcalRec::instance()->initiateConnectionShutdown();
      });

  connect(ui_.connect_to_ecal_button, &QAbstractButton::clicked, this,
      [this]()
      {
        QEcalRec::instance()->sendRequestConnectToEcal();
      });

  connect(ui_.disconnect_from_ecal_button, &QAbstractButton::clicked, this,
      [this]()
      {
        QEcalRec::instance()->sendRequestDisconnectFromEcal();
      });

  connect(ui_.save_current_buffer_button, &QAbstractButton::clicked, this,
      [this]()
      {
        QEcalRec::instance()->sendRequestSavePreBufferedData();
      });

  connect(ui_.start_recording_button, &QAbstractButton::clicked, this,
      [this]()
      {
        QEcalRec::instance()->sendRequestStartRecording();
      });

  connect(ui_.stop_recording_button, &QAbstractButton::clicked, this,
      [this]()
      {
        QEcalRec::instance()->sendRequestStopRecording();
      });

  connect(ui_.wait_for_recorder_operations_dialog_button, &QAbstractButton::clicked, this,
      [this]()
      {
        WaitForLocalRecorderDialog dialog(this);

        dialog.exec();
      });
}

void RemoteMeasurementDebugWidget::updateTextView()
{
  auto recorder_instances = QEcalRec::instance()->recorderInstances();
  auto responses = QEcalRec::instance()->lastResponses();

  QString content;

  for (const auto& recorder_instance : recorder_instances)
  {
    content += recorder_instance.first.c_str();
    content += ": ";
    auto response_it = responses.find(recorder_instance.first);
    if (response_it == responses.end())
    {
      content += "Pending...";
    }
    else
    {
      content += (response_it->second.first ? "OK" : "ERROR");
      if (!response_it->second.second.empty())
      {
        content += "(" + QString(response_it->second.second.c_str()) + ")";
      }
    }
    content += "\n";
  }

  ui_.instances_textedit->setText(content);
}

RemoteMeasurementDebugWidget::~RemoteMeasurementDebugWidget()
{
}
