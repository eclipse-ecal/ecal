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
#include <cfloat>
#include <time.h>
#include <stdlib.h> 

#include <CustomQt/QAdvancedTreeView.h>
#include <CustomQt/QStandardTreeItem.h>

#include <QDoubleValidator>
#include <QClipboard>
#include <QMenu>
#include <QDebug>

#include "signal_tree_model.h"
#include "signal_tree_item.h"
#include "tabwidget_container.h"
#include "chart_widget.h"
#include "chart_settings.h"
#include "util.h"

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

  void slt_on_itemClicked(const QModelIndex& index);
  void slt_on_valueSignalChanged(QString& value, QString& full_signal_name, SignalTreeItem::Columns column_number);

signals:
  void newItemChecked(const QString& full_signal_name, int model_column);
  void removeItemChecked(const QString& full_signal_name, int model_column);
  void sgn_updateTree();
  void sgn_errorMessageReceived();

protected:
  Ui::PluginWidget ui_;

private slots:
  void updateTree();
  void showErrorMessage();

  void contextMenu(const QPoint& pos);
  void copyCurrentIndexToClipboard() const;
  void copySelectedRowToClipboard() const;

  void showContextMenu(const QPoint& pos);
  void closeTab();
  void removeCurveFromTabCloseButton(QAction* action);
  void removeCurveFromTab(const QString& full_signal_name, int column_number, int tab_index);
  void changeChartSettings();
  void updateCurveColors();

private:
  QAdvancedTreeView* tree_view_;
  SignalTreeModel* tree_model_;
  QTabWidget* plotting_tabbed_widget_;
  TabWidgetContainer tabwidget_container_;
  QTextEdit* logging_text_edit_;

  eCAL::protobuf::CDynamicSubscriber subscriber_;
  eCAL::protobuf::CProtoDecoder protobuf_decoder;
  std::shared_ptr<ProtobufTreeBuilder> protobuf_tree_builder;

  std::mutex                         proto_message_mutex_;
  google::protobuf::Message*         last_proto_message_;
  eCAL::Time::ecal_clock::time_point last_message_publish_timestamp_;
  QString                            last_error_string_;
  bool                               last_message_was_error_;
  int                                error_counter_;
  SignalPlotting::PLUGIN_STATE       plugin_state_;

  QByteArray                         initial_tree_state_;

  QString                            topic_name_, topic_type_;
  bool                               new_msg_available_;
  int                                received_message_counter_;
  QList<QString>                     rounded_signals_names_;

  QString                            key_to_close_;

  void   onProtoMessageCallback(const google::protobuf::Message& message, long long send_time_usecs);
  void   onProtoErrorCallback(const std::string& error);
  void   updatePublishTimeLabel();
  bool   find_items(QAbstractTreeItem* tree_item);
  QColor generateRandomColor(); 
  void   changeCloseButtonState(const QString& signal_name);
  void   setVisibleSplitterHandle(bool state);
};
