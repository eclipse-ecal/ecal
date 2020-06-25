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

#include "upload_settings_dialog.h"

#include <QPushButton>
#include <QFile>

UploadSettingsDialog::UploadSettingsDialog(const eCAL::rec_server::UploadConfig& upload_config, QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  connect(ui_.manual_ftp_radiobutton, &QAbstractButton::toggled, ui_.manual_ftp_settings_groupbox, &QWidget::setEnabled);

  switch (upload_config.type_)
  {
  case eCAL::rec_server::UploadConfig::Type::FTP:
    ui_.manual_ftp_radiobutton->setChecked(true);
    break;
  default:
    ui_.internal_ftp_radiobutton->setChecked(true);
    break;
  }

  ui_.hostname_lineedit->setText (QString::fromStdString(upload_config.host_));
  ui_.port_spinbox     ->setValue(upload_config.port_);
  ui_.username_lineedit->setText (QString::fromStdString(upload_config.username_));
  ui_.password_lineedit->setText (QString::fromStdString(upload_config.password_));
  ui_.root_dir_lineedit->setText (QString::fromStdString(upload_config.root_path_));

  ui_.delete_files_checkbox->setChecked(upload_config.delete_after_upload_);
}

UploadSettingsDialog::~UploadSettingsDialog()
{}

eCAL::rec_server::UploadConfig UploadSettingsDialog::uploadConfig() const
{
  eCAL::rec_server::UploadConfig upload_config;

  if (ui_.manual_ftp_radiobutton->isChecked())
    upload_config.type_ = eCAL::rec_server::UploadConfig::Type::FTP;
  else
    upload_config.type_ = eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP;

  upload_config.host_      = ui_.hostname_lineedit->text().toStdString();
  upload_config.port_      = ui_.port_spinbox->value();
  upload_config.username_  = ui_.username_lineedit->text().toStdString();
  upload_config.password_  = ui_.password_lineedit->text().toStdString();
  upload_config.root_path_ = ui_.root_dir_lineedit->text().toStdString();

  upload_config.delete_after_upload_ = ui_.delete_files_checkbox->isChecked();

  return upload_config;
}