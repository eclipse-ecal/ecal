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

#include "visualisation_window.h"


#include <QSettings>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QScreen>


VisualisationWindow::VisualisationWindow(const QString& topic_name, const QString& topic_type, QWidget *parent)
  : QMainWindow(parent)
  , initial_resize_executed_(false)
{
  restoreWindowState();

  QString window_title = topic_name;
  if (!topic_type.isEmpty())
  {
    window_title += (" (" + topic_type + ")");
  }
  setWindowTitle(window_title);

  visualisation_widget = new VisualisationWidget(topic_name, topic_type, this);
  connect(this, &VisualisationWindow::updateVisualizationWidget, visualisation_widget, &VisualisationWidget::checkForMorePublishersWithSameTopic);
  setCentralWidget(visualisation_widget);

  QIcon icon;
  icon.addFile(QStringLiteral(":/ecalmon/APP_ICON"), QSize(), QIcon::Normal, QIcon::Off);
  setWindowIcon(icon);
}

void VisualisationWindow::resetLayout(int target_screen)
{
  showNormal();
  resize(450, 500);
  move(QGuiApplication::screens().at(target_screen)->availableGeometry().center() - rect().center());

  visualisation_widget->resetLayout();
}

VisualisationWindow::~VisualisationWindow()
{
}

void VisualisationWindow::updatePublishers(const eCAL::pb::Monitoring & monitoring_pb)
{
  emit updateVisualizationWidget(monitoring_pb);
}

void VisualisationWindow::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key::Key_Escape)
  {
    close();
  }
}

bool VisualisationWindow::event(QEvent * e)
{
  if (e->type() == QEvent::WindowDeactivate)
  {
    visualisation_widget->saveState();
  }
  else if ((e->type() == QEvent::Resize))
  {
    if (initial_resize_executed_)
    {
      saveWindowSize();
    }
    initial_resize_executed_ = true;
  }
  else if (e->type() == QEvent::WindowStateChange)
  {
    saveWindowState();
  }
  return QMainWindow::event(e);
}

void VisualisationWindow::saveWindowState()
{
  QSettings settings;
  settings.beginGroup("reflection_window");
  settings.setValue("maximized", isMaximized());
  settings.endGroup();
}

void VisualisationWindow::saveWindowSize()
{
  QSettings settings;
  settings.beginGroup("reflection_window");
  settings.setValue("window_size", size());
  settings.endGroup();
}

void VisualisationWindow::restoreWindowState()
{
  QSettings settings;
  settings.beginGroup("reflection_window");

  QVariant size_variant      = settings.value("window_size");
  QVariant maximized_variant = settings.value("maximized");

  if (size_variant.isValid())
  {
    QSize size = size_variant.toSize();
    resize(size);
  }
  else
  {
    resize(450, 500);
  }

  if (maximized_variant.isValid())
  {
    bool maximized = maximized_variant.toBool();
    if (maximized)
      showMaximized();
  }

  settings.endGroup();
}

void VisualisationWindow::setParseTimeEnabled(bool enabled)
{
  visualisation_widget->setParseTimeEnabled(enabled);
}