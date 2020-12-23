/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include <vector>

#include <QAbstractItemModel>

#include <ecal/ecal.h>

class SeverityLevelModel : public QAbstractListModel
{
  Q_OBJECT

public:
  SeverityLevelModel(QObject *parent = nullptr);
  ~SeverityLevelModel();

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

  QString getString(int row) const;
  eCAL_Process_eSeverity_Level getSeverityLevel(int row) const;

  int getRow(eCAL_Process_eSeverity_Level level) const;

private:
  std::vector<std::pair<QString, eCAL_Process_eSeverity_Level>> rows;
};