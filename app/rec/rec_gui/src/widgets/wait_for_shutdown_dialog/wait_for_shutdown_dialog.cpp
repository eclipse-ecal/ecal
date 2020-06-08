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

#include "wait_for_shutdown_dialog.h"

#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QResizeEvent>
#include <QPalette>
#include <QColor>

#include "qecalrec.h"

#include <widgets/bytes_to_pretty_string_utils.h>

// TODO: This window must differentiate between the local recorder flushing frames, uploading from the local recorder and uploading to the local FTP Server

////////////////////////////////////
// Constructor & Destructor
////////////////////////////////////

WaitForShutdownDialog::WaitForShutdownDialog(QWidget *parent)
  : QDialog(parent)
  , safe_to_extit_(false)
  , built_in_recorder_initial_unflushed_frames_(0)
{
  ui_.setupUi(this);

  // Upload button
  //QString ftp_target_string;

  if (QEcalRec::instance()->uploadConfig().type_ == eCAL::rec_server::UploadConfig::Type::INTERNAL_FTP)
    ui_.upload_button->setText(tr("Merge / Upload all (to this PC)"));
  else
    ui_.upload_button->setText(tr("Merge / Upload all (to ") + QString::fromStdString(QEcalRec::instance()->uploadConfig().host_) + ")");

  connect(ui_.upload_button, &QAbstractButton::clicked, this, &WaitForShutdownDialog::uploadAllMeasurements);

  // Update timer
  update_timer_ = new QTimer(this);
  connect(update_timer_, &QTimer::timeout, this, &WaitForShutdownDialog::updateAllFields);
  update_timer_->start(100);

  // Button box
  QPushButton* cancel_button = ui_.button_box->addButton(QDialogButtonBox::StandardButton::Cancel);
  cancel_button->setAutoDefault(true);
  cancel_button->setDefault(true);

  shutdown_button_ = new QPushButton(tr("Force close"), this);
  shutdown_button_->setIcon(QIcon(":/ecalicons/WARNING"));
  shutdown_button_->setDefault(false);
  shutdown_button_->setAutoDefault(false);
  ui_.button_box->addButton(shutdown_button_, QDialogButtonBox::ButtonRole::AcceptRole);

  connect(shutdown_button_, &QAbstractButton::clicked, this, &QDialog::accept);
  connect(cancel_button,    &QAbstractButton::clicked, this, &QDialog::reject);

  // Fix disabled-colors
  QPalette label_palette = ui_.built_in_recorder_uploading_label->palette();
  label_palette.setCurrentColorGroup(QPalette::ColorGroup::Disabled);
  QColor diabled_label_color = label_palette.color(QPalette::WindowText);

  setDisabledTextColor(ui_.built_in_recorder_uploading_groupbox, diabled_label_color);
  setDisabledTextColor(ui_.built_in_ftp_server_busy_groupbox,    diabled_label_color);
  setDisabledTextColor(ui_.wait_for_flushing_groupbox,           diabled_label_color);
  setDisabledTextColor(ui_.non_uploaded_recordings_groupbox,     diabled_label_color);

  // Hide all warnings
  ui_.built_in_recorder_uploading_groupbox->setVisible(false);
  ui_.built_in_ftp_server_busy_groupbox   ->setVisible(false);
  ui_.wait_for_flushing_groupbox          ->setVisible(false);
  ui_.non_uploaded_recordings_groupbox    ->setVisible(false);

  // Initial update
  updateAllFields();
}

WaitForShutdownDialog::~WaitForShutdownDialog()
{}


////////////////////////////////////
// Update
////////////////////////////////////

bool WaitForShutdownDialog::safeToExit() const
{
  return safe_to_extit_;
}

