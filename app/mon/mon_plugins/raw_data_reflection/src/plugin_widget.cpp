/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

  // Setup frame
  QFrame* frame = new QFrame(this);
  frame->setFrameShape(QFrame::StyledPanel);
  frame->setFrameStyle(QFrame::Plain);
  QLayout* frame_layout = new QVBoxLayout(this);
  frame_layout->setContentsMargins(QMargins(0, 0, 0, 0));
  frame->setLayout(frame_layout);

  size_label_ = new QLabel(this);
  size_label_->setText("-- No messages received, yet --");
  frame_layout->addWidget(size_label_);

  blob_text_edit_ = new QPlainTextEdit(this);
  blob_text_edit_->setFont(QFont("Courier New"));
  blob_text_edit_->setReadOnly(true);
  frame_layout->addWidget(blob_text_edit_);

  ui_.content_layout->addWidget(frame);

  // Connect the eCAL Subscriber
  subscriber_.SetReceiveCallback([this](const eCAL::STopicId& /*topic_id*/,
    const eCAL::SDataTypeInformation& /*data_type_info*/,
    const eCAL::SReceiveCallbackData& callback_data)
    {
      ecalMessageReceivedCallback(callback_data);
    });
}

PluginWidget::~PluginWidget()
{
  subscriber_.RemoveReceiveCallback();
}

void PluginWidget::ecalMessageReceivedCallback(const eCAL::SReceiveCallbackData& callback_data)
{
  std::lock_guard<std::mutex> message_lock(message_mutex_);
  last_message_ = QByteArray(static_cast<const char*>(callback_data.buffer), callback_data.buffer_size);

  last_message_publish_timestamp_ = eCAL::Time::ecal_clock::time_point(std::chrono::microseconds(callback_data.send_timestamp));

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
    updateRawMessageView();
    updatePublishTimeLabel();
    ui_.received_message_counter_label->setText(QString::number(received_message_counter_));
  }
}

void PluginWidget::onResume()
{
  // (Re)Connect the eCAL Subscriber
  subscriber_.SetReceiveCallback([this](const eCAL::STopicId& /*topic_id*/,
    const eCAL::SDataTypeInformation& /*data_type_info*/,
    const eCAL::SReceiveCallbackData& callback_data)
    {
      ecalMessageReceivedCallback(callback_data);
    });
}

void PluginWidget::onPause()
{
  subscriber_.RemoveReceiveCallback();
}

void PluginWidget::updateRawMessageView()
{
  std::lock_guard<std::mutex> message_lock(message_mutex_);


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  const quint16 crc16 = qChecksum(last_message_.data(), last_message_.length());
#else
  const quint16 crc16 = qChecksum(last_message_);
#endif

  QString crc16_string = QString("%1").arg(QString::number(crc16, 16).toUpper(), 4, '0');
  QString size_text = tr("Binary data of ") + QString::number(last_message_.size()) + tr(" bytes (CRC16: ") + crc16_string + ")";

  size_label_->setText(size_text);

  QByteArray const last_message_trimmed(last_message_.data(), std::min(static_cast<std::size_t>(last_message_.length()), std::size_t(1024)));
  blob_text_edit_->setPlainText(
#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
    bytesToHex(last_message_trimmed, ' ')
#else //QT_VERSION
    last_message_trimmed.toHex(' ')
#endif // QT_VERSION
    + (last_message_trimmed.length() < last_message_.length() ? QString("...") : QString())
  );

  new_msg_available_ = false;
}

QWidget* PluginWidget::getWidget()
{
  return this;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
QString PluginWidget::bytesToHex(const QByteArray& byte_array, char separator)
{
  QString hex_string;
  hex_string.reserve(byte_array.size() * 2 + (separator ? byte_array.size() : 0));

  for (int i = 0; i < byte_array.size(); i++)
  {
    QByteArray temp_array;
    temp_array.push_back(byte_array[i]);
    hex_string += temp_array.toHex();
    if (separator)
    {
      hex_string += separator;
    }
  }
  return hex_string;
}
#endif //QT_VERSION