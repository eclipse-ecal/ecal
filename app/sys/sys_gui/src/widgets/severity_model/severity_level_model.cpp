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

#include "severity_level_model.h"

SeverityLevelModel::SeverityLevelModel(QObject *parent)
  : QAbstractListModel(parent)
{
  rows.push_back(std::make_pair(QString("Level 1"), eCAL_Process_eSeverity_Level::proc_sev_level1));
  rows.push_back(std::make_pair(QString("Level 2"), eCAL_Process_eSeverity_Level::proc_sev_level2));
  rows.push_back(std::make_pair(QString("Level 3"), eCAL_Process_eSeverity_Level::proc_sev_level3));
  rows.push_back(std::make_pair(QString("Level 4"), eCAL_Process_eSeverity_Level::proc_sev_level4));
  rows.push_back(std::make_pair(QString("Level 5"), eCAL_Process_eSeverity_Level::proc_sev_level5));
}

SeverityLevelModel::~SeverityLevelModel()
{}

int SeverityLevelModel::rowCount(const QModelIndex& /*parent*/) const
{
  return (int)rows.size();
}

QVariant SeverityLevelModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::ItemDataRole::DisplayRole)
  {
    return rows[index.row()].first;
  }
  return QVariant();
}

QString SeverityLevelModel::getString(int row) const
{
  return rows[row].first;
}

eCAL_Process_eSeverity_Level SeverityLevelModel::getSeverityLevel(int row) const
{
  return rows[row].second;
}

int SeverityLevelModel::getRow(eCAL_Process_eSeverity_Level level) const
{
  for (int i = 0; i < (int)rows.size(); i++)
  {
    if (rows[i].second == level)
      return i;
  }
  return -1;
}
