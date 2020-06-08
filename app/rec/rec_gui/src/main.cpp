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

#include "ecalrec_gui.h"
#include <QtWidgets/QApplication>
#include <ecal/ecal.h>

#include "qecalrec.h"

int main(int argc, char *argv[])
{

  // Just make sure that eCAL is initialized
  eCAL::Initialize(0, nullptr, "eCALRecGUI", eCAL::Init::Default | eCAL::Init::Service | eCAL::Init::Monitoring);
  eCAL::Monitoring::SetFilterState(false);

  QApplication a(argc, argv);

  a.setOrganizationName      ("Continental");
  a.setOrganizationDomain    ("continental-corporation.com");
  a.setApplicationName       ("ecalrecgui");
  a.setApplicationDisplayName("eCAL Recorder");

  EcalRecGui* w = new EcalRecGui();
  w->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);

  QEcalRec::instance()->setParent(w);

  if (argc > 1)
  {
    std::string config_path = argv[1];
    QEcalRec::instance()->loadConfigFromFile(config_path);
  }

  w->show();
  int return_code = a.exec();

  eCAL::Finalize();

  return return_code;
}
