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

PluginWidget::PluginWidget(const QString& topic_name, const QString& topic_type, QWidget* parent) :
  QWidget(parent),
  logging_text_edit_(nullptr),
  subscriber_(topic_name.toStdString()),
  last_proto_message_(nullptr),
  last_message_publish_timestamp_(eCAL::Time::ecal_clock::time_point(eCAL::Time::ecal_clock::duration(-1))),
  last_error_string_(""),
  last_message_was_error_(false),
  error_counter_(0),
  plugin_state_(SignalPlotting::PLUGIN_STATE::RUNNING),
  topic_name_(topic_name),
  topic_type_(topic_type),
  new_msg_available_(false),
  received_message_counter_(0)
{
  ui_.setupUi(this);

  // Create logging layout
  logging_text_edit_ = new QTextEdit(this);
  ui_.logging_layout->addWidget(logging_text_edit_);
  logging_text_edit_->setEnabled(false);
  // Create the content tree
  tree_model_ = new SignalTreeModel(this);
  tree_view_ = new QAdvancedTreeView(this);
  tree_view_->setModel(tree_model_);
  ui_.treeview_layout->addWidget(tree_view_);

  // Create the plotting widget
  plotting_tabbed_widget_ = new QTabWidget(this);
  ui_.widget_plot_layout->addWidget(plotting_tabbed_widget_);
  plotting_tabbed_widget_->hide();
 
  int largeWidth = QGuiApplication::primaryScreen()->size().width();
  ui_.splitter->setSizes(QList<int>({ largeWidth , largeWidth }));
  setVisibleSplitterHandle(false);

  auto index_tree_area = ui_.splitter->indexOf(ui_.treeview_area);
  ui_.splitter->setCollapsible(index_tree_area, false);
  ui_.treeview_area->setMinimumWidth(1);

  // Add context menu
  plotting_tabbed_widget_->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(plotting_tabbed_widget_->tabBar(), &QTabBar::customContextMenuRequested, this, &PluginWidget::showContextMenu);

  protobuf_tree_builder = std::make_shared<ProtobufTreeBuilder>(tree_model_);
  protobuf_decoder.SetVisitor(protobuf_tree_builder);

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

  // Set default visible columns
  QList<int> visible_columns
  {
    (int)SignalTreeModel::Columns::FIELD_NAME,
    (int)SignalTreeModel::Columns::TYPE_NAME,
    (int)SignalTreeModel::Columns::VALUE,
    (int)SignalTreeModel::Columns::MULTI,
    (int)SignalTreeModel::Columns::SINGLE
  };
  for (int i = 0; i < tree_model_->columnCount(); i++)
  {
    tree_view_->setColumnHidden(i, !visible_columns.contains(i));
  }

  // Save the initial state
  initial_tree_state_ = tree_view_->saveState();

  connect(this, &PluginWidget::sgn_updateTree, this, &PluginWidget::onUpdate, Qt::QueuedConnection);
  connect(this, &PluginWidget::sgn_errorMessageReceived,this,&PluginWidget::showErrorMessage, Qt::QueuedConnection);

  // Add some text that tells the user that no data has been received
  logging_text_edit_->append("  Haven't received any data on topic \"" + topic_name + "\", yet.");
  logging_text_edit_->setVisible(true);
  tree_view_->setVisible(false);

  // Context menu
  tree_view_->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(tree_view_, &QTreeView::customContextMenuRequested, this, &PluginWidget::contextMenu);
  connect(tree_view_, &QAdvancedTreeView::keySequenceCopyPressed, this, &PluginWidget::copySelectedRowToClipboard);
  connect(tree_view_, &QTreeView::clicked, this, &PluginWidget::slt_on_itemClicked);
  connect(tree_model_, &SignalTreeModel::plottedSignalValueChanged, this, &PluginWidget::slt_on_valueSignalChanged);

  connect(this, &PluginWidget::newItemChecked, tree_model_, &SignalTreeModel::appendItemChecked);
  connect(this, &PluginWidget::removeItemChecked, tree_model_, &SignalTreeModel::removeItemChecked);
  connect(tree_model_, &SignalTreeModel::restoreState, this, &PluginWidget::slt_on_itemClicked);

  srand(time(0));
}

