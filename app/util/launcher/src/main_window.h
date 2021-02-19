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

/**
 * @brief eCALLauncher Application
**/

#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QMouseEvent>

#include "about_dialog.h"

namespace Ui
{
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:

  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  bool eventFilter(QObject * watched, QEvent * event);
public slots:
  void mousePressEvent(QMouseEvent * event);

private slots:
  void on_pushButton_monitor_clicked();
  void on_pushButton_sys_clicked();
  void on_pushButton_play_clicked();
  void on_pushButton_rec_clicked();
  void on_pushButton_info_clicked();
  void on_pushButton_exit_clicked();

private:

  Ui::MainWindow*   _ui;
  QPoint            _dragPosition;
  AboutDialog*      _about_dialog;
};
