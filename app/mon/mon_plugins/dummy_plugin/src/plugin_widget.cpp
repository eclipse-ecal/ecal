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

#include "PluginWidget.h"

#include <QDebug>

PluginWidget::PluginWidget(const QString& topic_name, const QString& topic_type, QWidget* parent): QWidget(parent)
{
  qDebug() << "Dummy Plugin initialized with topic " << topic_name << " (" << topic_type << ")"; 
}

PluginWidget::~PluginWidget()
{
  qDebug() << "Dummy Plugin deinitialized";
}

void PluginWidget::onUpdate()
{
  //qDebug() << "Dummy Plugin on update";
}

void PluginWidget::onResume()
{
  qDebug() << "Dummy Plugin on Resume";
}

void PluginWidget::onPause()
{
  qDebug() << "Dummy Plugin on Pause";
}

QWidget* PluginWidget::getWidget()
{
  return this;
}