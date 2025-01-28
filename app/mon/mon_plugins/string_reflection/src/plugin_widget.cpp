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

#include "plugin_widget.h"

#include <QFrame>
#include <QLayout>
#include <QFont>

#include <QDebug>

PluginWidget::PluginWidget(const QString& topic_name, const QString&, QWidget* parent): QWidget(parent),
  subscriber_(topic_name.toStdString()),
  last_message_publish_timestamp_(eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(-1))),
  new_msg_available_(false),
  received_message_counter_(0)
{
  ui_.setupUi(this);

  // Timestamp warning
  int label_height = ui_.publish_timestamp_warning_label->sizeHint().height();
  QPixmap warning_icon = QPixmap(":/ecalicons/WARNING").scaled(label_height, label_height, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
  ui_.publish_timestamp_warning_label->setPixmap(warning_icon);
  ui_.publish_timestamp_warning_label->setVisible(false);

  // Create a text edit that displays the received messages
  text_edit_ = new QTextEdit(this);
  text_edit_->setReadOnly(true);
  text_edit_->setText("-- No messages received, yet --");
  ui_.content_layout->addWidget(text_edit_);

  // Connect the eCAL Subscriber
  subscriber_.SetReceiveCallback(std::bind(&PluginWidget::ecalMessageReceivedCallback, this, std::placeholders::_2, std::placeholders::_3));
}

PluginWidget::~PluginWidget()
{
  subscriber_.RemoveReceiveCallback();
}

void PluginWidget::ecalMessageReceivedCallback(const std::string& message, long long publish_timestamp_usecs)
{
  std::lock_guard<std::mutex> message_lock(message_mutex_);
  last_message_ = message;

  last_message_publish_timestamp_ = eCAL::Time::ecal_clock::time_point(std::chrono::microseconds(publish_timestamp_usecs));

  received_message_counter_++;
  new_msg_available_ = true;
}

void PluginWidget::updatePublishTimeLabel()
{
  eCAL::Time::ecal_clock::time_point publish_time = last_message_publish_timestamp_;
  eCAL::Time::ecal_clock::time_point receive_time = eCAL::Time::ecal_clock::now();

  if (publish_time < eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(0)))
    return;

  auto diff = receive_time - publish_time;

  if ((diff > std::chrono::milliseconds(100))
    || (diff < std::chrono::milliseconds(-100)))
  {
    ui_.publish_timestamp_warning_label->setVisible(true);
    QString diff_string = QString::number(std::chrono::duration_cast<std::chrono::duration<double>>(diff).count(), 'f', 6);
    ui_.publish_timestamp_warning_label->setToolTip(tr("The publisher is not synchronized, properly.\nCurrent time difference: ") + diff_string + " s");
  }
  else
  {
    ui_.publish_timestamp_warning_label->setVisible(false);
  }

  QString time_string;

  //if (parse_time_)
  //{
  //  QDateTime q_ecal_time = QDateTime::fromMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(publish_time.time_since_epoch()).count()).toUTC();
  //  time_string = q_ecal_time.toString("yyyy-MM-dd HH:mm:ss.zzz");
  //}
  //else
  {
    double seconds_since_epoch = std::chrono::duration_cast<std::chrono::duration<double>>(publish_time.time_since_epoch()).count();
    time_string = QString::number(seconds_since_epoch, 'f', 6) + " s";
  }

  ui_.publish_timestamp_label->setText(time_string);
}

void PluginWidget::onUpdate()
{
  if (new_msg_available_)
  {
    updateStringMessageView();
    updatePublishTimeLabel();
    ui_.received_message_counter_label->setText(QString::number(received_message_counter_));
  }
}

void PluginWidget::onResume()
{
  subscriber_.SetReceiveCallback(std::bind(&PluginWidget::ecalMessageReceivedCallback, this, std::placeholders::_2, std::placeholders::_3));
}

void PluginWidget::onPause()
{
  subscriber_.RemoveReceiveCallback();
}

void PluginWidget::updateStringMessageView()
{
  std::lock_guard<std::mutex> message_lock(message_mutex_);
  text_edit_->setText(last_message_.c_str());
  new_msg_available_ = false;
}

QWidget* PluginWidget::getWidget()
{
  return this;
}
