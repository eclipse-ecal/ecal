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
#include "util.h"

#include <CustomQt/QStandardTreeItem.h>

#include <QClipboard>
#include <QMenu>
#include <QDebug>

#include "capnproto_tree_builder.h"

PluginWidget::PluginWidget(const QString& topic_name, const QString& topic_type, QWidget* parent):
  QWidget(parent),
  tree_view_(new QAdvancedTreeView(this)),
  tree_model_(new MonitorTreeModel(this)),
  subscriber_(topic_name.toStdString()),
  capnproto_message_builder(nullptr),
  capnproto_message_iterator(tree_model_),
  last_message_publish_timestamp_(eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(-1))),
  last_error_string_(""),
  last_message_was_error_(false),
  currently_showing_error_item_(true),
  error_counter_(0),
  topic_name_(topic_name),
  topic_type_(topic_type),
  new_msg_available_(false),
  received_message_counter_(0)
{
  ui_.setupUi(this);

  // Create the content tree
  tree_view_->setModel(tree_model_);
  ui_.content_layout->addWidget(tree_view_);

  // Timestamp warning
  int label_height = ui_.publish_timestamp_warning_label->sizeHint().height();
  QPixmap warning_icon = QPixmap(":/ecalicons/WARNING").scaled(label_height, label_height, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
  ui_.publish_timestamp_warning_label->setPixmap(warning_icon);
  ui_.publish_timestamp_warning_label->setVisible(false);

  // Add eCAL Callbacks
  subscriber_.SetReceiveCallback(std::bind(&PluginWidget::onProtoMessageCallback, this, std::placeholders::_2, std::placeholders::_3));
  subscriber_.SetErrorCallback(std::bind(&PluginWidget::onProtoErrorCallback, this, std::placeholders::_1));

  // Button connections
  connect(ui_.expand_button, &QPushButton::clicked, [this]() { tree_view_->expandAll();   });
  connect(ui_.collapse_button, &QPushButton::clicked, [this]() { tree_view_->collapseAll(); });
  connect(ui_.display_blobs_checkbox, &QCheckBox::stateChanged, [this](int state) { tree_model_->setDisplayBlobs(state == Qt::CheckState::Checked); });

  // Set default visible columns
  QList<int> visible_columns
  {
    (int)MonitorTreeModel::Columns::FIELD_NAME,
    (int)MonitorTreeModel::Columns::TYPE_NAME,
    (int)MonitorTreeModel::Columns::VALUE
  };
  for (int i = 0; i < tree_model_->columnCount(); i++)
  {
    tree_view_->setColumnHidden(i, !visible_columns.contains(i));
  }

  // Save the initial state
  initial_tree_state_ = tree_view_->saveState();

  // TODO: Save/Restore States

  // Add some text that tells the user that no data has been received
  QStandardTreeItem* never_received_data_item = new QStandardTreeItem();
  never_received_data_item->setData(0, "Haven't received any data on topic \"" + topic_name + "\", yet", Qt::ItemDataRole::DisplayRole);
  never_received_data_item->setData(0, QColor(127, 127, 127), Qt::ItemDataRole::ForegroundRole);
  tree_model_->insertItem(never_received_data_item);
  tree_view_->setFirstColumnSpanned(0, QModelIndex(), true);

  // Context menu
  tree_view_->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(tree_view_, &QTreeView::customContextMenuRequested, this, &PluginWidget::contextMenu);
  connect(tree_view_, &QAdvancedTreeView::keySequenceCopyPressed, this, &PluginWidget::copySelectedRowToClipboard);
}

PluginWidget::~PluginWidget() noexcept
{
#ifndef NDEBUG
  qDebug().nospace() << "[" << metaObject()->className() << "]: Deleting Widget for topic " << topic_name_;
#endif // NDEBUG

  subscriber_.RemoveReceiveCallback();
  //subscriber_.RemoveErrorCallback();
  
  {
    std::lock_guard<std::mutex> lock(capnproto_message_mutex_);
    if (capnproto_message_builder)
      delete capnproto_message_builder;
  }
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

////////////////////////////////////////////////////////////////////////////////
//// Tree View Update                                                       ////
////////////////////////////////////////////////////////////////////////////////

// eCAL Callback
void PluginWidget::onProtoMessageCallback(const capnp::DynamicStruct::Reader& message, long long send_time_usecs)
{
  {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(capnproto_message_mutex_);

    // Create a copy of the new message as member variable. We cannot use a reference here, as this may cause a deadlock with the GUI thread
    if (!capnproto_message_builder)
    {
      capnproto_message_builder = new capnp::MallocMessageBuilder();
    }
    capnproto_message_builder->setRoot(message);
    schema = message.getSchema();

    last_message_publish_timestamp_ = eCAL::Time::ecal_clock::time_point(std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::microseconds(send_time_usecs)));

    last_message_was_error_ = false;
    error_counter_ = 0;

    new_msg_available_ = true;
    received_message_counter_++;
  }
}

// eCAL Callback
void PluginWidget::onProtoErrorCallback(const std::string& error)
{
  {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(capnproto_message_mutex_);

    // Copy the message into a member variable
    last_error_string_ = error.c_str();

    last_message_was_error_ = true;
    error_counter_++;

    new_msg_available_ = true;
    received_message_counter_++;
  }
}

// Actual Tree Update
void PluginWidget::updateTree()
{
  std::lock_guard<std::mutex> lock(capnproto_message_mutex_);

  if (last_message_was_error_)
  {
    // If the last message was an error, we remove everything from the
    // tree view and replace it with a dummy error-item. We also log an eCAL
    // error message.

    eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_error, "Error when receiving data on topic \"" + topic_name_.toStdString() + "\": " + last_error_string_.toStdString());

    tree_model_->removeAllChildren();
    currently_showing_error_item_ = true;
    QStandardTreeItem* error_item = new QStandardTreeItem();
    error_item->setData(0, "ERROR [" + QString::number(error_counter_) + "]: " + last_error_string_, Qt::ItemDataRole::DisplayRole);
    error_item->setData(0, QColor(192, 0, 0), Qt::ItemDataRole::ForegroundRole);
    tree_model_->insertItem(error_item);
    tree_view_->setFirstColumnSpanned(0, QModelIndex(), true);
  }
  else
  {
    // If the last message was a valid message, we display that message in the
    // tree view. If the tree view is currently showing a dummy error message,
    // we have to remove it first.

    if (currently_showing_error_item_)
    {
      tree_model_->removeAllChildren();
      currently_showing_error_item_ = false;
    }

    // TODO MORGEN
    auto builder = capnproto_message_builder->getRoot<capnp::DynamicStruct>(schema);
    capnproto_message_iterator.ProcessMessage(builder.asReader());
  }

  new_msg_available_ = false;
}

