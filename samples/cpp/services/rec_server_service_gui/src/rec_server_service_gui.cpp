/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2018 Continental Corporation
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

#include "rec_server_service_gui.h"

#include <QTextStream>
#include <QMessageBox>
#include <stdint.h>

RecServerServiceGui::RecServerServiceGui(QWidget *parent)
  : QMainWindow(parent)
{
  ui_.setupUi(this);

  // initialize eCAL API
  eCAL::Initialize(0, nullptr, "RecServerServiceGui");

  // create player service client
  recorder_service_.AddResponseCallback([this](const struct eCAL::SServiceInfo& service_info, const std::string& response) {this->onRecorderResponse(service_info, response); });

  connect(ui_.hostname_lineedit, &QLineEdit::editingFinished, this, [this]() {recorder_service_.SetHostName(ui_.hostname_lineedit->text().toStdString()); });

  connect(ui_.send_request_button,       &QPushButton::clicked,                 this,                   &RecServerServiceGui::sendRequest);
  connect(ui_.response_clear_button,     &QPushButton::clicked,                 ui_.response_texteedit, &QTextEdit::clear);
  connect(this,                          &RecServerServiceGui::setResponseSignal,  ui_.response_texteedit, &QTextEdit::setText                   ,Qt::ConnectionType::QueuedConnection);
}

RecServerServiceGui::~RecServerServiceGui()
{}

////////////////////////////////////////////////////////////////////////////////
//// Request                                                                ////
////////////////////////////////////////////////////////////////////////////////

void RecServerServiceGui::sendRequest()
{
  if (ui_.request_get_status_radiobutton->isChecked())              getStatus();
  else if (ui_.request_load_config_file_radiobutton->isChecked())   loadConfigFile();
  else if (ui_.request_activate_radiobutton->isChecked())           activate();
  else if (ui_.request_deactivate_radiobutton->isChecked())         deActivate();
  else if (ui_.request_start_recording_radiobutton->isChecked())    startRecording();
  else if (ui_.request_stop_recording_radiobutton->isChecked())     stopRecording();
  else if (ui_.request_save_buffer_radiobutton->isChecked())        saveBuffer();
  else if (ui_.request_upload_measurement_radiobutton->isChecked()) uploadMeasurement();
  else if (ui_.request_delete_measurement_radiobutton->isChecked()) deleteMeasurement();
  else if (ui_.request_add_comment_radiobutton->isChecked())        addComment();
}

void RecServerServiceGui::getStatus()
{
  eCAL::pb::rec_server::GenericRequest request;
  bool success = recorder_service_.Call("GetStatus", request);

  if (!success)
    ui_.response_texteedit->setText("Unable to call service");
}

void RecServerServiceGui::loadConfigFile()
{
  eCAL::pb::rec_server::LoadConfigRequest request;
  request.set_config_path(ui_.request_load_config_file_param_config_path_lineedit->text().toStdString());
  bool success = recorder_service_.Call("LoadConfigFile", request);

  if (!success)
    ui_.response_texteedit->setText("Unable to call service");
}

void RecServerServiceGui::activate()
{
  eCAL::pb::rec_server::GenericRequest request;
  bool success = recorder_service_.Call("Activate", request);

  if (!success)
    ui_.response_texteedit->setText("Unable to call service");
}

void RecServerServiceGui::deActivate()
{
  eCAL::pb::rec_server::GenericRequest request;
  bool success = recorder_service_.Call("DeActivate", request);

  if (!success)
    ui_.response_texteedit->setText("Unable to call service");
}

void RecServerServiceGui::startRecording()
{
  eCAL::pb::rec_server::GenericRequest request;
  bool success = recorder_service_.Call("StartRecording", request);

  if (!success)
    ui_.response_texteedit->setText("Unable to call service");
}

void RecServerServiceGui::stopRecording()
{
  eCAL::pb::rec_server::GenericRequest request;
  bool success = recorder_service_.Call("StopRecording", request);

  if (!success)
    ui_.response_texteedit->setText("Unable to call service");
}

void RecServerServiceGui::saveBuffer()
{
  eCAL::pb::rec_server::GenericRequest request;
  bool success = recorder_service_.Call("SaveBuffer", request);

  if (!success)
    ui_.response_texteedit->setText("Unable to call service");
}

