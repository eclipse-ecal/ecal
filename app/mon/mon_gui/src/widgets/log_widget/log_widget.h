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

#include <QWidget>
#include <QTimer>
#include <QStandardItemModel>

#include "ui_log_widget.h"

#include "widgets/models/log_model.h"
#include "widgets/models/log_sort_filter_proxy_model.h"

class LogWidget : public QWidget
{
  Q_OBJECT

public:
  LogWidget(QWidget *parent = Q_NULLPTR);
  ~LogWidget();

  bool isPaused() const;

  void setParseTimeEnabled(bool enabled);
  bool isParseTimeEnabled() const;

public slots:
  void getEcalLogs();

  void setPaused(bool paused);
  void setPollSpeed(int milliseconds);

  void clearLog();

  void saveLogAs();

  void resetLayout();

private slots:
  void updateLogLevelFilter();
  void contextMenu(const QPoint &pos);

signals:
  void paused(bool paused);

private:
  Ui::LogWidget ui_{};

  QTimer* log_update_timer_;
  int log_update_time_milliseconds_;
  bool parse_time_enabled_;

  LogModel* log_model_;
  LogSortFilterProxyModel* log_proxy_model_;

  QStandardItemModel* filter_combobox_model_;
  const QVector<int> filter_columns_
  {
    LogModel::Columns::HOST_NAME,
    LogModel::Columns::PROCESS_ID,
    LogModel::Columns::PROCESS_NAME,
    LogModel::Columns::PROCESS_PATH,
    LogModel::Columns::MESSAGE,
  };

  QByteArray initial_tree_state_;
  int        initial_log_level_filter_{};
  bool       initial_auto_scroll_{};


  void saveGuiSettings();
  void loadGuiSettings();
  void saveInitialState();

  void setLogLevelFilter(int log_level);

  void copySelectedRows();
};
