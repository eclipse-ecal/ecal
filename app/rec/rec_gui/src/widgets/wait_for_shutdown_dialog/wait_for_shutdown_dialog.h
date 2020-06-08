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

#include <QDialog>

#include "ui_wait_for_shutdown_dialog.h"

#include <QPixmap>

#include <rec_client_core/state.h>

class WaitForShutdownDialog : public QDialog
{
Q_OBJECT

////////////////////////////////////
// Constructor & Destructor
////////////////////////////////////
public:
  WaitForShutdownDialog(QWidget *parent = Q_NULLPTR);
  ~WaitForShutdownDialog();

////////////////////////////////////
// Update
////////////////////////////////////
public:
  bool safeToExit() const;

private:
  bool updateWaitForFlushing();
  bool updateBuiltInRecorderUploading(const eCAL::rec::RecorderStatus& local_rec_status);
  bool updateBuiltInFtpServer();
  bool updateNonUploadedMeasurements();

  void updateItIsSafeToExit(bool safe_to_exit);

private slots:
  bool updateAllFields();

////////////////////////////////////
// Helper Methods
////////////////////////////////////
private:
  void setDisabledTextColor(QWidget* widget, const QColor& color);

private slots:
  void uploadAllMeasurements() const;

////////////////////////////////////
// Member Variables
////////////////////////////////////
private:
  Ui::WaitForShutdownDialog ui_;

  bool safe_to_extit_;

  QTimer*      update_timer_;
  QPushButton* shutdown_button_;

  int64_t built_in_recorder_initial_unflushed_frames_;
  std::map<uint64_t, eCAL::rec::UploadStatus> built_in_recorer_uploading_jobs_;
};
