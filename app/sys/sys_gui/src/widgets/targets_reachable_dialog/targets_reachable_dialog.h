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

#include <QDialog>
#include "ui_targets_reachable_dialog.h"

#include <string>
#include <vector>
#include <QDialogButtonBox>


class TargetsReachableDialog : public QDialog
{
  Q_OBJECT

public:
  TargetsReachableDialog(QWidget *parent, std::vector<std::string> targets);
  ~TargetsReachableDialog();

  bool shallStartTasks();

public slots:
  void update();

private:
  static const int IMAGE_SIZE      = 20;

  static const int ICON_COL        = 0;
  static const int TARGET_NAME_COL = 1;
  static const int STATUS_COL      = 2;


  Ui::TargetsReachableDialog ui_;
  QPushButton* cancel_button_;
  QPushButton* start_button_;

  const QPixmap x_icon         = QPixmap(":/ecalicons/CROSS")    .scaled(IMAGE_SIZE, IMAGE_SIZE, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
  const QPixmap checkmark_icon = QPixmap(":/ecalicons/CHECKMARK").scaled(IMAGE_SIZE, IMAGE_SIZE, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);

  std::vector<std::string> targets_;
  std::vector<bool>        targets_available_;

  bool start_tasks_;
  bool close_dialog_;

private slots:
  void cancelClicked();
  void startClicked();
};