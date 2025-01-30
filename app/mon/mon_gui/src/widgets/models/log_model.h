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

#pragma once

#include <QAbstractItemModel>

#include <QVector>
#include <QColor>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/core/pb/logging.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class LogModel : public QAbstractItemModel 
{
  Q_OBJECT

public:
  struct LogEntry
  { //-V802
    long long time;
    QString   host_name;
    int       process_id;
    QString   process_name;
    QString   process_path;
    int       log_level;
    QString   message;
  };

  enum Columns : int
  {
    TIME,
    LOG_LEVEL,
    HOST_NAME,
    PROCESS_ID,
    PROCESS_NAME,
    PROCESS_PATH,
    MESSAGE,

    COLUMN_COUNT
  };

  LogModel(QObject* parent = Q_NULLPTR);
  ~LogModel();

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  int columnCount(const QModelIndex &parent) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  void insertLogs(const eCAL::pb::LogMessageList& logs);

  void setParseTimeEnabled(bool enabled);
  bool isParseTimeEnabled() const;

  void clear();

  bool dumpToCsv(const QString& path);

private:
  QList<LogEntry> logs_;

  const QMap<Columns, QString> header_data
  {
    { Columns::TIME,         "Time" },
    { Columns::HOST_NAME,    "Host" },
    { Columns::PROCESS_ID,   "PROCESS_ID" },
    { Columns::PROCESS_NAME, "Process" },
    { Columns::PROCESS_PATH, "Process Path" },
    { Columns::LOG_LEVEL,    "Level" },
    { Columns::MESSAGE,      "Message" },
  };

  int max_entries_;

  bool parse_time_enabled_;

  static QString logLevelToString(int log_level);
  static QVariant logLevelColor(int log_level);
  QString timeToString(long long milliseconds) const;
};
