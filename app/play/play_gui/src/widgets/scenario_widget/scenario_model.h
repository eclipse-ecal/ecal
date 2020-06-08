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

#include <QAbstractItemModel>

#include <ecal/ecal_time.h>

#include <ecal_play_scenario.h>

class ScenarioModel : public QAbstractItemModel
{
  Q_OBJECT

////////////////////////////////////
// Constructor & Destructor
////////////////////////////////////
public:
  ScenarioModel(QObject* parent = nullptr);
  ~ScenarioModel();

////////////////////////////////////
// QAbstractItemModel overrides
////////////////////////////////////
public:
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;

////////////////////////////////////
// API
////////////////////////////////////
public:
  void setScenarios(const std::vector<EcalPlayScenario>& scenarios, const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& measurement_boundaries);
  std::vector<EcalPlayScenario> getScenarios() const;

  const EcalPlayScenario& scenario(int row) const;
  int addScenario(const EcalPlayScenario& scenario);
  void removeScenario(int row_to_remove);

public slots:
  void triggerRepaint(const QModelIndex& index);

////////////////////////////////////
// Column data
////////////////////////////////////
public:
  enum class Column
  {
    SCENARIO_NAME,
    SCENARIO_TIMESTAMP,

    COLUMN_COUNT
  };

private:
  const std::map<int, QString> header_data
  {
    { (int)Column::SCENARIO_NAME,      "Label" },
    { (int)Column::SCENARIO_TIMESTAMP, "Timestamp" },
  };

////////////////////////////////////
// Member variables
////////////////////////////////////
private:
  std::vector<EcalPlayScenario> scenarios_;
  std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point> measurement_boundaries_;
};