////////////////////////////////////////////////////////////////////////////////
//// Context Menu                                                           ////
////////////////////////////////////////////////////////////////////////////////

void PluginWidget::contextMenu(const QPoint &pos)
{
  auto selected_rows = tree_view_->selectionModel()->selectedRows((int)MonitorTreeModel::Columns::FIELD_NAME);
  if (selected_rows.size() > 0)
  {
    QMenu context_menu(this);

    QVariant header_data_variant = tree_view_->model()->headerData(tree_view_->currentIndex().column(), Qt::Orientation::Horizontal, Qt::ItemDataRole::DisplayRole);
    QString header_data = QtUtil::variantToString(header_data_variant);

    QAction* copy_element_action = new QAction(tr("Copy element (") + header_data + ")", &context_menu);
    QAction* copy_row_action = new QAction(tr("Copy row"), &context_menu);

    connect(copy_element_action, &QAction::triggered, this, &PluginWidget::copyCurrentIndexToClipboard);
    connect(copy_row_action, &QAction::triggered, this, &PluginWidget::copySelectedRowToClipboard);

    context_menu.addAction(copy_element_action);
    context_menu.addAction(copy_row_action);

    context_menu.exec(tree_view_->viewport()->mapToGlobal(pos));
  }
}

void PluginWidget::copyCurrentIndexToClipboard() const
{
  auto selected_rows = tree_view_->selectionModel()->selectedRows((int)MonitorTreeModel::Columns::FIELD_NAME);;

  if (selected_rows.size() > 0)
  {
    QModelIndex selected_row = selected_rows.first();

    QModelIndex selected_index = tree_view_->model()->index(selected_row.row(), tree_view_->currentIndex().column(), selected_row.parent());

    QVariant variant_data = tree_view_->model()->data(selected_index, Qt::ItemDataRole::DisplayRole);

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(QtUtil::variantToString(variant_data));
  }
}

void PluginWidget::copySelectedRowToClipboard() const
{
  auto selected_rows = tree_view_->selectionModel()->selectedRows((int)MonitorTreeModel::Columns::FIELD_NAME);;

  if (selected_rows.size() > 0)
  {
    auto selected_row = selected_rows.first();
    QString clipboard_string;

    for (int column = 0; column < tree_view_->model()->columnCount(); column++)
    {
      if (!tree_view_->isColumnHidden(column))
      {
        QModelIndex index = tree_view_->model()->index(selected_row.row(), column, selected_row.parent());
        QVariant variant_data = tree_view_->model()->data(index, Qt::ItemDataRole::DisplayRole);
        if (!clipboard_string.isEmpty())
        {
          clipboard_string += '\t';
        }
        clipboard_string += QtUtil::variantToString(variant_data);
      }
    }

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(clipboard_string);
  }
}
void PluginWidget::onUpdate()
{
  if (new_msg_available_)
  {
    updateTree();
    updatePublishTimeLabel();
    ui_.received_message_counter_label->setText(QString::number(received_message_counter_));
  }
}

void PluginWidget::onResume()
{
  // Add eCAL Callbacks
  subscriber_.SetReceiveCallback(std::bind(&PluginWidget::onProtoMessageCallback, this, std::placeholders::_2, std::placeholders::_3));
  subscriber_.SetErrorCallback(std::bind(&PluginWidget::onProtoErrorCallback, this, std::placeholders::_1));
}

void PluginWidget::onPause()
{
  subscriber_.RemoveReceiveCallback();
  subscriber_.RemoveErrorCallback();
}

QWidget* PluginWidget::getWidget()
{
  return this;
}
