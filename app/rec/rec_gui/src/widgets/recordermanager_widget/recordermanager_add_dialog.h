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

#include <QDialog>
#include "ui_recordermanager_add_dialog.h"

#include <QStringList>
#include <QStandardItemModel>

class RecorderManagerAddDialog : public QDialog
{
  Q_OBJECT

public:
  RecorderManagerAddDialog(const QStringList& existing_hosts, bool initially_add_host, const QString& intial_host_selection, QWidget *parent = Q_NULLPTR);
  ~RecorderManagerAddDialog();

public:
  bool addHostSelected() const;
  bool addAddonSelected() const;
  QString newHostName() const;
  std::pair<QString, QString> newAddon() const;

private slots:
  void updateHostOrAddonSelection();

private:
  Ui::RecorderManagerAddDialog ui_;
};
