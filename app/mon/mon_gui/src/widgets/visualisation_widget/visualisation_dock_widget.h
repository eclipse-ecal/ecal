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
#include <QTimer>

#include "ui_visualisation_dock_widget.h"

class VisualisationDockWidget : public QDockWidget
{
  Q_OBJECT

public:
  VisualisationDockWidget(const QString& topic_name, const QString& topic_type, const QString& plugin_iid, QWidget *parent = Q_NULLPTR);
  ~VisualisationDockWidget();

  QString getTopicName() const;
  QString getTopicType() const;
  QString getPluginIID() const;

protected:
  Ui::visualisation_dock_widget ui_;

private:
  static const int UPDATE_INTERVAL_MS = 50;

  QTimer update_timer_;

  QString topic_name_;
  QString topic_type_;
  QString plugin_iid_;
};