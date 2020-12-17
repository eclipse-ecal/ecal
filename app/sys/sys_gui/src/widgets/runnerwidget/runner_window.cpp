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

#include "runner_window.h"

#include <QSettings>
#include <QScreen>
#include <QDesktopWidget>

RunnerWindow::RunnerWindow(QWidget *parent)
    : QMainWindow(parent)
{
  ui.setupUi(this);

  runner_widget = new RunnerWidget(this);
  ui.central_layout->addWidget(runner_widget);

  initial_window_geometry_ = saveGeometry();
}

void RunnerWindow::resetLayout(int target_screen)
{
  restoreGeometry(initial_window_geometry_);
  move(QGuiApplication::screens().at(target_screen)->availableGeometry().center() - rect().center());

  runner_widget->resetLayout();
}

RunnerWindow::~RunnerWindow()
{
}

void RunnerWindow::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key::Key_Escape)
  {
    close();
  }
}