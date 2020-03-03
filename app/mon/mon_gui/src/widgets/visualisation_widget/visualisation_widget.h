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

#include <QWidget>
#include "ui_visualisation_widget.h"

#include "plugin/plugin_manager.h"

#include <memory>
#include <mutex>
#include <chrono>
#include <atomic>

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include <ecal/ecal.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4146)
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif


class VisualisationWidget : public QWidget
{
  Q_OBJECT

public:
  VisualisationWidget(const QString& topic_name, const QString& topic_type, QWidget *parent = Q_NULLPTR);
  ~VisualisationWidget();

  bool isPaused() const;

  virtual void saveState  () {}
  virtual void resetLayout() {}

protected:
  Ui::VisualisationWidget ui_;

public slots:
  void setParseTimeEnabled(bool enabled);
  void checkForMorePublishersWithSameTopic(const eCAL::pb::Monitoring& monitoring_pb);

private:
  static const int UPDATE_INTERVAL_MS = 50;

  QString topic_name_;
  QString topic_type_;
  QTimer    update_timer_;
  QDateTime last_update_time_;
  
  QTabWidget tab_widget_;

  QVector<eCAL::mon::PluginWidgetInterface*> plugin_widgets_;
};
