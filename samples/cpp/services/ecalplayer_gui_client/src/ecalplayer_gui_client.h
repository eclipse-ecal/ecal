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
#include <ecal/pb/play/service.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "ui_main_window.h"

class EcalplayGuiClient : public QMainWindow
{
  Q_OBJECT

public:
  EcalplayGuiClient(QWidget *parent = Q_NULLPTR);
  ~EcalplayGuiClient();

private slots:
  void getConfigRequest();
  void setConfigRequest();
  void commandRequest();

signals:
  void setResponseSignal(QString response);

private:
  void onPlayerResponse(const struct eCAL::SServiceResponse& service_response_);

private:
  Ui::EcalplayGuiServiceMainWindow ui_;

  eCAL::protobuf::CServiceClient<eCAL::pb::play::EcalPlayService> player_service_;
};
