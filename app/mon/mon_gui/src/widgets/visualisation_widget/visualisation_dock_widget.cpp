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

#include "visualisation_dock_widget.h"

#include "plugin/plugin_manager.h"

#ifndef NDEBUG
  #ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4251 4800) // disable QDebug Warnings
  #endif // _MSC_VER

  #include <QDebug>

  #ifdef _MSC_VER
    #pragma warning(pop)
  #endif // _MSC_VER
#endif // NDEBUG

VisualisationDockWidget::VisualisationDockWidget(const QString& topic_name, const QString& topic_type, const QString& plugin_iid, QWidget *parent)
  : QDockWidget(parent), topic_name_(topic_name), topic_type_(topic_type), plugin_iid_(plugin_iid)
{
  ui_.setupUi(this);

  auto plugin_widget = PluginManager::getInstance()->CreatePlugin(topic_name, topic_type, plugin_iid, this);

  if (plugin_widget != nullptr)
  {
    const auto& plugin_name = PluginManager::getInstance()->getPluginData(plugin_iid).meta_data.name;
    setWindowTitle(plugin_name + " - " + topic_name + "(" + topic_type + ")");
    ui_.visualisation_dock_widget_context_frame_layout->addWidget(plugin_widget->getWidget());
    ui_.visualisation_unavailable_label->hide();
    connect(&update_timer_, &QTimer::timeout, [plugin_widget]() {plugin_widget->onUpdate(); });
    update_timer_.setSingleShot(false);
    update_timer_.setInterval(UPDATE_INTERVAL_MS);
    update_timer_.start();
  }
  else
  {
    setWindowTitle("Visualisation - " + topic_name + "(" + topic_type + ")");
  }
}

QString VisualisationDockWidget::getTopicName() const
{
  return topic_name_;
}

QString VisualisationDockWidget::getTopicType() const
{
  return topic_type_;
}

QString VisualisationDockWidget::getPluginIID() const
{
  return plugin_iid_;
}

VisualisationDockWidget::~VisualisationDockWidget()
{
}