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
#include "ecal_play_globals.h"

#include <ecal/ecal.h>

#include <QPushButton>
#include <QtGlobal>

AboutDialog::AboutDialog(QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);
  ui_.version_label->setText("Version: " + QString(EcalPlayGlobals::VERSION_STRING));
  ui_.ecalversion_label->setText("eCAL " + QString(ECAL_VERSION) + " (" + QString(ECAL_DATE) + ")");

  ui_.ecal_runtime_version_string_label->setText(QString::fromStdString(eCAL::GetVersionString()) + " (" + QString::fromStdString(eCAL::GetVersionDateString()) + ")");
  ui_.ecal_compiletime_versin_string_label->setText(QString(ECAL_VERSION) + " (" + QString(ECAL_DATE) + ")");
  ui_.qt_runtime_version_string_label->setText(QString(qVersion()));
  ui_.qt_compiletime_version_string_label->setText(QString(QT_VERSION_STR));

  connect(ui_.button_box->button(QDialogButtonBox::StandardButton::Ok), SIGNAL(clicked()), this, SLOT(close()));
}

AboutDialog::~AboutDialog()
{
}