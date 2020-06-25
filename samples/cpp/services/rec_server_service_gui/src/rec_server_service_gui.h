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

#include <QtWidgets/QMainWindow>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/client.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100 4505 4800)
#endif
#include <ecal/pb/rec/server_service.pb.h>
#include <ecal/pb/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ui_rec_server_service_gui.h"

class RecServerServiceGui : public QMainWindow
{
  Q_OBJECT

public:
  RecServerServiceGui(QWidget *parent = Q_NULLPTR);
  ~RecServerServiceGui();

private slots:
  void sendRequest();

  void getStatus();
  void loadConfigFile();
  void activate();
  void deActivate();
  void startRecording();
  void stopRecording();
  void saveBuffer();
  void uploadMeasurement();
  void deleteMeasurement();
  void addComment();

signals:
  void setResponseSignal(QString response);

private:
  void onRecorderResponse(const struct eCAL::SServiceInfo& service_info_, const std::string& response_);






private:
  Ui::RecServerServiceGui ui_;

  eCAL::protobuf::CServiceClient<eCAL::pb::rec_server::EcalRecServerService> recorder_service_;
};
