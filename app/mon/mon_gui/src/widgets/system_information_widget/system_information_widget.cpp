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

#include "system_information_widget.h"

#include <ecal/ecal.h>

SystemInformationWidget::SystemInformationWidget(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);
  
  std::string config_dump;
  eCAL::Process::DumpConfig(config_dump);

  ui_.system_information_text->document()->setPlainText(config_dump.c_str());

  ui_.system_information_text->moveCursor(QTextCursor::Start);
  ui_.system_information_text->ensureCursorVisible();
}

SystemInformationWidget::~SystemInformationWidget()
{
}