PluginWidget::~PluginWidget()
{
#ifndef NDEBUG
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  qDebug().nospace() << "[" << PluginWidget::metaObject()->className() << "]: Deleting Widget for topic " << topic_name_;
#endif // NDEBUG

  subscriber_.RemoveReceiveCallback();
  subscriber_.RemoveErrorCallback();

  {
    std::lock_guard<std::mutex> lock(proto_message_mutex_);
    last_proto_message_.reset();
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

QColor PluginWidget::generateRandomColor()
{
  auto r = (double)std::rand() / RAND_MAX * 255;
  auto g = (double)std::rand() / RAND_MAX * 255;
  auto b = (double)std::rand() / RAND_MAX * 255;

  return QColor(r, g, b);
}

void PluginWidget::changeCloseButtonState(const QString& signal_name)
{
  auto close_button = tabwidget_container_.getTabWidget(SignalPlotting::KMultiTabName)->close_button_;
  int multi_column_number = (int)SignalTreeItem::Columns::MULTI;

  // add or remove action if it checked/unchecked
  if (tree_model_->getListOfCheckedItems(multi_column_number).contains(signal_name))
  {
    // if it is the first one checked, no not create an action because it would show a little arrow
    // if it is the second one, create an action for the first one also
    if(tree_model_->getCountCheckedItems(multi_column_number) > 1)
    {
      auto action = new QAction(signal_name, this);
      action->setCheckable(false);
      close_button->addAction(action);
    }
    if (tree_model_->getCountCheckedItems(multi_column_number) == 2)
    {
      auto first_action = new QAction(tree_model_->getListOfCheckedItems(multi_column_number).first(), this);
      first_action->setCheckable(false);
      close_button->addAction(first_action);
    }
  }
  else
  {
    for (auto action : close_button->actions())
    {
      if (action->text() == signal_name)
      {
        close_button->removeAction(action);
      }
    }
  }
  if (tree_model_->getListOfCheckedItems(multi_column_number).count() > 1)
  {
    close_button->setPopupMode(QToolButton::MenuButtonPopup);
    close_button->setArrowType(Qt::ArrowType::DownArrow);
  }
  else
  {
    // if we have only one curve, remove that action too, no arrow
    if (!close_button->actions().isEmpty())
    {
      for (auto action : close_button->actions())
      {
        close_button->removeAction(action);
      }
    }
    close_button->setPopupMode(QToolButton::DelayedPopup);
    close_button->setArrowType(Qt::ArrowType::NoArrow);
  }
}

void PluginWidget::setVisibleSplitterHandle(bool state)
{
  ui_.splitter->handle(ui_.splitter->indexOf(ui_.plot_widget))->setEnabled(state);
  ui_.splitter->handle(ui_.splitter->indexOf(ui_.plot_widget))->setVisible(state);
  if (state)
  {
    ui_.splitter->setStyleSheet("");
  }
  else
  {
    ui_.splitter->setStyleSheet("QSplitter::handle { image: none; }");
  }
}


////////////////////////////////////////////////////////////////////////////////
//// Tree View Update                                                       ////
////////////////////////////////////////////////////////////////////////////////

// eCAL Callback
void PluginWidget::onProtoMessageCallback(const std::shared_ptr<google::protobuf::Message>& message, long long send_time_usecs)
{

  {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(proto_message_mutex_);

    // Create a copy of the new message as member variable. We cannot use a reference here, as this may cause a deadlock with the GUI thread
    last_proto_message_ = message;

    last_message_publish_timestamp_ = eCAL::Time::ecal_clock::time_point(std::chrono::duration_cast<eCAL::Time::ecal_clock::duration>(std::chrono::microseconds(send_time_usecs)));

    last_message_was_error_ = false;
    error_counter_ = 0;

    new_msg_available_ = true;
    received_message_counter_++;
    emit sgn_updateTree();
  }
}

// eCAL Callback
void PluginWidget::onProtoErrorCallback(const std::string& error)
{
  {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(proto_message_mutex_);

    // Copy the message into a member variable
    last_error_string_ = error.c_str();

    last_message_was_error_ = true;
    error_counter_++;

    new_msg_available_ = true;
    received_message_counter_++;
    emit sgn_errorMessageReceived();
  }
}

// Actual Tree Update
void PluginWidget::updateTree()
{
  std::lock_guard<std::mutex> lock(proto_message_mutex_);

  if (last_message_was_error_)
  {
    /* If the last message was an error, we remove everything from the
    * tree view and display it. We also log an eCAL
    * error message. */
    showErrorMessage();
  }
  else
  {
    /* If the last message was a valid message, we display that message in the
    * tree view. If currently is showing a dummy error message,
    * we have to remove it first.*/
    protobuf_decoder.ProcProtoMsg(*last_proto_message_);
    tree_view_->setVisible(true);
    logging_text_edit_->clear();
    logging_text_edit_->setVisible(false);
    new_msg_available_ = false;
  }
}

void PluginWidget::showErrorMessage()
{
    eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_error, "Error when receiving data on topic \"" + topic_name_.toStdString() + "\": " + last_error_string_.toStdString());

    tree_model_->removeAllChildren();
    tree_view_->setVisible(false);
    plotting_tabbed_widget_->setVisible(false);
    setVisibleSplitterHandle(false);
    logging_text_edit_->clear();
    logging_text_edit_->append("ERROR [" + QString::number(error_counter_) + "]: " + last_error_string_);
    logging_text_edit_->setVisible(true);
}

////////////////////////////////////////////////////////////////////////////////
//// Context Menu                                                           ////
////////////////////////////////////////////////////////////////////////////////

void PluginWidget::contextMenu(const QPoint& pos)
{
  auto selected_rows = tree_view_->selectionModel()->selectedRows((int)SignalTreeModel::Columns::FIELD_NAME);
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
  auto selected_rows = tree_view_->selectionModel()->selectedRows((int)SignalTreeModel::Columns::FIELD_NAME);;

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
  auto selected_rows = tree_view_->selectionModel()->selectedRows((int)SignalTreeModel::Columns::FIELD_NAME);

  if (selected_rows.size() > 0)
  {
    auto selected_row = selected_rows.first();
    auto tree_view_model = tree_view_->model();
    QString clipboard_string;

    for (int column = 0; column < tree_view_model->columnCount(); column++)
    {
      if (!tree_view_->isColumnHidden(column))
      {
        QModelIndex index = tree_view_model->index(selected_row.row(), column, selected_row.parent());
        QVariant variant_data = tree_view_model->data(index, Qt::ItemDataRole::DisplayRole);
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

void PluginWidget::showContextMenu(const QPoint& pos)
{
  if (pos.isNull())
    return;

  auto tabIndex = plotting_tabbed_widget_->tabBar()->tabAt(pos);
  plotting_tabbed_widget_->setCurrentIndex(tabIndex);

  QMenu menu(this);
  menu.addAction("Chart settings", this, &PluginWidget::changeChartSettings);
  menu.addAction("Close", this, &PluginWidget::closeTab);
  menu.exec(plotting_tabbed_widget_->tabBar()->mapToGlobal(pos));
}

bool PluginWidget::find_items(QAbstractTreeItem* tree_item)
{
  SignalTreeItem* item = static_cast<SignalTreeItem*>(tree_item);
  if (item)
  {
    if (tree_model_->getFullSignalName(item) == key_to_close_)
      return true;
  }
  return false;

}

void PluginWidget::closeTab()
{
  auto tab_index = plotting_tabbed_widget_->tabBar()->currentIndex();
  if (tab_index != -1)
  {
    auto tab_name = plotting_tabbed_widget_->tabText(tab_index);
    QList<QString> signals_to_delete;
    int column_number;

    if (tab_name == SignalPlotting::KMultiTabName)
    {
      column_number = (int)SignalTreeItem::Columns::MULTI;
      signals_to_delete = tree_model_->getListOfCheckedItems((int)SignalTreeItem::Columns::MULTI);
    }
    else
    {
      column_number = (int)SignalTreeItem::Columns::SINGLE;
      signals_to_delete.append(tab_name);
    }

    for (auto signal : signals_to_delete)
    {
      removeCurveFromTab(signal, column_number, tab_index);

      if (column_number == (int)SignalTreeItem::Columns::MULTI)
      {
        tabwidget_container_.hideTabWidget(tabwidget_container_.getTabWidget(SignalPlotting::KMultiTabName), 0, SignalPlotting::KMultiTabName);
      }
      else
      {
        tabwidget_container_.hideTabWidget(tabwidget_container_.getTabWidget(signal), 0, signal);
      }

      tabwidget_container_.updateAllTabWidgetsIndex(TabWidgetContainer::MathOPeration::Decrease, tab_index);
    }
  }
}

void PluginWidget::removeCurveFromTabCloseButton(QAction* action)
{
  removeCurveFromTab(action->text(), (int)SignalTreeItem::Columns::MULTI, tabwidget_container_.getTabWidgetIndex(SignalPlotting::KMultiTabName));
}

void PluginWidget::removeCurveFromTab(const QString& full_signal_name, int column_number, int tab_index)
{
  key_to_close_ = full_signal_name;

  std::function<bool(QAbstractTreeItem*)> p(std::bind(&PluginWidget::find_items, this, std::placeholders::_1));
  QList<QAbstractTreeItem*> items_found = tree_model_->findItems(p);

  if (items_found.count() == 1)
  {
    auto row_number = items_found.first()->row();

    auto model_index = tree_model_->index(row_number, column_number, tree_model_->index(items_found.first()->parentItem()));
    emit slt_on_itemClicked(model_index);
  }
  else
  {
    // if the checked signal is no longer in the tree view, just remove the tab
    plotting_tabbed_widget_->removeTab(tab_index);
    emit removeItemChecked(key_to_close_, column_number);
    changeCloseButtonState(key_to_close_);
  }
}

void PluginWidget::changeChartSettings()
{
  auto tab_index = plotting_tabbed_widget_->tabBar()->currentIndex();
  auto tab_name = plotting_tabbed_widget_->tabText(tab_index);

  // if no custom bounds were set, populate fixed minimun and maximum with current ones
  ChartWidget* current_chart_widget = tabwidget_container_.getTabWidget(tab_name);
  SignalPlotting::ChartCustomSettings& current_chart_settings = *current_chart_widget->chart_settings_;

  if (!current_chart_settings.was_changed_YAxis_Scale_Minimum_)
  {
    current_chart_settings.bounds_YAxis_to_display_.setMinValue(current_chart_widget->getCurrentPlotYAxisBounds().interval().minValue());
  }
  if (!current_chart_settings.was_changed_YAxis_Scale_Maximum_)
  {
    current_chart_settings.bounds_YAxis_to_display_.setMaxValue(current_chart_widget->getCurrentPlotYAxisBounds().interval().maxValue());
  }

  current_chart_settings.current_YAxis_bounds_ = current_chart_widget->getCurrentPlotYAxisBounds();

  ChartSettings settings_window(current_chart_settings, *current_chart_widget->getAllCurves(), this);
  connect(&settings_window, &ChartSettings::updateChartSettings, current_chart_widget, &ChartWidget::chartSettingsChanged);
  connect(&settings_window, &ChartSettings::updateChartSettings, this, &PluginWidget::updateCurveColors);
  settings_window.setWindowFlags(settings_window.windowFlags() & ~Qt::WindowContextHelpButtonHint);
  settings_window.setWindowTitle("Chart settings for " + tab_name);
  settings_window.exec();
}

void PluginWidget::updateCurveColors()
{
  for (auto tab_widget : tabwidget_container_.getAllTabWidgets())
  {
    auto curves = tab_widget->getAllCurves();
    QMapIterator<QString, SignalPlotting::Curve*> it(*curves);

    while (it.hasNext()) 
    {
      it.next();

      SignalPlotting::Curve* current_curve = it.value();

      // reset tree item color
      if (current_curve->has_color_changed_)
      {
        key_to_close_ = it.key();
        std::function<bool(QAbstractTreeItem*)> p(std::bind(&PluginWidget::find_items, this, std::placeholders::_1));
        QList<QAbstractTreeItem*> items_found = tree_model_->findItems(p);

        if (items_found.count() == 1)
        {
          QColor color = tab_widget->getColorForCurve(it.key());
          color.setAlpha(SignalPlotting::kTreeItemBackgroundAlpha);
          SignalTreeItem* tree_item = static_cast<SignalTreeItem*>(items_found.first());
          tree_item->setItemBackgroundColor(color);
        }
      }

      if (current_curve->has_color_changed_ || current_curve->has_width_changed)
      {
        current_curve->has_color_changed_ = false;
        current_curve->has_width_changed = false;
        current_curve->qwt_curve_->setPen(current_curve->curve_color_, current_curve->curve_width_);

        for (auto tab_widget_iter : tabwidget_container_.getAllTabWidgets())
        {
          QMapIterator<QString, SignalPlotting::Curve*> iterator(*tab_widget_iter->getAllCurves());

          while (iterator.hasNext())
          {
            iterator.next();

            if (iterator.key() == it.key())
            {
              // same curve in other tab
              SignalPlotting::Curve* other_curve = iterator.value();

              other_curve->curve_color_ = it.value()->curve_color_;
              other_curve->curve_width_ = it.value()->curve_width_;

              other_curve->qwt_curve_->setPen(other_curve->curve_color_, other_curve->curve_width_);
            }
          }
        }
      }
    }
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

  plugin_state_ = SignalPlotting::PLUGIN_STATE::RUNNING;
  for (auto widget : tabwidget_container_.getAllTabWidgets())
  {
    widget->addResumeLine();
    widget->setPluginState(plugin_state_);
  }

}

void PluginWidget::onPause()
{
  plugin_state_ = SignalPlotting::PLUGIN_STATE::PAUSED;
  for (auto widget : tabwidget_container_.getAllTabWidgets())
  {
    widget->setPluginState(SignalPlotting::PLUGIN_STATE::PAUSED);
    widget->addPauseLine();
  }

}

void PluginWidget::slt_on_itemClicked(const QModelIndex& index)
{
  // if the click action was made on the fused or single column
  if ((index.column() == (int)SignalTreeItem::Columns::MULTI) || (index.column() == (int)SignalTreeItem::Columns::SINGLE))
  {
    QAbstractTreeItem* item = tree_model_->item(index);

    QString full_signal_name = tree_model_->getFullSignalName(item);
    SignalTreeItem* tree_item = static_cast<SignalTreeItem*>(item);

    if (tree_item && tree_item->flags(index.column()).testFlag(Qt::ItemIsUserCheckable))
    {
      tree_model_->setCheckedState(tree_item, index.column());

      int current_working_column_index;
      int opposite_column_index;
      QString current_tab_name;
      QString opposite_tab_name;

      if (index.column() == (int)SignalTreeItem::Columns::MULTI)
      {
        current_working_column_index = (int)SignalTreeItem::Columns::MULTI;
        opposite_column_index = (int)SignalTreeItem::Columns::SINGLE;
        current_tab_name = SignalPlotting::KMultiTabName;
        opposite_tab_name = full_signal_name;
      }
      else
      {
        current_working_column_index = (int)SignalTreeItem::Columns::SINGLE;
        opposite_column_index = (int)SignalTreeItem::Columns::MULTI;
        current_tab_name = full_signal_name;
        opposite_tab_name = SignalPlotting::KMultiTabName;
      }

      if (tree_item->getItemCheckedState(current_working_column_index) == Qt::Unchecked)
      {
        emit removeItemChecked(full_signal_name, current_working_column_index);

        ChartWidget* current_chart_widget = tabwidget_container_.getTabWidget(current_tab_name);
        int current_tab_index = tabwidget_container_.getTabWidgetIndex(current_tab_name);

        if (current_working_column_index == (int)SignalTreeItem::Columns::MULTI)
        {
          current_chart_widget->detachCurve(full_signal_name);
          current_chart_widget->resetScale();
        }

        if (tree_model_->getCountCheckedItems(current_working_column_index, current_tab_name) == 0)
        {
          plotting_tabbed_widget_->removeTab(current_tab_index);
          tabwidget_container_.hideTabWidget(current_chart_widget, 0, current_tab_name);
          tabwidget_container_.updateAllTabWidgetsIndex(TabWidgetContainer::MathOPeration::Decrease, current_tab_index);
        }
        else
        {
          // if unchecked one curve from multi but there are still some curves, set current index to 0
          plotting_tabbed_widget_->setCurrentIndex(current_tab_index);
        }

        // if it is still checked on the oposite column, do not remove it
        if (!tree_model_->getListOfCheckedItems(opposite_column_index).contains(full_signal_name))
        {
          if (rounded_signals_names_.contains(full_signal_name))
            rounded_signals_names_.removeAll(full_signal_name);
        }

        // remove background color for item if it is not checked on the opposite column
        if (tabwidget_container_.hasTab(opposite_tab_name))
        {
          if (!tree_model_->getListOfCheckedItems(opposite_column_index).contains(full_signal_name))
            tree_item->setItemBackgroundColor(QColor());
        }
        else
        {
          tree_item->setItemBackgroundColor(QColor());
        }

        if (current_working_column_index == (int)SignalTreeItem::Columns::MULTI)
        {
          changeCloseButtonState(full_signal_name);
        }
      }
      else
      {
        if (!plotting_tabbed_widget_->isVisible())
        {
          plotting_tabbed_widget_->showNormal();
          setVisibleSplitterHandle(true);
        }

        if (tree_model_->getCountCheckedItems(current_working_column_index, current_tab_name) == 0)
        {
          // no tab was ever created, then create one
          if (!tabwidget_container_.hasTab(current_tab_name))
          {
            ChartWidget* new_chart_widget = tabwidget_container_.createTabWidget(plugin_state_, current_tab_name, plotting_tabbed_widget_);
            connect(new_chart_widget->close_button_, &QToolButton::clicked, this, &PluginWidget::closeTab);
            connect(new_chart_widget->close_button_, &QToolButton::triggered, this, &PluginWidget::removeCurveFromTabCloseButton);
            connect(new_chart_widget->chart_settings_button_, &QPushButton::clicked, this, &PluginWidget::changeChartSettings);
          }

          ChartWidget* current_chart_widget = tabwidget_container_.getTabWidget(current_tab_name);

          auto tab_index = current_working_column_index == (int)SignalTreeItem::Columns::MULTI ? 0 : plotting_tabbed_widget_->count();

          plotting_tabbed_widget_->insertTab(tab_index, current_chart_widget, current_tab_name);
          plotting_tabbed_widget_->widget(tab_index)->showNormal();

          if (current_working_column_index == (int)SignalTreeItem::Columns::MULTI)
          {
            tabwidget_container_.updateAllTabWidgetsIndex(TabWidgetContainer::MathOPeration::Increase, 0);
          }
          else
          {
            tabwidget_container_.updateTabWidgetIndex(current_tab_name, tab_index);
          }
        }

        auto current_chart_tab = tabwidget_container_.getTabWidget(current_tab_name);
        auto current_tab_index = tabwidget_container_.getTabWidgetIndex(current_tab_name);

        // if it was not already shown, create a new Curve
        if (!current_chart_tab->hasCurve(full_signal_name))
        {
          QColor curve_color;
          auto curve_width = SignalPlotting::kDefaultCurveWidth;
          if (tabwidget_container_.hasTab(opposite_tab_name))
          {
            if (tabwidget_container_.getTabWidget(opposite_tab_name)->hasCurve(full_signal_name))
            {
              ChartWidget* opposite_tab = tabwidget_container_.getTabWidget(opposite_tab_name);
              curve_color = opposite_tab->getColorForCurve(full_signal_name);
              curve_width = opposite_tab->getWidthForCurve(full_signal_name);
            }  
          }
          else
          {
            curve_color = generateRandomColor();
          }

          current_chart_tab->attachNewCurve(full_signal_name, curve_color, curve_width);

          curve_color.setAlpha(SignalPlotting::kTreeItemBackgroundAlpha);
          tree_item->setItemBackgroundColor(curve_color);
        }
        else
        {
          // if it was already shown, reattach it
          current_chart_tab->reattachCurve(full_signal_name);

          QColor background_color = current_chart_tab->getColorForCurve(full_signal_name);
          background_color.setAlpha(SignalPlotting::kTreeItemBackgroundAlpha);
          tree_item->setItemBackgroundColor(background_color);
        }

        if (!tree_model_->getListOfCheckedItems(current_working_column_index).contains(full_signal_name))
        {
          emit newItemChecked(full_signal_name, current_working_column_index);

          // only change close button state for multi, for single, only one curve is ever plotted
          // if signal has dissapeard and shown again, do not add another action
          if (current_working_column_index == (int)SignalTreeItem::Columns::MULTI)
          {
            changeCloseButtonState(full_signal_name);
          }
          plotting_tabbed_widget_->setCurrentIndex(current_tab_index);
        }
      }

      // hide plotting widget if there are no signals to display
      if (plotting_tabbed_widget_->isVisible() &&
        tree_model_->getListOfCheckedItems((int)SignalTreeItem::Columns::MULTI).count() == 0 &&
        tree_model_->getListOfCheckedItems((int)SignalTreeItem::Columns::SINGLE).count() == 0)
      {
        plotting_tabbed_widget_->hide();
        setVisibleSplitterHandle(false);
      }
    }
  }
}

void PluginWidget::slt_on_valueSignalChanged(QString& value, QString& full_signal_name, SignalTreeItem::Columns column_number)
{
  ChartWidget* chart_widget = column_number == SignalTreeItem::Columns::MULTI ? tabwidget_container_.getTabWidget(SignalPlotting::KMultiTabName) : tabwidget_container_.getTabWidget(full_signal_name);

  if (chart_widget != NULL)
  {
    if (value == "true" || value == "false")
    {
      chart_widget->setLastReceivedValue(full_signal_name, value == "true" ? 1.0 : 0.0);
    }
    else
    {
      if (value != "nan")
      {
        auto value_to_send = value;
        if (value_to_send.contains("e"))
        {
          auto exponent = value_to_send.mid(value_to_send.lastIndexOf("e") + 1).toInt();
          auto base = value_to_send.left(value_to_send.lastIndexOf("e"));

          bool exponent_too_big = false;
          if (exponent > 0)
          {
            if (exponent > DBL_MAX_10_EXP)
            {
              exponent = DBL_MAX_10_EXP;
              exponent_too_big = true;

              auto new_number = base + QString('e') + QString::number(exponent);

              if (!(-DBL_MAX <= new_number.toDouble() && new_number.toDouble() <= DBL_MAX))
              {
                exponent -= 1;
              }
            }
          }
          else
          {
            if (exponent < DBL_MIN_10_EXP)
            {
              exponent = DBL_MIN_10_EXP;
              exponent_too_big = true;

              auto new_number = base + QString('e') + QString::number(exponent);

              if (!(-DBL_MAX <= new_number.toDouble() && new_number.toDouble() <= DBL_MAX))
              {
                exponent -= 1;
              }
            }
          }
          if (exponent_too_big)
          {
            value_to_send = base + QString('e') + QString::number(exponent);

            // if it doesn't contain that signal already and it is plotted on multi or fused, append it
            if (!rounded_signals_names_.contains(full_signal_name))
            {
              if (tree_model_->getListOfCheckedItems((int)SignalTreeItem::Columns::MULTI).contains(full_signal_name) ||
                tree_model_->getListOfCheckedItems((int)SignalTreeItem::Columns::SINGLE).contains(full_signal_name))
              {
                rounded_signals_names_.append(full_signal_name);
              }
            }
          }
        }
        chart_widget->setLastReceivedValue(full_signal_name, value_to_send.toDouble());

        if (!rounded_signals_names_.isEmpty())
        {
          ui_.warning_label->setText(QString("The following field(s) might be rounded to fit double precision: ") + rounded_signals_names_.join(",") + "!");
        }
        else
        {
          ui_.warning_label->clear();
        }
      }
    }
  }
}

QWidget* PluginWidget::getWidget()
{
  return this;
}
