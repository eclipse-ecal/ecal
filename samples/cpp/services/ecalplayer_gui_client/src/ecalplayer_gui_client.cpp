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

#include "ecalplayer_gui_client.h"

#include <QTextStream>
#include <QMessageBox>

EcalplayGuiClient::EcalplayGuiClient(QWidget *parent)
  : QMainWindow(parent)
{
  ui_.setupUi(this);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "ecalplayer gui client");

  // create player service client
  player_service_.AddResponseCallback([this](const struct eCAL::SServiceResponse& service_response) {this->onPlayerResponse(service_response); });

  connect(ui_.get_config_request_button, &QPushButton::clicked,                 this,                   &EcalplayGuiClient::getConfigRequest);
  connect(ui_.set_config_request_button, &QPushButton::clicked,                 this,                   &EcalplayGuiClient::setConfigRequest);
  connect(ui_.command_request_button,    &QPushButton::clicked,                 this,                   &EcalplayGuiClient::commandRequest);
  connect(ui_.response_clear_button,     &QPushButton::clicked,                 ui_.response_texteedit, &QTextEdit::clear);
  connect(this,                          &EcalplayGuiClient::setResponseSignal, ui_.response_texteedit, &QTextEdit::setText                   ,Qt::ConnectionType::QueuedConnection);
}

EcalplayGuiClient::~EcalplayGuiClient()
{}

////////////////////////////////////////////////////////////////////////////////
//// Request                                                                ////
////////////////////////////////////////////////////////////////////////////////

void EcalplayGuiClient::getConfigRequest()
{
  eCAL::pb::play::GetConfigRequest get_config_request;
  player_service_.Call("GetConfig", get_config_request);
}

void EcalplayGuiClient::setConfigRequest()
{
  eCAL::pb::play::SetConfigRequest set_config_request;
  auto config = set_config_request.mutable_config()->mutable_items();

  if (ui_.set_config_measurement_path_checkbox->isChecked())
  {
    (*config)["measurement_path"] = ui_.set_config_measurement_path_lineedit->text().toStdString();
  }

  if (ui_.set_config_limit_play_speed_checkbox->isChecked())
  {
    (*config)["limit_play_speed"] = ui_.set_config_limit_play_speed_lineedit->text().toStdString();
  }

  if (ui_.set_config_play_speed_checkbox->isChecked())
  {
    (*config)["play_speed"] = ui_.set_config_play_speed_lineedit->text().toStdString();
  }

  if (ui_.set_config_frame_dropping_allowed_checkbox->isChecked())
  {
    (*config)["frame_dropping_allowed"] = ui_.set_config_frame_dropping_allowed_lineedit->text().toStdString();
  }

  if (ui_.set_config_enforce_delay_accuracy_checkbox->isChecked())
  {
    (*config)["enforce_delay_accuracy"] = ui_.set_config_enforce_delay_accuracy_lineedit->text().toStdString();
  }

  if (ui_.set_config_repeat_checkbox->isChecked())
  {
    (*config)["repeat"] = ui_.set_config_repeat_lineedit->text().toStdString();
  }

  if (ui_.set_config_limit_interval_start_rel_secs_checkbox->isChecked())
  {
    (*config)["limit_interval_start_rel_secs"] = ui_.set_config_limit_interval_start_rel_secs_lineedit->text().toStdString();
  }

  if (ui_.set_config_limit_interval_end_rel_secs_checkbox->isChecked())
  {
    (*config)["limit_interval_end_rel_secs"] = ui_.set_config_limit_interval_end_rel_secs_lineedit->text().toStdString();
  }

  player_service_.Call("SetConfig", set_config_request);
}