void RecServerServiceGui::uploadMeasurement()
{
  eCAL::pb::rec_server::GenericMeasurementRequest request;

  if(!ui_.request_upload_measurement_param_meas_id_lineedit->text().isEmpty())
  {
    try
    {
      int64_t meas_id = std::stoll(ui_.request_upload_measurement_param_meas_id_lineedit->text().toStdString());
      request.set_meas_id(meas_id);      
    }
    catch(...)
    {
      QMessageBox error_message;
      error_message.setIcon       (QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText       ("Unable to parse meas_id");
      error_message.exec();

      return;
    }
  }

  bool success = recorder_service_.Call("UploadMeasurement", request);

  if (!success)
    ui_.response_texteedit->setText("Unable to call service");
}

void RecServerServiceGui::deleteMeasurement()
{
  eCAL::pb::rec_server::GenericMeasurementRequest request;

  if(!ui_.request_delete_measurement_lineedit->text().isEmpty())
  {
    try
    {
      int64_t meas_id = std::stoll(ui_.request_delete_measurement_lineedit->text().toStdString());
      request.set_meas_id(meas_id);      
    }
    catch(...)
    {
      QMessageBox error_message;
      error_message.setIcon       (QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText       ("Unable to parse meas_id");
      error_message.exec();

      return;
    }
  }

  bool success = recorder_service_.Call("DeleteMeasurement", request);

  if (!success)
    ui_.response_texteedit->setText("Unable to call service");
}

void RecServerServiceGui::addComment()
{
  eCAL::pb::rec_server::AddCommentRequest request;

  if(!ui_.request_add_comment_param_meas_id_lineedit->text().isEmpty())
  {
    try
    {
      int64_t meas_id = std::stoll(ui_.request_add_comment_param_meas_id_lineedit->text().toStdString());
      request.set_meas_id(meas_id);      
    }
    catch(...)
    {
      QMessageBox error_message;
      error_message.setIcon       (QMessageBox::Icon::Critical);
      error_message.setWindowTitle("Error");
      error_message.setText       ("Unable to parse meas_id");
      error_message.exec();

      return;
    }
  }

  request.set_comment(ui_.request_add_comment_param_comment_textedit->toPlainText().toStdString());

  bool success = recorder_service_.Call("AddComment", request);

  if (!success)
    ui_.response_texteedit->setText("Unable to call service");
}

////////////////////////////////////////////////////////////////////////////////
//// Response                                                               ////
////////////////////////////////////////////////////////////////////////////////

void RecServerServiceGui::onRecorderResponse(const struct eCAL::SServiceInfo& service_info_, const std::string& response_)
{
  QString     response_string;
  QTextStream response_stream(&response_string);

  switch (service_info_.call_state)
  {

  // service successful executed
  case call_state_executed:
  {
    if ((service_info_.method_name == "StartRecording")
      || (service_info_.method_name == "SaveBuffer"))
    {
      eCAL::pb::rec_server::JobStartedResponse response;
      response.ParseFromString(response_);

      response_stream << "RecorderService " << service_info_.method_name.c_str() << " called successfully on host " << service_info_.host_name.c_str() << "\n";
      response_stream << "------------------------------------------------\n\n";
      response_stream << response.DebugString().c_str();
    }
    else if (service_info_.method_name == "GetStatus")
    {
      eCAL::pb::rec_server::Status response;
      response.ParseFromString(response_);

      response_stream << "RecorderService " << service_info_.method_name.c_str() << " called successfully on host " << service_info_.host_name.c_str() << "\n";
      response_stream << "------------------------------------------------\n\n";
      response_stream << response.DebugString().c_str();
    }
    else
    {
      eCAL::pb::rec_server::ServiceResult response;
      response.ParseFromString(response_);
      response_stream << "RecorderService " << service_info_.method_name.c_str() << " called successfully on host " << service_info_.host_name.c_str() << "\n";
      response_stream << "------------------------------------------------\n\n";
      response_stream << response.DebugString().c_str();
    }
    break;
  }

  // service execution failed
  case call_state_failed:
  {
    //eCAL::pb::Response response;
    //response.ParseFromString(response_);
    //response_stream << "RecorderService " << service_info_.method_name.c_str() << " failed with \"" << response.error().c_str() << "\" on host " << service_info_.host_name.c_str() << "\n";
    //response_stream << "------------------------------------------------\n\n";
    //response_stream << response.DebugString().c_str();
    response_stream << "Service call failed.\n";
    break;
  }
  default:
    break;
  }
  
  emit setResponseSignal(response_string);

}

