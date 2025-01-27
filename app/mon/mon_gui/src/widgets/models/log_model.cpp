/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include "log_model.h"
#include "util.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/monitoring.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "item_data_roles.h"

#include <ecal/ecal.h>

#include <QFile>
#include <QTextStream>
#include <QDateTime>

LogModel::LogModel(QObject* parent)
  : QAbstractItemModel(parent) 
  , max_entries_(100000)
  , parse_time_enabled_(true)
{}

LogModel::~LogModel()
{}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid())
  {
    int row = index.row();
    Columns column = (Columns)index.column();

    if (role == Qt::ItemDataRole::DisplayRole)
    {
      switch (column)
      {
      case TIME:
        return timeToString(logs_[row].time);
      case HOST_NAME:
        return logs_[row].host_name;
      case PROCESS_ID:
        return logs_[row].process_id;
      case PROCESS_NAME:
        return logs_[row].process_name;
      case PROCESS_PATH:
        return logs_[row].process_path;
      case LOG_LEVEL:
        return logLevelToString(logs_[row].log_level);
      case MESSAGE:
        return logs_[row].message;
      default:
        return QVariant();
      }
    }
    else if ((role == ItemDataRoles::FilterRole) || (role == ItemDataRoles::SortRole))
    {
      switch (column)
      {
      case TIME:
        return logs_[row].time;
      case HOST_NAME:
        return logs_[row].host_name;
      case PROCESS_ID:
        return logs_[row].process_id;
      case PROCESS_NAME:
        return logs_[row].process_name;
      case PROCESS_PATH:
        return logs_[row].process_path;
      case LOG_LEVEL:
        return logs_[row].log_level;
      case MESSAGE:
        return logs_[row].message;
      default:
        return QVariant();
      }
    }
    else if (role == Qt::ItemDataRole::ForegroundRole)
    {
      switch (column)
      {
      case LOG_LEVEL:
        return logLevelColor(logs_[row].log_level);
      default:
        return QVariant();
      }
    }
  }
  return QVariant();
}

QString LogModel::logLevelToString(int log_level)
{
  switch (log_level)
  {
  case eCAL::Logging::eLogLevel::log_level_debug1:
    return "Debug 1";
  case eCAL::Logging::eLogLevel::log_level_debug2:
    return "Debug 2";
  case eCAL::Logging::eLogLevel::log_level_debug3:
    return "Debug 3";
  case eCAL::Logging::eLogLevel::log_level_debug4:
    return "Debug 4";
  case eCAL::Logging::eLogLevel::log_level_info:
    return "Info";
  case eCAL::Logging::eLogLevel::log_level_warning:
    return "Warning";
  case eCAL::Logging::eLogLevel::log_level_error:
    return "Error";
  case eCAL::Logging::eLogLevel::log_level_fatal:
    return "Fatal";
  default:
    return "Unknown";
  }
}

QVariant LogModel::logLevelColor(int log_level)
{
  switch (log_level)
  {
  case eCAL::Logging::eLogLevel::log_level_info:
    return QColor(91, 155, 213);
  case eCAL::Logging::eLogLevel::log_level_warning:
    return QColor(255, 192, 0);
  case eCAL::Logging::eLogLevel::log_level_error:
    return QColor(255, 100, 10);
  case eCAL::Logging::eLogLevel::log_level_fatal:
    return QColor(192, 0, 0);
  default:
    return QVariant(); // Invalid QVariant // Default color for "Debug x"
  }
}

QString LogModel::timeToString(long long microseconds) const
{
  
  if (isParseTimeEnabled())
  {
    QDateTime q_ecal_time = QDateTime::fromMSecsSinceEpoch(microseconds / 1000).toUTC();
    return q_ecal_time.toString("yyyy-MM-dd HH:mm:ss.zzz");
  }
  else
  {
    double seconds_since_epoch = (double)microseconds / 1000000.0;
    return QString::number(seconds_since_epoch, 'f', 6);
  }
}

QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Orientation::Horizontal) && (role == Qt::ItemDataRole::DisplayRole))
  {
    return header_data[(Columns)section];
  }
  return QVariant();
}

int LogModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }
  return Columns::COLUMN_COUNT;
}

int LogModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    return 0;
  }
  return logs_.size();
}

QModelIndex LogModel::index(int row, int column, const QModelIndex &/*parent*/) const
{
  return createIndex(row, column, nullptr);
}

QModelIndex LogModel::parent(const QModelIndex &/*index*/) const
{
  return QModelIndex();
}

Qt::ItemFlags LogModel::flags(const QModelIndex &index) const
{
  if (index.isValid())
  {
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemNeverHasChildren;
  }
  return QAbstractItemModel::flags(index);
}

void LogModel::insertLogs(const eCAL::pb::LogMessageList& logging_pb)
{
  int inserted_row_count = logging_pb.log_messages().size();
  if (inserted_row_count <= 0) return;
  
  int size_before = logs_.size();

  // Remove entries from the top
  if (inserted_row_count + size_before > max_entries_)
  {
    int rows_to_remove = inserted_row_count - (max_entries_ - size_before);
    if (rows_to_remove > size_before)
    {
      clear();
    }
    else
    {
      beginRemoveRows(QModelIndex(), 0, rows_to_remove - 1);
      for (int i = 0; i < rows_to_remove; i++)
      {
        logs_.removeFirst();
      }
      endRemoveRows();
    }
  }

  // Add new elements to the bottom
  size_before = logs_.size();
  int size_after = (size_before + inserted_row_count < max_entries_ ? size_before + inserted_row_count : max_entries_);
  beginInsertRows(QModelIndex(), size_before, size_after - 1);

  int counter = inserted_row_count;
  for (auto& log_message_pb : logging_pb.log_messages())
  {
    if (counter <= max_entries_)
    {
      LogModel::LogEntry entry;

      entry.time = log_message_pb.time();
      entry.host_name = log_message_pb.host_name().c_str();
      entry.process_id = log_message_pb.process_id();
      entry.process_path = log_message_pb.process_name().c_str();
      entry.process_name = log_message_pb.unit_name().c_str();
      entry.log_level = log_message_pb.level();
      entry.message = log_message_pb.content().c_str();

      logs_.push_back(entry);
    }
    counter--;
  }

  endInsertRows();
}

void LogModel::clear()
{
  beginResetModel();
  logs_.clear();
  endResetModel();
}

bool LogModel::dumpToCsv(const QString& path)
{
  QFile file(path);
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream stream(&file);

    // Header
    for (int col = 0; col < COLUMN_COUNT; col++)
    {
      stream << "\"" << headerData(col, Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole).toString() << "\";";
    }

    stream << "\r\n";

    // Data
    for (int row = 0; row < rowCount(QModelIndex()); row++)
    {
      for (int col = 0; col < COLUMN_COUNT; col++)
      {
        QVariant current_data = data(index(row, col), Qt::ItemDataRole::DisplayRole);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        switch (current_data.typeId())
#else
        switch ((QMetaType::Type)current_data.type())
#endif
        {
        case QMetaType::QString:
          stream << "\"" << current_data.toString().replace("\"", "\"\"") << "\"";
          break;
        default:
          stream << QtUtil::variantToString(current_data);
        }
        stream << ";";
      }
      stream << "\r\n";
    }

    return true;
  }
  return false;
}

void LogModel::setParseTimeEnabled(bool enabled)
{
  bool update = (parse_time_enabled_ != enabled);
  parse_time_enabled_ = enabled;

  if (update)
  {
    emit dataChanged(index(0, Columns::TIME), index(rowCount(), Columns::TIME), QVector<int>{Qt::DisplayRole});
  }
}

bool LogModel::isParseTimeEnabled() const
{
  return parse_time_enabled_;
}