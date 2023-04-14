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

#include <QWidget>
#include "ui_raw_monitoring_data_widget.h"

// Include the monitoring pb header
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/monitoring.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


class RawMonitoringDataWidget : public QWidget
{
  Q_OBJECT

public:
  RawMonitoringDataWidget(QWidget *parent = Q_NULLPTR);
  ~RawMonitoringDataWidget();

public slots:
  void setRawMonitoringData(const eCAL::pb::Monitoring& monitoring_data);
  void updateRawMonitoringData();

private slots:
  void saveToFile();

private:
  Ui::RawMonitoringDataWidget ui_;
};
