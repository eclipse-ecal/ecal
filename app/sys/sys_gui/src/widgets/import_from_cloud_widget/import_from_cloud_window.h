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

#pragma once

#include <QMainWindow>
#include <QKeyEvent>

#include "ui_import_from_cloud_window.h"
#include "import_from_cloud_widget.h"

class ImportFromCloudWindow : public QMainWindow
{
  Q_OBJECT

public:
  ImportFromCloudWindow(QWidget *parent = Q_NULLPTR);
  ~ImportFromCloudWindow();

  void resetLayout(int target_screen = -1);

  ImportFromCloudWidget* import_from_cloud_widget_;

protected:
  void keyPressEvent(QKeyEvent * event);

  void showEvent(QShowEvent* event);
  void closeEvent(QCloseEvent* event);

private:
  Ui::ImportFromCloudWindow ui_;

  QByteArray initial_window_geometry_;
};
