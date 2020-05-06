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

#include "host_picker.h"

#include <QPushButton>
#include <QSet>
#include <QInputDialog>

HostPicker::HostPicker(const QStringList& available_hosts, const QStringList& initial_selection, QWidget *parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  host_list_model_ = new QStandardItemModel(this);

  // Merge available hosts and initial selections
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QSet<QString> host_set = QSet<QString>::fromList(available_hosts + initial_selection);
  QStringList host_stringlist = host_set.toList();
  host_stringlist.sort(Qt::CaseSensitivity::CaseInsensitive);
#else
  QStringList host_list = available_hosts + initial_selection;
  QSet<QString> host_set = QSet<QString>(host_list.begin(), host_list.end());
  QStringList host_stringlist = host_set.values();
#endif

  // Add available hosts
  for (const auto& host : host_stringlist)
  {
    QStandardItem* item = new QStandardItem(host);
    item->setCheckable(true);
    item->setCheckState(initial_selection.contains(host) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    host_list_model_->appendRow(item);
  }

  ui_.host_list->setModel(host_list_model_);

  connect(ui_.add_button, &QAbstractButton::clicked, this, &HostPicker::addButtonClicked);

  connect(host_list_model_, &QAbstractItemModel::dataChanged,  this, &HostPicker::updateOkButton);
  connect(host_list_model_, &QAbstractItemModel::rowsInserted, this, &HostPicker::updateOkButton);


  // Initial state
  updateOkButton();
}

HostPicker::~HostPicker()
{}

QStringList HostPicker::getSelectedHosts()
{
  QStringList selected_hosts;

  for (int i = 0; i < host_list_model_->rowCount(); i++)
  {
    if (host_list_model_->item(i)->checkState() == Qt::CheckState::Checked)
    {
      selected_hosts.push_back(host_list_model_->item(i)->data(Qt::ItemDataRole::DisplayRole).toString());
    }
  }
  return selected_hosts;
}

void HostPicker::updateOkButton()
{
  ui_.button_box->button(QDialogButtonBox::Ok)->setEnabled(!getSelectedHosts().empty());
}

void HostPicker::addButtonClicked()
{
  QString text = QInputDialog::getText(this, tr("Add host"), tr("Hostname:"), QLineEdit::Normal);
  if (!text.isEmpty())
  {
    // Check if the host already exists
    for (int i = 0; i < host_list_model_->rowCount(); i++)
    {
      if (host_list_model_->item(i)->data(Qt::ItemDataRole::DisplayRole).toString() == text)
        return;
    }

    QStandardItem* item = new QStandardItem(text);
    item->setCheckable(true);
    item->setCheckState(Qt::CheckState::Checked);
    host_list_model_->appendRow(item);
  }
}
