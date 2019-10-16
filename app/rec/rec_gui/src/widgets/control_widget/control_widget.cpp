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
  , record_button_state_is_record_(true)
  , activate_button_state_is_activate_(true)
  , enable_client_connections_action_state_is_enable_(true)
  , connect_to_ecal_action_state_is_connect_(true)
{
  ui_.setupUi(this);

  // Record Button
  connect(ui_.record_button, &QAbstractButton::clicked, QEcalRec::instance(),
      [this]()
      {
        if (record_button_state_is_record_)
          QEcalRec::instance()->sendRequestStartRecording();
        else
          QEcalRec::instance()->sendRequestStopRecording();
      });
  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,    this, &ControlWidget::updateRecordButton);
  connect(QEcalRec::instance(), &QEcalRec::recorderInstancesChangedSignal, this, &ControlWidget::updateRecordButton);

  // Save Buffer button
  connect(ui_.save_buffer_button, &QAbstractButton::clicked, QEcalRec::instance(), []() {QEcalRec::instance()->sendRequestSavePreBufferedData(); });

  connect(QEcalRec::instance(), &QEcalRec::preBufferingEnabledChangedSignal,  this, &ControlWidget::updateSaveBufferButton);
  connect(QEcalRec::instance(), &QEcalRec::clientConnectionsEnabledSignal,    this, &ControlWidget::updateSaveBufferButton);
  connect(QEcalRec::instance(), &QEcalRec::connectedToEcalStateChangedSignal, this, &ControlWidget::updateSaveBufferButton);
  connect(QEcalRec::instance(), &QEcalRec::recorderInstancesChangedSignal,    this, &ControlWidget::updateSaveBufferButton);

  // Activate button
  activate_button_menu_ = new QMenu(this);
  enable_client_connections_action_ = new QAction(activate_button_menu_);
  connect_to_ecal_action_ = new QAction(activate_button_menu_);
  activate_button_menu_->addAction(enable_client_connections_action_);
  activate_button_menu_->addAction(connect_to_ecal_action_);
  ui_.activate_button->setMenu(activate_button_menu_);

  connect(ui_.activate_button, &QAbstractButton::clicked, QEcalRec::instance(),
      [this]()
      {
        if (activate_button_state_is_activate_)
          QEcalRec::instance()->sendRequestConnectToEcal();
        else
          QEcalRec::instance()->initiateConnectionShutdown();
      });
  connect(enable_client_connections_action_, &QAction::triggered, QEcalRec::instance(),
      [this]()
      {
        if (enable_client_connections_action_state_is_enable_)
          QEcalRec::instance()->setClientConnectionsEnabled(true);
        else
          QEcalRec::instance()->setClientConnectionsEnabled(false);
      });
  connect(connect_to_ecal_action_, &QAction::triggered, QEcalRec::instance(),
      [this]()
      {
        if (connect_to_ecal_action_state_is_connect_)
          QEcalRec::instance()->sendRequestConnectToEcal();
        else
          QEcalRec::instance()->sendRequestDisconnectFromEcal();
      });
  connect(QEcalRec::instance(), &QEcalRec::recordingStateChangedSignal,       this, &ControlWidget::updateActivateButton);
  connect(QEcalRec::instance(), &QEcalRec::recorderInstancesChangedSignal,    this, &ControlWidget::updateActivateButton);
  connect(QEcalRec::instance(), &QEcalRec::connectedToEcalStateChangedSignal, this, &ControlWidget::updateActivateButton);
  connect(QEcalRec::instance(), &QEcalRec::clientConnectionsEnabledSignal,    this, &ControlWidget::updateActivateButton);

  // initial state
  updateRecordButton();
  updateSaveBufferButton();
  updateActivateButton();
}

ControlWidget::~ControlWidget()
{}

void ControlWidget::updateRecordButton()
{
  if (QEcalRec::instance()->recordersRecording() && record_button_state_is_record_)
  {
    ui_.record_button->setIcon(QIcon(":/ecalicons/STOP_TEXT"));
    ui_.record_button->setText(tr("Stop"));
    ui_.record_button->setToolTip(tr("Stop recording"));
    ui_.record_button->setEnabled(true);
    record_button_state_is_record_ = false;
  }
  else if (!QEcalRec::instance()->recordersRecording())
  {
    if (!record_button_state_is_record_)
    {
      ui_.record_button->setIcon(QIcon(":/ecalicons/RECORD_TEXT"));
      ui_.record_button->setText(tr("Record"));
      ui_.record_button->setToolTip(tr("Start recording"));
      record_button_state_is_record_ = true;
    }

    ui_.record_button->setEnabled(QEcalRec::instance()->recorderInstances().size() > 0);
  }
}

void ControlWidget::updateSaveBufferButton()
{
  ui_.save_buffer_button->setEnabled(QEcalRec::instance()->preBufferingEnabled()
    && QEcalRec::instance()->clientConnectionsEnabled()
    && QEcalRec::instance()->recordersConnectedToEcal()
    && QEcalRec::instance()->recorderInstances().size() > 0);
}

void ControlWidget::updateActivateButton()
{
  // Tool button
  if ((!QEcalRec::instance()->clientConnectionsEnabled() || !QEcalRec::instance()->recordersConnectedToEcal())
    && !activate_button_state_is_activate_)
  {
    ui_.activate_button->setIcon(QIcon(":/ecalicons/POWER_ON"));
    ui_.activate_button->setText(tr("Activate"));
    ui_.activate_button->setToolTip(tr("Connect to clients and initialize them"));
    activate_button_state_is_activate_ = true;
  }
  else if ((QEcalRec::instance()->clientConnectionsEnabled() && QEcalRec::instance()->recordersConnectedToEcal())
    && activate_button_state_is_activate_)
  {
    ui_.activate_button->setIcon(QIcon(":/ecalicons/POWER_OFF"));
    ui_.activate_button->setText(tr("De-activate"));
    ui_.activate_button->setToolTip(tr("De-initialize clients and disconnect from them"));
    activate_button_state_is_activate_ = false;
  }

  ui_.activate_button    ->setEnabled(!QEcalRec::instance()->recordersRecording());
  enable_client_connections_action_->setEnabled(!QEcalRec::instance()->recordersRecording());
  connect_to_ecal_action_->setEnabled(!QEcalRec::instance()->recordersRecording());

  // Menu
  if (QEcalRec::instance()->clientConnectionsEnabled())
  {
    enable_client_connections_action_->setText(tr("Disconnect from clients"));
    enable_client_connections_action_->setIcon(QIcon(":/ecalicons/DISCONNECTED"));
    enable_client_connections_action_state_is_enable_ = false;
  }
  else if (!QEcalRec::instance()->clientConnectionsEnabled())
  {
    enable_client_connections_action_->setText(tr("Connect to clients"));
    enable_client_connections_action_->setIcon(QIcon(":/ecalicons/CONNECTED"));
    enable_client_connections_action_state_is_enable_ = true;
  }

  if (QEcalRec::instance()->recordersConnectedToEcal())
  {
    connect_to_ecal_action_->setText(tr("Disconnect from eCAL"));
    connect_to_ecal_action_->setIcon(QIcon(":/ecalicons/CROSS"));
    connect_to_ecal_action_state_is_connect_ = false;
  }
  else if (!QEcalRec::instance()->recordersConnectedToEcal())
  {
    connect_to_ecal_action_->setText(tr("Connect to eCAL"));
    connect_to_ecal_action_->setIcon(QIcon(":/ecalicons/CHECKMARK"));
    connect_to_ecal_action_state_is_connect_ = true;
  }
}