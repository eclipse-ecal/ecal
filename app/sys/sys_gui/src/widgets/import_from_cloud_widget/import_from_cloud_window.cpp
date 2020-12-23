/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#include "import_from_cloud_window.h"

#include <QDesktopWidget>
#include <QScreen>

ImportFromCloudWindow::ImportFromCloudWindow(QWidget *parent)
    : QMainWindow(parent)
{
  ui_.setupUi(this);

  import_from_cloud_widget_ = new ImportFromCloudWidget(this);
  import_from_cloud_widget_->setUpdateEnabled(false);
  ui_.central_layout->addWidget(import_from_cloud_widget_);

  connect(import_from_cloud_widget_, &ImportFromCloudWidget::closeSignal, [this]() { close(); });

  initial_window_geometry_ = saveGeometry();
}

ImportFromCloudWindow::~ImportFromCloudWindow()
{
}

void ImportFromCloudWindow::resetLayout(int target_screen)
{
  restoreGeometry(initial_window_geometry_);

  move(QGuiApplication::screens().at(target_screen)->availableGeometry().center() - rect().center());

  import_from_cloud_widget_->resetLayout();
}

void ImportFromCloudWindow::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key::Key_Escape)
  {
    close();
  }
}

void ImportFromCloudWindow::showEvent(QShowEvent* event)
{
  import_from_cloud_widget_->reload();
  import_from_cloud_widget_->setUpdateEnabled(true);
  QWidget::showEvent(event);
}

void ImportFromCloudWindow::closeEvent(QCloseEvent* event)
{
  import_from_cloud_widget_->setUpdateEnabled(false);
  import_from_cloud_widget_->clear();
  event->accept();
}
