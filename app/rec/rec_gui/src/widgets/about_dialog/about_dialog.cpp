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

#include "about_dialog.h"
#include "rec_client_core/ecal_rec_defs.h"

#include <ecal/ecal_defs.h>

#include <QPushButton>

AboutDialog::AboutDialog(QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);
  ui_.version_label->setText("Version: " + QString(ECAL_REC_VERSION_STRING));
  ui_.ecalversion_label->setText("eCAL " + QString(ECAL_VERSION) + " (" + QString(ECAL_DATE) + ")");
  connect(ui_.button_box->button(QDialogButtonBox::StandardButton::Ok), SIGNAL(clicked()), this, SLOT(close()));
}

AboutDialog::~AboutDialog()
{
}