void EcalplayGuiClient::commandRequest()
{
  eCAL::pb::play::CommandRequest command_request;

  QString command_string = ui_.command_request_command_combobox->currentText();
  if (command_string == "none")
  {
    command_request.set_command(eCAL::pb::play::CommandRequest_eCommand::CommandRequest_eCommand_none);
  }
  else if (command_string == "initialize")
  {
    command_request.set_command(eCAL::pb::play::CommandRequest_eCommand::CommandRequest_eCommand_initialize);
  }
  else if (command_string == "de_initialize")
  {
    command_request.set_command(eCAL::pb::play::CommandRequest_eCommand::CommandRequest_eCommand_de_initialize);
  }
  else if (command_string == "jump_to")
  {
    command_request.set_command(eCAL::pb::play::CommandRequest_eCommand::CommandRequest_eCommand_jump_to);
  }
  else if (command_string == "play")
  {
    command_request.set_command(eCAL::pb::play::CommandRequest_eCommand::CommandRequest_eCommand_play);
  }
  else if (command_string == "pause")
  {
    command_request.set_command(eCAL::pb::play::CommandRequest_eCommand::CommandRequest_eCommand_pause);
  }
  else if (command_string == "step")
  {
    command_request.set_command(eCAL::pb::play::CommandRequest_eCommand::CommandRequest_eCommand_step);
  }
  else if (command_string == "step_channel")
  {
    command_request.set_command(eCAL::pb::play::CommandRequest_eCommand::CommandRequest_eCommand_step_channel);
  }
  else if (command_string == "exit")
  {
    command_request.set_command(eCAL::pb::play::CommandRequest_eCommand::CommandRequest_eCommand_exit);
  }
  else
  {
    QMessageBox error_message(
      QMessageBox::Icon::Critical
      , tr("Error")
      , tr("Unknown command: ") + command_string
      , QMessageBox::Button::Ok
      , this);
    error_message.exec();

    return;
  }

  if (ui_.command_request_channel_mapping_checkbox->isChecked())
  {
    auto channel_map = command_request.mutable_channel_mapping();
    
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList lines = ui_.command_request_channel_mapping_textedit->toPlainText().split("\n", Qt::SplitBehaviorFlags::SkipEmptyParts);
#else // QT_VERSION
    QStringList lines = ui_.command_request_channel_mapping_textedit->toPlainText().split("\n", QString::SplitBehavior::SkipEmptyParts);
#endif // QT_VERSION
    for (auto& line : lines)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
      QStringList mapping_list = line.split("=", Qt::SplitBehaviorFlags::SkipEmptyParts);
#else // QT_VERSION
      QStringList mapping_list = line.split("=", QString::SplitBehavior::SkipEmptyParts);
#endif // QT_VERSION
      if (mapping_list.size() == 2)
      {
        (*channel_map)[mapping_list.at(0).trimmed().toStdString()] = mapping_list.at(1).trimmed().toStdString();
      }
    }
  }

  if (ui_.command_request_step_reference_channel_checkbox->isChecked())
  {
    command_request.set_step_reference_channel(ui_.command_request_step_reference_channel_lineedit->text().toStdString());
  }

  if (ui_.command_request_rel_time_secs_checkbox->isChecked())
  {
    command_request.set_rel_time_secs(ui_.command_request_rel_time_secs_spinbox->value());
  }

  player_service_.Call("SetCommand", command_request);
}


////////////////////////////////////////////////////////////////////////////////
//// Response                                                               ////
////////////////////////////////////////////////////////////////////////////////

void EcalplayGuiClient::onPlayerResponse(const struct eCAL::SServiceResponse& service_response_)
{
  QString response_string;
  QTextStream response_stream(&response_string);

  switch (service_response_.call_state)
  {
    // service successful executed
  case call_state_executed:
  {
    if (service_response_.method_name == "GetConfig")
    {
      eCAL::pb::play::GetConfigResponse response;
      response.ParseFromString(service_response_.response);

      response_stream << "PlayerService " << service_response_.method_name.c_str() << " called successfully on host " << service_response_.host_name.c_str() << "\n";
      response_stream << "------------------------------------------------\n\n";
      response_stream << response.DebugString().c_str();
    }
    else
    {
      eCAL::pb::play::Response response;
      response.ParseFromString(service_response_.response);
      response_stream << "PlayerService " << service_response_.method_name.c_str() << " called successfully on host " << service_response_.host_name.c_str() << "\n";
      response_stream << "------------------------------------------------\n\n";
      response_stream << response.DebugString().c_str();
    }
    break;
  }
  // service execution failed
  case call_state_failed:
  {
    eCAL::pb::play::Response response;
    response.ParseFromString(service_response_.response);
    response_stream << "PlayerService " << service_response_.method_name.c_str() << " failed with \"" << response.error().c_str() << "\" on host " << service_response_.host_name.c_str() << "\n";
    response_stream << "------------------------------------------------\n\n";
    response_stream << response.DebugString().c_str();
    break;
  }
  default:
    break;
  }
  
  emit setResponseSignal(response_string);

}

