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

#include "scenario_model.h"

#include <widgets/models/item_data_roles.h>

ScenarioModel::ScenarioModel(QObject* parent)
  : QAbstractItemModel(parent)
{}

ScenarioModel::~ScenarioModel()
{}

int ScenarioModel::columnCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
    return (int)Column::COLUMN_COUNT;
  else
    return 0;
}

int ScenarioModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid())
    return (int)scenarios_.size();
  else
    return 0;
}

QVariant ScenarioModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Orientation::Horizontal)
    && (role == Qt::ItemDataRole::DisplayRole))
  {
    return header_data.at(section);
  }
  else
  {
    return QVariant::Invalid;
  }
}

QVariant ScenarioModel::data(const QModelIndex &index, int role) const
{
  const Column column = (Column)index.column();
  const int row = index.row();

  if (column == Column::SCENARIO_NAME)
  {
    if ((role == Qt::ItemDataRole::DisplayRole)
      || (role == Qt::ItemDataRole::EditRole))
    {
      return QString::fromStdString(scenarios_[row].name_);
    }
  }

  else if (column == Column::SCENARIO_TIMESTAMP)
  {
    if (role == Qt::ItemDataRole::DisplayRole)
    {
      double seconds_since_start = std::chrono::duration_cast<std::chrono::duration<double>>(scenarios_[row].time_ - measurement_boundaries_.first).count();
      return QString::number(seconds_since_start, 'f', 3);
    }
    else if (role == Qt::ItemDataRole::EditRole)
    {
      double seconds_since_start = std::chrono::duration_cast<std::chrono::duration<double>>(scenarios_[row].time_ - measurement_boundaries_.first).count();
      return seconds_since_start;
    }
    else if (role == ItemDataRoles::SortRole)
    {
      double seconds_since_start = std::chrono::duration_cast<std::chrono::duration<double>>(scenarios_[row].time_ - measurement_boundaries_.first).count();
      return seconds_since_start;
    }
    else if (role == Qt::ItemDataRole::TextAlignmentRole)
    {
      return static_cast<int>(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    }
  }

  // default to display role
  if ((role == ItemDataRoles::SortRole)
    || (role == ItemDataRoles::FilterRole)
    || (role == Qt::ItemDataRole::ToolTipRole))
  {
    return data(index, Qt::ItemDataRole::DisplayRole);
  }

  return QVariant();
}

bool ScenarioModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  const Column column = (Column)index.column();
  const int    row    = index.row();

  if(role == Qt::ItemDataRole::EditRole)
  {
    if (column == Column::SCENARIO_NAME)
    {
      QString new_name = value.toString();
      new_name = new_name.trimmed();

      if (new_name.isEmpty())
        return false;

      scenarios_[row].name_ = new_name.toStdString();
      emit dataChanged(index, index);
      return true;
    }

    if (column == Column::SCENARIO_TIMESTAMP)
    {
      double seconds_since_start = value.toDouble();

      if (seconds_since_start < 0)
        return false;

      eCAL::Time::ecal_clock::time_point new_scenario_timepoint = measurement_boundaries_.first
              + std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::duration<double>(seconds_since_start));

      scenarios_[row].time_ = new_scenario_timepoint;
      emit dataChanged(index, index);
      return true;
    }
  }

  return false;
}

Qt::ItemFlags ScenarioModel::flags(const QModelIndex &index) const
{
  if (index.isValid())
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemNeverHasChildren | Qt::ItemFlag::ItemIsEditable;
  else
    return QAbstractItemModel::flags(index);
}

QModelIndex ScenarioModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
  return createIndex(row, column, nullptr);
}

QModelIndex ScenarioModel::parent(const QModelIndex &/*index*/) const
{
  return QModelIndex();
}

////////////////////////////////////
// API
////////////////////////////////////

void ScenarioModel::setScenarios(const std::vector<EcalPlayScenario>& scenarios, const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& measurement_boundaries)
{
  beginResetModel();
  scenarios_              = scenarios;
  measurement_boundaries_ = measurement_boundaries;
  endResetModel();
}

std::vector<EcalPlayScenario> ScenarioModel::getScenarios() const
{
  return scenarios_;
}

const EcalPlayScenario& ScenarioModel::scenario(int row) const
{
  return scenarios_[row];
}

int ScenarioModel::addScenario(const EcalPlayScenario& scenario)
{
  beginInsertRows(QModelIndex(), static_cast<int>(scenarios_.size()), static_cast<int>(scenarios_.size()));
  scenarios_.push_back(scenario);
  endInsertRows();
  return static_cast<int>(scenarios_.size()) - 1;
}

void ScenarioModel::removeScenario(int row_to_remove)
{
  beginRemoveRows(QModelIndex(), row_to_remove, row_to_remove);
  scenarios_.erase(std::next(scenarios_.begin(), row_to_remove));
  endRemoveRows();
}

void ScenarioModel::triggerRepaint(const QModelIndex& index)
{
  emit dataChanged(index, index, { Qt::ItemDataRole::DisplayRole });
}
