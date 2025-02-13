/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "EcalrecGuiClient.h"

#include <QTextStream>
#include <QMessageBox>

EcalrecGuiClient::EcalrecGuiClient(QWidget *parent)
  : QMainWindow(parent)
{
  ui_.setupUi(this);

  // initialize eCAL API
  eCAL::Initialize("RecClientServiceGui");

  connect(ui_.hostname_lineedit, &QLineEdit::editingFinished, this, [this]() {hostname_ = ui_.hostname_lineedit->text().toStdString(); });

  connect(ui_.get_config_request_button, &QPushButton::clicked,                 this,                   &EcalrecGuiClient::getConfigRequest);
  connect(ui_.set_config_request_button, &QPushButton::clicked,                 this,                   &EcalrecGuiClient::setConfigRequest);
  connect(ui_.command_request_button,    &QPushButton::clicked,                 this,                   &EcalrecGuiClient::commandRequest);
  connect(ui_.get_state_request_button,  &QPushButton::clicked,                 this,                   &EcalrecGuiClient::getStateRequest);
  connect(ui_.response_clear_button,     &QPushButton::clicked,                 ui_.response_texteedit, &QTextEdit::clear);
  connect(this,                          &EcalrecGuiClient::setResponseSignal,  ui_.response_texteedit, &QTextEdit::setText                   ,Qt::ConnectionType::QueuedConnection);
}

EcalrecGuiClient::~EcalrecGuiClient()
{}

void EcalrecGuiClient::getConfigRequest()
{
  eCAL::pb::rec_client::GetConfigRequest get_config_request;
  callService("GetConfig", get_config_request);
}

void EcalrecGuiClient::setConfigRequest()
{
  eCAL::pb::rec_client::SetConfigRequest set_config_request;
  auto config = set_config_request.mutable_config()->mutable_items();

  if (ui_.set_config_max_pre_buffer_length_secs_checkbox->isChecked())
  {
    (*config)["max_pre_buffer_length_secs"] = ui_.set_config_max_pre_buffer_length_secs_lineedit->text().toStdString();
  }

  if (ui_.set_config_pre_buffering_enabled_checkbox->isChecked())
  {
    (*config)["pre_buffering_enabled"] = ui_.set_config_pre_buffering_enabled_lineedit->text().toStdString();
  }

  if (ui_.set_config_host_filter_checkbox->isChecked())
  {
    (*config)["host_filter"] = ui_.set_config_host_filter_textedit->toPlainText().toStdString();
  }

  if (ui_.set_config_record_mode_checkbox->isChecked())
  {
    (*config)["record_mode"] = ui_.set_config_record_mode_lineedit->text().toStdString();
  }

  if (ui_.set_config_listed_topics_checkbox->isChecked())
  {
    (*config)["listed_topics"] = ui_.set_config_listed_topics_textedit->toPlainText().toStdString();
  }

  if (ui_.set_config_enabled_addons_checkbox->isChecked())
  {
    (*config)["enabled_addons"] = ui_.set_config_enabled_addons_textedit->toPlainText().toStdString();
  }

  callService("SetConfig", set_config_request);
}

