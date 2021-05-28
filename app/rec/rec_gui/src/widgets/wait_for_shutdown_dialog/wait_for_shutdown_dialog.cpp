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

////////////////////////////////////
// Constructor & Destructor
////////////////////////////////////

WaitForShutdownDialog::WaitForShutdownDialog(QWidget *parent)
  : QDialog(parent)
  , safe_to_extit_(false)
  , initial_unflushed_frames_(0)
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
  QPalette label_palette = ui_.wait_for_uploads_groupbox->palette();
  label_palette.setCurrentColorGroup(QPalette::ColorGroup::Disabled);
  QColor diabled_label_color = label_palette.color(QPalette::WindowText);

  setDisabledTextColor(ui_.wait_for_uploads_groupbox, diabled_label_color);
  setDisabledTextColor(ui_.built_in_ftp_server_busy_groupbox,    diabled_label_color);
  setDisabledTextColor(ui_.wait_for_flushing_groupbox,           diabled_label_color);
  setDisabledTextColor(ui_.non_uploaded_recordings_groupbox,     diabled_label_color);

  // Hide all warnings
  ui_.wait_for_uploads_groupbox           ->setVisible(false);
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
  if (initial_unflushed_frames_ == 0)
  {
    initial_unflushed_frames_ = unflushed_frames;
  }

  bool finished = (job_ids_flushing.size() == 0);

  double relative_flushed = 1.0;
  if (initial_unflushed_frames_ > 0)
  {
    relative_flushed = (double)(initial_unflushed_frames_ - unflushed_frames) / (double)initial_unflushed_frames_;
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

bool WaitForShutdownDialog::updateBuiltInRecorderUploading(const std::list<eCAL::rec_server::JobHistoryEntry>& job_history)
{
  // Update the state of all uploads that we already know of
  for (auto& meas_upload_pair : uploading_jobs_)
  {
    const auto  meas_id       = meas_upload_pair.first;
    
    // Search for the according entry in the job_history
    const auto job_it = std::find_if(job_history.begin(), job_history.end(), [meas_id](const eCAL::rec_server::JobHistoryEntry& job_history_entry) { return job_history_entry.local_evaluated_job_config_.GetJobId() == meas_id; });

    for (auto& host_upload_pair : meas_upload_pair.second)
    {
      const auto& hostname        = host_upload_pair.first;
      bool&       still_uploading = host_upload_pair.second.first;
      auto&       upload_status   = host_upload_pair.second.second;

      bool upload_status_found = false;

      if (job_it !=  job_history.end())
      {
        // Search for the according Host
        auto client_status_it = job_it->client_statuses_.find(hostname);

        if (client_status_it != job_it->client_statuses_.end())
        {
          upload_status   = client_status_it->second.job_status_.upload_status_;
          still_uploading = (client_status_it->second.job_status_.state_ == eCAL::rec::JobState::Uploading);
          upload_status_found = true;
        }
      }

      if (!upload_status_found)
      {
        // Not sure how we got the upload status, but now it is missing. We assume it being finished. This should never happen if all clients function normally.
        still_uploading = false;
      }
    }
  }

  // Now check if there are any new clients uploading something!
  for (const auto& job_history_entry : job_history)
  {
    for (const auto& client_status : job_history_entry.client_statuses_)
    {
      if (client_status.second.job_status_.state_ == eCAL::rec::JobState::Uploading)
      {
        const int64_t                  meas_id       = job_history_entry.local_evaluated_job_config_.GetJobId();
        const std::string&             hostname      = client_status.first;
        const eCAL::rec::UploadStatus& upload_status = client_status.second.job_status_.upload_status_;

        auto known_uploading_meas_it = uploading_jobs_.find(meas_id);
        if (known_uploading_meas_it == uploading_jobs_.end())
        {
          known_uploading_meas_it = uploading_jobs_.emplace(meas_id, std::map<std::string, std::pair<bool, eCAL::rec::UploadStatus>>()).first;
        }

        auto known_uploading_host_it = known_uploading_meas_it->second.find(hostname);
        if (known_uploading_host_it == known_uploading_meas_it->second.end())
        {
          known_uploading_meas_it->second.emplace(hostname, std::pair<bool, eCAL::rec::UploadStatus>(true, upload_status));
        }
      }
    }
  }

  // Lets accumulate all data!
  int      num_jobs_uploading(0);
  uint64_t num_total_bytes   (0);
  uint64_t num_uploaded_bytes(0);
  double   rel_uploaded_     (1.0);

  for (const auto& job : uploading_jobs_)
  {
    bool job_uploading(false);

    for (const auto& client_upload_status:  job.second)
    {
      job_uploading      = (job_uploading || client_upload_status.second.first);
      num_total_bytes    += client_upload_status.second.second.bytes_total_size_;
      num_uploaded_bytes += client_upload_status.second.second.bytes_uploaded_;
    }

    if (job_uploading)
      num_jobs_uploading++;
  }

  if (num_total_bytes > 0)
    rel_uploaded_ = (double)num_uploaded_bytes / (double)num_total_bytes;

  // Update the GUI text
  ui_.wait_for_uploads_label->setText(QString::number(num_jobs_uploading)
    + (num_jobs_uploading == 1 ? tr(" recording is being uploaded") : tr(" recordings are being uploaded")));
  
  // Set the progress bar value
  ui_.wait_for_uploads_progressbar->setValue((int)(rel_uploaded_ * (double)ui_.wait_for_uploads_progressbar->maximum()));

  // Set the progress bar text
  ui_.wait_for_uploads_progressbar->setFormat(bytesToPrettyString(num_uploaded_bytes) + " of " + bytesToPrettyString(num_total_bytes));

  // Enable / Disable the groupbox
  ui_.wait_for_uploads_groupbox->setEnabled(num_jobs_uploading > 0);

  if(num_jobs_uploading > 0)
    ui_.wait_for_uploads_groupbox->setVisible(true);

  return (num_jobs_uploading <= 0);
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
  auto job_history = QEcalRec::instance()->jobHistory();

  // Bitwise-or is intended!
  bool safe_to_exit = updateWaitForFlushing()
                    & updateBuiltInRecorderUploading(job_history)
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