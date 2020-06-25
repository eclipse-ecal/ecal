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

#include <sstream>
#include <iomanip>

#include <widgets/wait_for_shutdown_dialog/wait_for_shutdown_dialog.h>

RemoteMeasurementDebugWidget::RemoteMeasurementDebugWidget(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);

  update_timer_ = new QTimer(this);
  connect(update_timer_, &QTimer::timeout, this, &RemoteMeasurementDebugWidget::updateTextView);
  update_timer_->start(100);

  connect(ui_.host_add_button, &QAbstractButton::clicked, this,
      [this]()
      {
        const std::string hostname = ui_.hostname_lineedit->text().toStdString();

        auto enabled_rec_clients = QEcalRec::instance()->enabledRecClients();
        eCAL::rec_server::ClientConfig client_settings;
        enabled_rec_clients.emplace(hostname, client_settings);

        QEcalRec::instance()->setEnabledRecClients(enabled_rec_clients);
      });
  connect(ui_.host_remove_button, &QAbstractButton::clicked, this,
      [this]()
      {
        const std::string hostname = ui_.hostname_lineedit->text().toStdString();
        auto enabled_recorders = QEcalRec::instance()->enabledRecClients();
        auto rec_it = enabled_recorders.find(hostname);
        if (rec_it != enabled_recorders.end())
        {
          enabled_recorders.erase(rec_it);
          QEcalRec::instance()->setEnabledRecClients(enabled_recorders);
        }
      });

  connect(ui_.addon_add_button, &QAbstractButton::clicked, this,
      [this]()
      {
        const std::string hostname   = ui_.hostname_lineedit->text().toStdString();
        const std::string addon_id   = ui_.addon_id_lineedit->text().toStdString();

        auto enabled_recorders = QEcalRec::instance()->enabledRecClients();
        auto rec_it = enabled_recorders.find(hostname);
        if (rec_it != enabled_recorders.end())
        {
          auto addon_it = rec_it->second.enabled_addons_.find(addon_id);
          if (addon_it == rec_it->second.enabled_addons_.end())
          {
            rec_it->second.enabled_addons_.emplace(addon_id);
          }
          QEcalRec::instance()->setEnabledRecClients(enabled_recorders);
        }
      });

  connect(ui_.addon_remove_button, &QAbstractButton::clicked, this,
      [this]()
      {
        const std::string hostname   = ui_.hostname_lineedit->text().toStdString();
        const std::string addon_name = ui_.addon_id_lineedit->text().toStdString();
        auto enabled_recorders = QEcalRec::instance()->enabledRecClients();
        auto rec_it = enabled_recorders.find(hostname);
        if (rec_it != enabled_recorders.end())
        {
          auto addon_it = rec_it->second.enabled_addons_.find(addon_name);
          if (addon_it != rec_it->second.enabled_addons_.end())
          {
            rec_it->second.enabled_addons_.erase(addon_it);
          }
          QEcalRec::instance()->setEnabledRecClients(enabled_recorders);
        }
      });

  connect(ui_.activate_connections_button, &QAbstractButton::clicked, this,
      []()
      {
        QEcalRec::instance()->setConnectionToClientsActive(true);
      });

  connect(ui_.deactivate_connections_button, &QAbstractButton::clicked, this,
      []()
      {
        QEcalRec::instance()->setConnectionToClientsActive(false);
      });

  connect(ui_.connect_to_ecal_button, &QAbstractButton::clicked, this,
      []()
      {
        QEcalRec::instance()->connectToEcal();
      });

  connect(ui_.disconnect_from_ecal_button, &QAbstractButton::clicked, this,
      []()
      {
        QEcalRec::instance()->disconnectFromEcal();
      });

  connect(ui_.save_current_buffer_button, &QAbstractButton::clicked, this,
      []()
      {
        QEcalRec::instance()->savePreBufferedData();
      });

  connect(ui_.start_recording_button, &QAbstractButton::clicked, this,
      []()
      {
        QEcalRec::instance()->startRecording();
      });

  connect(ui_.stop_recording_button, &QAbstractButton::clicked, this,
      []()
      {
        QEcalRec::instance()->stopRecording();
      });

  connect(ui_.wait_for_recorder_operations_dialog_button, &QAbstractButton::clicked, this,
      [this]()
      {
        WaitForShutdownDialog dialog(this);

        dialog.exec();
      });

  connect(ui_.merge_measurements_button, &QAbstractButton::clicked, this,
      []()
      {
        if (!QEcalRec::instance()->jobHistory().empty())
          QEcalRec::instance()->uploadMeasurement(QEcalRec::instance()->jobHistory().back().local_evaluated_job_config_.GetJobId());
      });

  connect(ui_.add_comment_button, &QAbstractButton::clicked, this,
      [this]()
      {
        if (!QEcalRec::instance()->jobHistory().empty())
          QEcalRec::instance()->addComment(QEcalRec::instance()->jobHistory().back().local_evaluated_job_config_.GetJobId(), ui_.add_comment_textedit->toPlainText().toStdString());
      });
}

