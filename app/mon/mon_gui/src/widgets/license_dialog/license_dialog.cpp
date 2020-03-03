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

#include "license_dialog.h"

#include <QPushButton>
#include <QFile>

LicenseDialog::LicenseDialog(QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);
  connect(ui_.button_box->button(QDialogButtonBox::StandardButton::Ok), SIGNAL(clicked()), this, SLOT(close()));

  QFile license_file(":/ecalicons/FLATICON_LICENSE");
  license_file.open(QIODevice::ReadOnly | QIODevice::Text);
  QByteArray license_array = license_file.readAll();
  QString license_string = QString::fromLatin1(license_array.data());

  ui_.label->setText(license_string);
}

LicenseDialog::~LicenseDialog()
{
}