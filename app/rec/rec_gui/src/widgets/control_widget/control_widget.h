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

#include <QResizeEvent>

#include <chrono>
#include "ui_control_widget.h"

class ControlWidget : public QWidget
{
  Q_OBJECT

public:
  ControlWidget(QWidget *parent = Q_NULLPTR);
  ~ControlWidget();

protected:
  void resizeEvent(QResizeEvent *event);

private slots:
  void updateRecordButton();
  void updateSaveBufferButton();
  void updateActivateButton();

private:
  Ui::ControlWidget ui_;

  bool first_resize_event_;

  bool record_button_state_is_record_;
  bool activate_button_state_is_activate_;
};
