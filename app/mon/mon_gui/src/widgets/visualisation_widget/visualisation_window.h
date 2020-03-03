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

#include <QMainWindow>
#include "visualisation_widget.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4146)
#endif
#include "ecal/pb/monitoring.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class VisualisationWindow : public QMainWindow
{
  Q_OBJECT

public:
  VisualisationWindow(const QString& topic_name, const QString& topic_type, QWidget *parent = Q_NULLPTR);
  ~VisualisationWindow();

  void resetLayout(int target_screen = -1);

  VisualisationWidget* visualisation_widget;
  void restoreWindowState();

signals:
  void updateVisualizationWidget(const eCAL::pb::Monitoring & monitoring_pb);

public slots:
  void setParseTimeEnabled(bool enabled);
  void updatePublishers(const eCAL::pb::Monitoring& monitoring_pb);

protected:
  void keyPressEvent(QKeyEvent * event);

  bool event(QEvent* e);

private:
  bool initial_resize_executed_;

  void saveWindowState();
  void saveWindowSize();
};