void EcalrecGuiClient::commandRequest()
{
  eCAL::pb::rec_client::CommandRequest command_request;

  QString command_string = ui_.command_request_command_combobox->currentText();
  if (command_string == "none")
  {
    command_request.set_command(eCAL::pb::rec_client::CommandRequest_Command::CommandRequest_Command_none);
  }
  else if (command_string == "initialize")
  {
    command_request.set_command(eCAL::pb::rec_client::CommandRequest_Command::CommandRequest_Command_initialize);
  }
  else if (command_string == "de_initialize")
  {
    command_request.set_command(eCAL::pb::rec_client::CommandRequest_Command::CommandRequest_Command_de_initialize);
  }
  else if (command_string == "start_recording")
  {
    command_request.set_command(eCAL::pb::rec_client::CommandRequest_Command::CommandRequest_Command_start_recording);
  }
  else if (command_string == "stop_recording")
  {
    command_request.set_command(eCAL::pb::rec_client::CommandRequest_Command::CommandRequest_Command_stop_recording);
  }
  else if (command_string == "save_pre_buffer")
  {
    command_request.set_command(eCAL::pb::rec_client::CommandRequest_Command::CommandRequest_Command_save_pre_buffer);
  }
  else if (command_string == "upload_measurement")
  {
    command_request.set_command(eCAL::pb::rec_client::CommandRequest_Command::CommandRequest_Command_upload_measurement);
  }
  else if (command_string == "exit")
  {
    command_request.set_command(eCAL::pb::rec_client::CommandRequest_Command::CommandRequest_Command_exit);
  }

  auto command_params = command_request.mutable_command_params()->mutable_items();

  if (ui_.command_request_meas_id_checkbox->isChecked())
  {
    (*command_params)["meas_id"] = ui_.command_request_meas_id_lineedit->text().toStdString();
  }

  if (ui_.command_request_meas_root_dir_checkbox->isChecked())
  {
    (*command_params)["meas_root_dir"] = ui_.command_request_meas_root_dir_lineedit->text().toStdString();
  }

  if (ui_.command_request_meas_name_checkbox->isChecked())
  {
    (*command_params)["meas_name"] = ui_.command_request_meas_name_lineedit->text().toStdString();
  }

  if (ui_.command_request_description_checkbox->isChecked())
  {
    (*command_params)["description"] = ui_.command_request_description_textedit->toPlainText().toStdString();
  }

  if (ui_.command_request_max_file_size_mib_checkbox->isChecked())
  {
    (*command_params)["max_file_size_mib"] = ui_.command_request_max_file_size_mib_lineedit->text().toStdString();
  }

  if (ui_.command_one_file_per_topic_checkbox->isChecked())
  {
    (*command_params)["one_file_per_topic"] = ui_.command_one_file_per_topic_lineedit->text().toStdString();
  }

  if (ui_.command_request_protocol_checkbox->isChecked())
  {
    (*command_params)["protocol"] = ui_.command_request_protocol_lineedit->text().toStdString();
  }

  if (ui_.command_request_username_checkbox->isChecked())
  {
    (*command_params)["username"] = ui_.command_request_username_lineedit->text().toStdString();
  }

  if (ui_.command_request_password_checkbox->isChecked())
  {
    (*command_params)["password"] = ui_.command_request_password_lineedit->text().toStdString();
  }

  if (ui_.command_request_host_checkbox->isChecked())
  {
    (*command_params)["host"] = ui_.command_request_host_lineedit->text().toStdString();
  }

  if (ui_.command_request_port_checkbox->isChecked())
  {
    (*command_params)["port"] = ui_.command_request_port_lineedit->text().toStdString();
  }

  if (ui_.command_request_upload_path_checkbox->isChecked())
  {
    (*command_params)["upload_path"] = ui_.command_request_upload_path_lineedit->text().toStdString();
  }

  if (ui_.command_request_upload_metadata_files_checkbox->isChecked())
  {
    (*command_params)["upload_metadata_files"] = ui_.command_request_upload_metadata_files_lineedit->text().toStdString();
  }

  if (ui_.command_request_delete_after_upload_checkbox->isChecked())
  {
    (*command_params)["delete_after_upload"] = ui_.command_request_delete_after_upload_lineedit->text().toStdString();
  }

  callService("SetCommand", command_request);
}

void EcalrecGuiClient::getStateRequest()
{
  eCAL::pb::rec_client::GetStateRequest get_state_request;
  callService("GetState", get_state_request);
}

////////////////////////////////////////////////////////////////////////////////
//// Request                                                                ////
////////////////////////////////////////////////////////////////////////////////
template <typename RequestT>
void EcalrecGuiClient::callService(const std::string& method, const RequestT& request)
{
  auto client_instances = recorder_service_.GetClientInstances();
  for (auto& client_instance : client_instances)
  {
    // TODO: We need to filter for pid as well in the future?
    // Currently empty hostname means "all hosts"
    if ((client_instance.GetClientID().host_name == hostname_) || hostname_.empty())
    {
      client_instance.CallWithCallback(method, request, [this](const eCAL::SServiceResponse& service_response) {this->onRecorderResponse(service_response); });
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//// Response                                                               ////
////////////////////////////////////////////////////////////////////////////////

void EcalrecGuiClient::onRecorderResponse(const eCAL::SServiceResponse& service_response_)
{
  QString response_string;
  QTextStream response_stream(&response_string);

  const std::string& method_name = service_response_.service_method_information.method_name;
  const std::string& host_name   = service_response_.server_id.service_id.host_name;

  switch (service_response_.call_state)
  {
  // service successful executed
  case eCAL::eCallState::executed:
  {
    if (method_name == "GetConfig")
    {
      eCAL::pb::rec_client::GetConfigResponse response;
      response.ParseFromString(service_response_.response);

      response_stream << "RecorderService " << method_name.c_str() << " called successfully on host " << host_name.c_str() << "\n";
      response_stream << "------------------------------------------------\n\n";
      response_stream << response.DebugString().c_str();
    }
    else if (method_name == "GetState")
    {
      eCAL::pb::rec_client::State response;
      response.ParseFromString(service_response_.response);

      response_stream << "RecorderService " << method_name.c_str() << " called successfully on host " << host_name.c_str() << "\n";
      response_stream << "------------------------------------------------\n\n";
      response_stream << response.DebugString().c_str();
    }
    else
    {
      eCAL::pb::rec_client::Response response;
      response.ParseFromString(service_response_.response);
      response_stream << "RecorderService " << method_name.c_str() << " called successfully on host " << host_name.c_str() << "\n";
      response_stream << "------------------------------------------------\n\n";
      response_stream << response.DebugString().c_str();
    }
    break;
  }
  // service execution failed
  case eCAL::eCallState::failed:
  {
    eCAL::pb::rec_client::Response response;
    response.ParseFromString(service_response_.response);
    response_stream << "RecorderService " << method_name.c_str() << " failed with \"" << response.error().c_str() << "\" on host " << host_name.c_str() << "\n";
    response_stream << "------------------------------------------------\n\n";
    response_stream << response.DebugString().c_str();
    break;
  }
  default:
    break;
  }
  
  emit setResponseSignal(response_string);

}