bool WaitForShutdownDialog::updateWaitForFlushing()
{
  int64_t unflushed_frames = 0;
  std::set<uint64_t> job_ids_flushing;

  for (const auto& rec_status : QEcalRec::instance()->recorderStatuses())
  {
    for (const auto& job_status : rec_status.second.first.job_statuses_)
    {
      // Save the currently flushing jobs
      if (job_status.state_ == eCAL::rec::JobState::Flushing)
        job_ids_flushing.emplace(job_status.job_id_);

      // Accumulate HDF5 frames that still need to be flushed
      unflushed_frames += job_status.rec_hdf5_status_.unflushed_frame_count_;

      // Accumulate Addon frames that still need to be flushed
      for (const auto& addon_jobstatus : job_status.rec_addon_statuses_)
      {
        if (addon_jobstatus.second.state_ == eCAL::rec::RecAddonJobStatus::State::Flushing)
          job_ids_flushing.emplace(job_status.job_id_);

        unflushed_frames += addon_jobstatus.second.unflushed_frame_count_;
      }
    }
  }

  if (!QEcalRec::instance()->usingBuiltInRecorderEnabled())
  {
    // Even if the built-in recorder is not enabled, we must still wait for it to flush.

    auto built_in_rec_status = QEcalRec::instance()->builtInRecorderInstanceStatus();

    for (const auto& job_status : built_in_rec_status.job_statuses_)
    {
      // Save the currently flushing jobs
      if (job_status.state_ == eCAL::rec::JobState::Flushing)
        job_ids_flushing.emplace(job_status.job_id_);

      // Accumulate HDF5 frames that still need to be flushed
      unflushed_frames += job_status.rec_hdf5_status_.unflushed_frame_count_;

      // Accumulate Addon frames that still need to be flushed
      for (const auto& job_addon_status : job_status.rec_addon_statuses_)
      {
        if (job_addon_status.second.state_ == eCAL::rec::RecAddonJobStatus::State::Flushing)
          job_ids_flushing.emplace(job_status.job_id_);

        unflushed_frames += job_addon_status.second.unflushed_frame_count_;
      }
    }
  }

  // Set the number of unflushed frames on the first run
  if (built_in_recorder_initial_unflushed_frames_ == 0)
  {
    built_in_recorder_initial_unflushed_frames_ = unflushed_frames;
  }

  bool finished = (job_ids_flushing.size() == 0);

  double relative_flushed = 1.0;
  if (built_in_recorder_initial_unflushed_frames_ > 0)
  {
    relative_flushed = (double)(built_in_recorder_initial_unflushed_frames_ - unflushed_frames) / (double)built_in_recorder_initial_unflushed_frames_;
  }

  if (!finished)
  {
    // Set the progress bar value and label
    ui_.wait_for_flushing_progressbar->setValue((int)((double)ui_.wait_for_flushing_progressbar->maximum() * relative_flushed));
    ui_.wait_for_flushing_progressbar->setFormat("%p%");
  }
  else
  {
    // Clear the progress bar value and label
    ui_.wait_for_flushing_progressbar->setValue(0);
    ui_.wait_for_flushing_progressbar->setFormat("100%");
  }

  // Set the label
  ui_.wait_for_flushing_label->setText(QString::number(job_ids_flushing.size())
    + (job_ids_flushing.size() == 1 ? tr(" recording is being flushed") : tr(" recordings are being flushed")));

  // Enable / Disable the groupbox
  ui_.wait_for_flushing_groupbox->setEnabled(!finished);

  if(!finished)
    ui_.wait_for_flushing_groupbox->setVisible(true);

  return finished;
}

bool WaitForShutdownDialog::updateBuiltInRecorderUploading(const eCAL::rec::RecorderStatus& local_rec_status)
{
  // Collect all jobs that are in UPLOADING state
  std::map<uint64_t, eCAL::rec::UploadStatus> currently_uploading_jobs;
  for (const auto& job_status : local_rec_status.job_statuses_)
  {
    if (job_status.state_ == eCAL::rec::JobState::Uploading)
    {
      currently_uploading_jobs.emplace(job_status.job_id_, job_status.upload_status_);
    }
  }

  // Update the text
  ui_.built_in_recorder_uploading_label->setText(QString::number(currently_uploading_jobs.size())
    + (currently_uploading_jobs.size() == 1 ? tr(" recording is being uploaded") : tr(" recordings are being uploaded")));

  // Set all old jobs from the uploading map to 0 bytes still uploading
  for (auto& job : built_in_recorer_uploading_jobs_)
  {
    job.second.bytes_uploaded_ = job.second.bytes_total_size_;
  }

  // Update the uploading map with the jobs that are currently uploading
  for (const auto& job : currently_uploading_jobs)
  {
    built_in_recorer_uploading_jobs_[job.first] = job.second;
  }

  // Calculate the totals
  uint64_t bytes_total    = 0;
  uint64_t bytes_uploaded = 0;
  for (auto& job : built_in_recorer_uploading_jobs_)
  {
    bytes_total    += job.second.bytes_total_size_;
    bytes_uploaded += job.second.bytes_uploaded_;
  }

  double relative_uploaded = 1.0;
  if (bytes_total > 0)
  {
    relative_uploaded = (double)bytes_uploaded / (double)bytes_total;
  }

  bool finished = (currently_uploading_jobs.size() == 0);
  
  if (!finished)
  {
    // Set the progress bar value
    ui_.built_in_recorder_uploading_progressbar->setValue((int)(relative_uploaded * (double)ui_.built_in_recorder_uploading_progressbar->maximum()));
  }
  else
  {
    // Clear progressbar value
    ui_.built_in_recorder_uploading_progressbar->setValue(0);
  }

  // Set the progress bar text
  ui_.built_in_recorder_uploading_progressbar->setFormat(bytesToPrettyString(bytes_uploaded) + " of " + bytesToPrettyString(bytes_total));

  // set the label
  ui_.built_in_recorder_uploading_label->setText(QString::number(currently_uploading_jobs.size())
    + (currently_uploading_jobs.size() == 1 ? tr(" recording is being uploaded") : tr(" recordings are being uploaded")));

  // Enable / Disable the groupbox
  ui_.built_in_recorder_uploading_groupbox->setEnabled(!finished);

  if(!finished)
    ui_.built_in_recorder_uploading_groupbox->setVisible(true);

  return finished;
}

