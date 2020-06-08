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

#include "recordermanager_add_dialog.h"

RecorderManagerAddDialog::RecorderManagerAddDialog(const QStringList& existing_hosts, bool initially_add_host, const QString& intial_host_selection, QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  connect(ui_.new_host_radiobutton,  &QRadioButton::clicked, this, &RecorderManagerAddDialog::updateHostOrAddonSelection);
  connect(ui_.new_addon_radiobutton, &QRadioButton::clicked, this, &RecorderManagerAddDialog::updateHostOrAddonSelection);

  ui_.host_combobox->addItems(existing_hosts);
  ui_.host_combobox->setCurrentText(intial_host_selection);

  if (!initially_add_host && !existing_hosts.empty())
    ui_.new_addon_radiobutton->setChecked(true);
  else
    ui_.new_host_radiobutton->setChecked(true);

  if (existing_hosts.empty())
    ui_.new_addon_radiobutton->setEnabled(false);

  updateHostOrAddonSelection();
}

RecorderManagerAddDialog::~RecorderManagerAddDialog()
{}

bool RecorderManagerAddDialog::addHostSelected() const
{
  return ui_.new_host_radiobutton->isChecked();
}

bool RecorderManagerAddDialog::addAddonSelected() const
{
  return ui_.new_addon_radiobutton->isChecked();
}

QString RecorderManagerAddDialog::newHostName() const
{
  return ui_.hostname_lineedit->text();
}

std::pair<QString, QString> RecorderManagerAddDialog::newAddon() const
{
  return std::make_pair(ui_.host_combobox->currentText(), ui_.addon_id_lineedit->text());
}

void RecorderManagerAddDialog::updateHostOrAddonSelection()
{
  if (ui_.new_host_radiobutton->isChecked())
  {
    ui_.hostname_lineedit->setEnabled(true);
    ui_.addon_widget->setEnabled(false);

    ui_.hostname_lineedit->setFocus();
    ui_.hostname_lineedit->selectAll();
  }
  else
  {
    ui_.hostname_lineedit->setEnabled(false);
    ui_.addon_widget->setEnabled(true);

    ui_.addon_id_lineedit->setFocus();
    ui_.addon_id_lineedit->selectAll();
  }
}