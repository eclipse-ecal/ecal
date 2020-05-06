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

#include <ecal/mon/plugin.h>
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/dynamic_subscriber.h>

#include <memory>
#include <mutex>

#include <CustomQt/QAdvancedTreeView.h>

#include "monitor_tree_model.h"

#include "ui_plugin_widget.h"

#include <ecal/protobuf/ecal_proto_decoder.h>
#include <protobuf_tree_builder.h>

class PluginWidget : public QWidget, public eCAL::mon::PluginWidgetInterface
{
  Q_OBJECT
public:
  PluginWidget(const QString& topic_name, const QString& topic_type, QWidget* parent = Q_NULLPTR);
  virtual ~PluginWidget();

  virtual QWidget* getWidget();

public slots:
  virtual void onUpdate();

  virtual void onResume();
  virtual void onPause();

protected:
  Ui::PluginWidget ui_;

private slots:
  void updateTree();

  void contextMenu(const QPoint &pos);
  void copyCurrentIndexToClipboard() const;
  void copySelectedRowToClipboard() const;

private:
  QAdvancedTreeView* tree_view_;
  MonitorTreeModel* tree_model_;

  eCAL::protobuf::CDynamicSubscriber subscriber_;
  eCAL::protobuf::CProtoDecoder protobuf_decoder;
  std::shared_ptr<ProtobufTreeBuilder> protobuf_tree_builder;

  std::mutex                         proto_message_mutex_;
  google::protobuf::Message*         last_proto_message_;
  eCAL::Time::ecal_clock::time_point last_message_publish_timestamp_;
  QString                            last_error_string_;
  bool                               last_message_was_error_;

  bool                               currently_showing_error_item_;
  int                                error_counter_;

  QByteArray                         initial_tree_state_;

  QString                            topic_name_, topic_type_;
  bool                               new_msg_available_;
  int                                received_message_counter_;

  void onProtoMessageCallback(const google::protobuf::Message& message, long long send_time_usecs);
  void onProtoErrorCallback(const std::string& error);
  void updatePublishTimeLabel();

  
};
