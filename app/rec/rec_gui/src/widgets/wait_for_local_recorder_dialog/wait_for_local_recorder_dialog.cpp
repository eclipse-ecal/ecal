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

#include "wait_for_local_recorder_dialog.h"

#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QResizeEvent>

#include "qecalrec.h"

WaitForLocalRecorderDialog::WaitForLocalRecorderDialog(QWidget *parent)
  : QDialog(parent)
  , first_resize_event_(true)
{
  ui_.setupUi(this);

  ui_.progress_bar->setMaximum((int)remainingFrames());

  update_timer_ = new QTimer(this);
  connect(update_timer_, &QTimer::timeout, this, &WaitForLocalRecorderDialog::updateState);

  ui_.button_box->addButton(QDialogButtonBox::StandardButton::Cancel);
  ui_.button_box->button(QDialogButtonBox::StandardButton::Cancel)->setAutoDefault(true);
  ui_.button_box->button(QDialogButtonBox::StandardButton::Cancel)->setDefault(true);

  shutdown_button_ = new QPushButton(tr("Force close"), this);
  shutdown_button_->setDefault(false);
  shutdown_button_->setAutoDefault(false);
  ui_.button_box->addButton(shutdown_button_, QDialogButtonBox::ButtonRole::AcceptRole);
}

WaitForLocalRecorderDialog::~WaitForLocalRecorderDialog()
{}

void WaitForLocalRecorderDialog::updateState()
{
  size_t remaining_frames = remainingFrames();

  ui_.progress_bar->setValue(ui_.progress_bar->maximum() - (int)remaining_frames);

  if (remaining_frames == 0)
  {
    shutdown_button_->setText(tr("Close"));
    ui_.flushing_to_disk_label->setText(tr("Finished"));
    ui_.icon_label->setPixmap(icon_ready);
    update_timer_->stop();
  }
}

void WaitForLocalRecorderDialog::resizeEvent(QResizeEvent *e)
{
  if (first_resize_event_)
  {
    int label_height = ui_.flushing_to_disk_label->height();
    icon_ready = QPixmap(":/ecalicons/CHECKMARK").scaledToHeight(label_height, Qt::TransformationMode::SmoothTransformation);
    icon_flushing = QPixmap(":/ecalicons/SAVE_TO_DISK").scaledToHeight(label_height, Qt::TransformationMode::SmoothTransformation);
    
    ui_.icon_label->setPixmap(icon_flushing);
    update_timer_->start(40);

    first_resize_event_ = false;
  }
  e->accept();
}

size_t WaitForLocalRecorderDialog::remainingFrames()
{
  auto local_recorder_state = QEcalRec::instance()->localRecorderInstanceState();

  size_t remaining_frames = local_recorder_state.main_recorder_state_.recording_queue_.first;
  for (const auto& buffer_writer : local_recorder_state.buffer_writers_)
  {
    remaining_frames += buffer_writer.recording_queue_.first;
  }

  return remaining_frames;
}