bool WaitForShutdownDialog::updateBuiltInFtpServer()
{
  int open_connections = QEcalRec::instance()->internalFtpServerOpenConnectionsCount();

  ui_.built_in_ftp_server_open_connections_label->setText(QString::number(open_connections));

  bool finished = (open_connections == 0);

  // Enable / disable the groupbox
  ui_.built_in_ftp_server_busy_groupbox->setEnabled(!finished);

  if(!finished)
    ui_.built_in_ftp_server_busy_groupbox->setVisible(true);

  return finished;
}

bool WaitForShutdownDialog::updateNonUploadedMeasurements()
{
  int number_of_non_uploaded_measurements = 0;

  // Count how many measurements can be uploaded
  for (const auto& job_history_entry : QEcalRec::instance()->jobHistory())
  {
    // TODO: using "canUploadMeasurment" is not sufficient. A measurement that has not been flushed by another client cannot be flushed, but it must be!
    if (QEcalRec::instance()->canUploadMeasurement(job_history_entry.local_evaluated_job_config_.GetJobId()))
      number_of_non_uploaded_measurements++;
  }

  bool finished = (number_of_non_uploaded_measurements == 0);

  // Set the label
  ui_.non_uploaded_recordings_label->setText(QString::number(number_of_non_uploaded_measurements)
    + (number_of_non_uploaded_measurements == 1 ? tr(" recording is not merged, yet. Click the button to upload it.") : tr(" recordings are not merged, yet. Click the button to upload them.")));

  // Enable / disable the groupbox
  ui_.non_uploaded_recordings_groupbox->setEnabled(!finished);

  if(!finished)
    ui_.non_uploaded_recordings_groupbox->setVisible(true);

  return finished;
}

void WaitForShutdownDialog::updateItIsSafeToExit(bool safe_to_exit)
{
  if (safe_to_exit)
  {
    shutdown_button_->setIcon(QIcon());
    shutdown_button_->setText(tr("Exit"));

    ui_.header_label->setDisabled(true);
    ui_.safe_to_exit_label->setText(tr("It is safe to close eCAL Rec now."));
  }
  else
  {
    shutdown_button_->setIcon(QIcon(":/ecalicons/WARNING"));
    shutdown_button_->setText(tr("Force close"));

    ui_.header_label->setDisabled(false);
    ui_.safe_to_exit_label->setText("");
  }
}

bool WaitForShutdownDialog::updateAllFields()
{
  const eCAL::rec::RecorderStatus local_rec_status = QEcalRec::instance()->builtInRecorderInstanceStatus();

  bool safe_to_exit = updateWaitForFlushing()
                    & updateBuiltInRecorderUploading(local_rec_status)
                    & updateBuiltInFtpServer()
                    & updateNonUploadedMeasurements();

  updateItIsSafeToExit(safe_to_exit);

  safe_to_extit_ = safe_to_exit;

  return safe_to_exit;
}

////////////////////////////////////
// Helper Methods
////////////////////////////////////

void WaitForShutdownDialog::setDisabledTextColor(QWidget* widget, const QColor& color)
{
  QPalette palette = widget->palette();
  palette.setColor(QPalette::ColorGroup::Disabled, QPalette::ColorRole::WindowText, color);
  //auto current_color_group = palette.currentColorGroup();

  //palette.setCurrentColorGroup(QPalette::Disabled);
  //palette.setColor(QPalette::WindowText, color);
  //palette.setCurrentColorGroup(current_color_group);

  widget->setPalette(palette);
}

void WaitForShutdownDialog::uploadAllMeasurements() const
{
  QEcalRec::instance()->uploadNonUploadedMeasurements(true);
}