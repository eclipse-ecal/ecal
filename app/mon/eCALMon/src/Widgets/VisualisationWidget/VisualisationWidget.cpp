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

#include "VisualisationWidget.h"

#include <CustomQt/QStandardTreeItem.h>

#include <QTabWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QTimer>
#include <QDebug>

#include <QLibrary>
#include <QPluginLoader>
#include <QJsonArray>

#include <chrono>

#include "EcalmonGlobals.h"

#include "ecal/ecal.h"

using namespace eCAL::mon;

VisualisationWidget::VisualisationWidget(const QString& topic_name, const QString& topic_type, QWidget *parent)
  : QWidget(parent)
  , topic_name_(topic_name)
  , topic_type_(topic_type)
{
  ui_.setupUi(this);

  // Set the headline
  QString header = topic_name;
  if (!topic_type.isEmpty())
  {
    header += (" (" + topic_type + ")");
  }
  ui_.topic_name_label->setText(header);

  auto supported_plugins_ = PluginManager::getInstance()->CreatePlugins(topic_name, topic_type, this);

  // add suitable plugin widgets to the tab view
  for (const auto& plugin : supported_plugins_)
  {
    auto plugin_info = plugin.first;
    auto plugin_widget = plugin.second;
    plugin_widgets_.push_back(plugin_widget);
    ui_.tab_widget->addTab(plugin_widget->getWidget(), plugin_info.meta_data.name);

    connect(ui_.pause_button, &QPushButton::toggled, [plugin_widget](bool checked) { if (checked) plugin_widget->onPause(); else plugin_widget->onResume(); });
    connect(&update_timer_, &QTimer::timeout, [this]() {if (!isPaused()) plugin_widgets_.at(ui_.tab_widget->currentIndex())->onUpdate(); });
  }
  
  // force update if tab view has changed
  connect(ui_.tab_widget, &QTabWidget::currentChanged, [this](auto index) {plugin_widgets_.at(index)->onUpdate(); });

  // should never happens because default plugins are always available
  if (plugin_widgets_.empty())
    return;

  // initialize the update timer and connect the update signal
  update_timer_.setSingleShot(false);
  update_timer_.setInterval(UPDATE_INTERVAL_MS);

  // Button connections
  connect(ui_.pause_button, &QPushButton::toggled,
    [this](bool checked)
  { 
    if (checked)
    {
      ui_.pause_button->setText(tr("Resume"));
      update_timer_.stop();
    }
    else
    {
      ui_.pause_button->setText(tr("Pause"));
      update_timer_.start();
    }
  });

  update_timer_.start();
 }

VisualisationWidget::~VisualisationWidget()
{
#ifndef NDEBUG
  qDebug().nospace() << "[" << metaObject()->className() << "]: Deleting Widget for topic " << topic_name_;
#endif // NDEBUG
}


void VisualisationWidget::setParseTimeEnabled(bool)
{
}


////////////////////////////////////////////////////////////////////////////////
//// Getter                                                                 ////
////////////////////////////////////////////////////////////////////////////////

bool VisualisationWidget::isPaused() const
{
  return ui_.pause_button->isChecked();
}