void RemoteMeasurementDebugWidget::updateTextView()
{
  auto recorder_statuses = QEcalRec::instance()->recorderStatuses();
  auto enabled_recorders = QEcalRec::instance()->enabledRecClients();

  std::stringstream content;
  content << std::left;

  for (const auto& recorder : recorder_statuses)
  {
    auto enabled_recorder_it = enabled_recorders.find(recorder.first);
    bool recorder_enabled = (enabled_recorder_it != enabled_recorders.end());

    // Main HDF5 recorder
    {
      // Enabled
      if (recorder_enabled)
        content << "[X] ";
      else
        content << "[ ] ";

      // Name
      content << std::setw(52) << recorder.first;
      content << " | ";

      // Initialized
      content << std::setw(10);
      if (recorder.second.first.initialized_ )
        content << (std::to_string(recorder.second.first.subscribed_topics_.size()) + " Topics");
      else
        content << "Not Init";

      content << " | ";

      // Status
      content << (recorder.second.first.info_.first ? "OK: " : "ERROR: ") << recorder.second.first.info_.second;

      content << std::endl;
    }

    // Addons
    for (const auto& addon_status : recorder.second.first.addon_statuses_)
    {
      // Enabled
      if (recorder_enabled && (enabled_recorder_it->second.enabled_addons_.find(addon_status.addon_id_) != enabled_recorder_it->second.enabled_addons_.end()))
        content << "[X] ";
      else
        content << "[ ] ";

      // Name
      content << std::setw(20) << (" -> " + addon_status.name_);
      content << std::setw(32) << ("(" + addon_status.addon_id_ + ")");
      content << " | ";

      // Initialized
      content << std::setw(10);
      if (addon_status.initialized_ )
        content << "Init";
      else
        content << "Not Init";

      content << " | ";

      // Status
      content << (addon_status.info_.first ? "OK: " : "ERROR: ") << addon_status.info_.second;

      content << std::endl;
    }

    // Non-existing addons
    {
      auto enabled_rec_settings_it = enabled_recorders.find(recorder.first);
      if (enabled_rec_settings_it != enabled_recorders.end())
      {
        for (const std::string& addon_id : enabled_rec_settings_it->second.enabled_addons_)
        {
          auto rec_addon_it = std::find_if(recorder.second.first.addon_statuses_.begin(), recorder.second.first.addon_statuses_.end()
                                        , [&addon_id](const eCAL::rec::RecorderAddonStatus& recorde_addon_status) -> bool
                                          {
                                            return recorde_addon_status.addon_id_ == addon_id;
                                          });

          if (rec_addon_it == recorder.second.first.addon_statuses_.end())
          {
            // enabled
            content << "[X] ";

            // Name
            content << std::setw(20) << (" -> NOT EXISTING");
            content << std::setw(32) << ("(" + addon_id + ")");
            content << " | ";

            // Initialized
            content << std::setw(10);
            content << " ";
            content << " | ";

            // Status
            content << "Not existing";
          }
        }
      }
    }
  }

  QString q_content = QString::fromStdString(content.str());
  if (ui_.instances_textedit->toPlainText() != q_content)
    ui_.instances_textedit->setText(q_content);

}

RemoteMeasurementDebugWidget::~RemoteMeasurementDebugWidget()
{
}
