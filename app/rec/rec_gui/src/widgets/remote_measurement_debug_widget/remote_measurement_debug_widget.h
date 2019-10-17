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
#include "ui_remote_measurement_debug_widget.h"

#include <set>
#include <string>

class RemoteMeasurementDebugWidget : public QWidget
{
  Q_OBJECT

public:
  RemoteMeasurementDebugWidget(QWidget *parent = Q_NULLPTR);
  ~RemoteMeasurementDebugWidget();

private slots:
  void updateTextView();

private:
  Ui::RemoteMeasurementDebugWidget ui_;

  QTimer* update_timer_;
};
