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

#include "control_widget.h"

#include "qecalrec.h"

#include <QIcon>
#include <QMenu>
#include <QAction>

ControlWidget::ControlWidget(QWidget *parent)
  : QWidget(parent)
  , first_resize_event_(true)
  , record_button_state_is_record_(true)
  , activate_button_state_is_activate_(true)
{
  ui_.setupUi(this);

  // Record Button
  connect(ui_.record_button, &QAbstractButton::clicked, QEcalRec::instance(),
      [this]()
      {
        if (record_button_state_is_record_)
          QEcalRec::instance()->startRecording();
        else
          QEcalRec::instance()->stopRecording();
      });
  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,    this, &ControlWidget::updateRecordButton);
  connect(QEcalRec::instance(), &QEcalRec::enabledRecClientsChangedSignal, this, &ControlWidget::updateRecordButton);

  // Save Buffer button
  connect(ui_.save_buffer_button, &QAbstractButton::clicked, QEcalRec::instance(), []() {QEcalRec::instance()->savePreBufferedData(); });

  connect(QEcalRec::instance(), &QEcalRec::preBufferingEnabledChangedSignal,  this, &ControlWidget::updateSaveBufferButton);
  connect(QEcalRec::instance(), &QEcalRec::connectionToClientsActiveChangedSignal,    this, &ControlWidget::updateSaveBufferButton);
  connect(QEcalRec::instance(), &QEcalRec::connectedToEcalStateChangedSignal, this, &ControlWidget::updateSaveBufferButton);
  connect(QEcalRec::instance(), &QEcalRec::enabledRecClientsChangedSignal,    this, &ControlWidget::updateSaveBufferButton);

  // Activate button
  connect(ui_.activate_button, &QAbstractButton::clicked, QEcalRec::instance(),
      [this]()
      {
        if (activate_button_state_is_activate_)
          QEcalRec::instance()->connectToEcal();
        else
          QEcalRec::instance()->setConnectionToClientsActive(false);
      });
  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,       this, &ControlWidget::updateActivateButton);
  connect(QEcalRec::instance(), &QEcalRec::enabledRecClientsChangedSignal,    this, &ControlWidget::updateActivateButton);
  connect(QEcalRec::instance(), &QEcalRec::connectedToEcalStateChangedSignal, this, &ControlWidget::updateActivateButton);
  connect(QEcalRec::instance(), &QEcalRec::connectionToClientsActiveChangedSignal,    this, &ControlWidget::updateActivateButton);


  // initial state
  updateRecordButton();
  updateSaveBufferButton();
  updateActivateButton();
}

ControlWidget::~ControlWidget()
{}

void ControlWidget::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  if (first_resize_event_)
  {
    ui_.activate_button   ->setMinimumWidth(ui_.activate_button->sizeHint().width());
    ui_.save_buffer_button->setMinimumWidth(ui_.save_buffer_button->sizeHint().width());

    first_resize_event_ = false;
  }
}

void ControlWidget::updateRecordButton()
{
  if (QEcalRec::instance()->recording() && record_button_state_is_record_)
  {
    ui_.record_button->setIcon(QIcon(":/ecalicons/STOP_TEXT"));
    ui_.record_button->setText(tr("Stop"));
    ui_.record_button->setToolTip(tr("Stop recording"));
    ui_.record_button->setEnabled(true);
    record_button_state_is_record_ = false;
  }
  else if (!QEcalRec::instance()->recording())
  {
    if (!record_button_state_is_record_)
    {
      ui_.record_button->setIcon(QIcon(":/ecalicons/RECORD_TEXT"));
      ui_.record_button->setText(tr("Record"));
      ui_.record_button->setToolTip(tr("Start recording"));
      record_button_state_is_record_ = true;
    }

    ui_.record_button->setEnabled(QEcalRec::instance()->enabledRecClients().size() > 0);
  }
}

void ControlWidget::updateSaveBufferButton()
{
  ui_.save_buffer_button->setEnabled(QEcalRec::instance()->preBufferingEnabled()
    && QEcalRec::instance()->connectionToClientsActive()
    && QEcalRec::instance()->connectedToEcal()
    && QEcalRec::instance()->enabledRecClients().size() > 0);
}

void ControlWidget::updateActivateButton()
{
  // Tool button
  if ((!QEcalRec::instance()->connectionToClientsActive() || !QEcalRec::instance()->connectedToEcal())
    && !activate_button_state_is_activate_)
  {
    ui_.activate_button->setIcon(QIcon(":/ecalicons/POWER_ON"));
    ui_.activate_button->setText(tr("Activate / Prepare"));
    ui_.activate_button->setToolTip(tr("Activate clients and start pre-buffering"));
    activate_button_state_is_activate_ = true;
  }
  else if ((QEcalRec::instance()->connectionToClientsActive() && QEcalRec::instance()->connectedToEcal())
    && activate_button_state_is_activate_)
  {
    ui_.activate_button->setIcon(QIcon(":/ecalicons/POWER_OFF"));
    ui_.activate_button->setText(tr("De-activate"));
    ui_.activate_button->setToolTip(tr("De-activate clients and stop pre-buffering"));
    activate_button_state_is_activate_ = false;
  }

  ui_.activate_button    ->setEnabled(!QEcalRec::instance()->recording());
}