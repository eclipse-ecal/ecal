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

#include "topic_widget.h"

#include "widgets/models/item_data_roles.h"
#include "widgets/models/tree_item_type.h"

#include "widgets/visualisation_widget/visualisation_window.h"

#include "ecalmon.h"

#include <ecal/ecal_util.h>

#include <iostream>

#include <QSettings>
#include <QMenu>
#include <QApplication>
#include <QRegularExpression>

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#include <QDesktopWidget>
#endif // QT_VERSION < QT_VERSION_CHECK(5, 15, 0)

TopicWidget::TopicWidget(QWidget *parent)
  : EcalmonTreeWidget(parent)
  , parse_time_(true)
{
  // Main Model
  topic_tree_model_ = new TopicTreeModel(this);
  setModel(topic_tree_model_);

  // Show-all-Proxy-Model
  loadRegExpLists();
  topic_sort_filter_proxy_model_ = new TopicSortFilterProxyModel(this);
  topic_sort_filter_proxy_model_->setFilterRole(ItemDataRoles::FilterRole);
  topic_sort_filter_proxy_model_->setSortRole(ItemDataRoles::SortRole);
  topic_sort_filter_proxy_model_->setFilterKeyColumn((int)TopicTreeModel::Columns::TOPIC_NAME);
  topic_sort_filter_proxy_model_->setRecursiveFilteringEnabled(true);
  topic_sort_filter_proxy_model_->setRegularExpressionLists(topic_exclude_regexp_list_, topic_include_regexp_list_);
  topic_sort_filter_proxy_model_->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  setAdditionalProxyModel(topic_sort_filter_proxy_model_);

  connect(ui_.show_all_checkbox, &QCheckBox::stateChanged,
    [this](int state)
  {
    if (state == Qt::CheckState::Checked)
    {
      topic_sort_filter_proxy_model_->setRegularExpressionLists(QList<QRegularExpression>{}, QList<QRegularExpression>{});
    }
    else
    {
      topic_sort_filter_proxy_model_->setRegularExpressionLists(topic_exclude_regexp_list_, topic_include_regexp_list_);
    }
  });

  // Un-hide the show-all checkbox
  ui_.show_all_checkbox->setHidden(false);

  // Set default forced column
  setDefaultForcedColumn((int)TopicTreeModel::Columns::TOPIC_NAME);

  // Set the filter columns
  QVector<int> filter_columns
  {
    (int)TopicTreeModel::Columns::TOPIC_NAME,
    (int)TopicTreeModel::Columns::HOST_NAME,
    (int)TopicTreeModel::Columns::PROCESS_NAME,
    (int)TopicTreeModel::Columns::UNIT_NAME,
    (int)TopicTreeModel::Columns::TOPIC_ENCODING,
    (int)TopicTreeModel::Columns::TOPIC_TYPE,
    (int)TopicTreeModel::Columns::TRANSPORT_LAYER,
  };
  setFilterColumns(filter_columns);

  // Set the default group-by-settings
  QList<int> group_by_enabled_columns
  {
    (int)TopicTreeModel::Columns::TOPIC_NAME,
    (int)TopicTreeModel::Columns::DIRECTION,
    (int)TopicTreeModel::Columns::UNIT_NAME,
    (int)TopicTreeModel::Columns::HOST_NAME,
    (int)TopicTreeModel::Columns::PROCESS_NAME,
    (int)TopicTreeModel::Columns::TOPIC_ENCODING,
    (int)TopicTreeModel::Columns::TOPIC_TYPE,
    (int)TopicTreeModel::Columns::TRANSPORT_LAYER,
  };

  QList<EcalmonTreeWidget::GroupSetting> preconfigured_group_by_settings;

  EcalmonTreeWidget::GroupSetting nothing;
  nothing.group_by_columns = {};
  nothing.auto_expand = -1;
  nothing.name = "Nothing (Plain list)";
  preconfigured_group_by_settings.push_back(nothing);

  EcalmonTreeWidget::GroupSetting process;
  process.group_by_columns = { (int)TopicTreeModel::Columns::HOST_NAME, (int)TopicTreeModel::Columns::UNIT_NAME };
  process.auto_expand = 0;
  process.name = "Process";
  preconfigured_group_by_settings.push_back(process);

  EcalmonTreeWidget::GroupSetting topic;
  topic.group_by_columns = { (int)TopicTreeModel::Columns::TOPIC_NAME };
  topic.auto_expand = -1;
  topic.name = "Topic";
  preconfigured_group_by_settings.push_back(topic);

  EcalmonTreeWidget::GroupSetting encoding;
  encoding.group_by_columns = { (int)TopicTreeModel::Columns::TOPIC_ENCODING, (int)TopicTreeModel::Columns::TOPIC_NAME };
  encoding.auto_expand = -1;
  encoding.name = "Encoding";
  preconfigured_group_by_settings.push_back(encoding);

  EcalmonTreeWidget::GroupSetting type;
  type.group_by_columns = { (int)TopicTreeModel::Columns::TOPIC_TYPE, (int)TopicTreeModel::Columns::TOPIC_NAME };
  type.auto_expand = -1;
  type.name = "Type";
  preconfigured_group_by_settings.push_back(type);

  setGroupSettings(preconfigured_group_by_settings, group_by_enabled_columns);

  // Connect the double-click-reflection-window
  connect(ui_.tree_view, &QTreeView::doubleClicked, this, &TopicWidget::openReflectionWindowForSelection);
  connect(ui_.tree_view, &QAdvancedTreeView::keyEnterPressed, this, &TopicWidget::openReflectionWindowForSelection);

  // Initial layout
  autoSizeColumns();

  // Set the default visible columns
  QVector<int> default_visible_columns
  {
    (int)TopicTreeModel::Columns::TOPIC_NAME,
    (int)TopicTreeModel::Columns::DIRECTION,
    (int)TopicTreeModel::Columns::UNIT_NAME,
    (int)TopicTreeModel::Columns::HOST_NAME,
    (int)TopicTreeModel::Columns::PID,
    (int)TopicTreeModel::Columns::TOPIC_ENCODING,
    (int)TopicTreeModel::Columns::TOPIC_TYPE,
    (int)TopicTreeModel::Columns::TOPIC_SIZE,
    (int)TopicTreeModel::Columns::MESSAGE_DROPS,
    (int)TopicTreeModel::Columns::DATA_CLOCK,
    (int)TopicTreeModel::Columns::DATA_FREQUENCY,
  };
  setVisibleColumns(default_visible_columns);

  // Set the initial Tree Group
  ui_.group_by_combobox->setCurrentIndex(1);

  // Save the initial state for the resetLayout function
  saveInitialState();

  // Load the settings from the last start
  loadGuiSettings("topic_widget");
}

TopicWidget::~TopicWidget()
{
  saveGuiSettings("topic_widget");

  // Close all currently open reflection windows
  for (VisualisationWindow* window : visualisation_windows_.values())
  {
    window->close();
  }
}

void TopicWidget::loadRegExpLists()
{

  QString exclude_string;
  QString include_string;
  exclude_string = QString::fromStdString(eCAL::Config::GetMonitoringFilterExcludeList()); //ini.GetValue(MON_SECTION_S, MON_FILTER_EXCL_S);

  include_string = QString::fromStdString(eCAL::Config::GetMonitoringFilterIncludeList()); //ini.GetValue(MON_SECTION_S, MON_FILTER_INCL_S);


  // The ecal.yaml defines a very strange regex format: A filter consists of
  // several regular expressions divided by "," or ";". Thus we have to
  // split the string into 'actual' regular expressions. This will break every
  // regular expression that properly uses a ",". We cannot do anything about
  // that without changing the ecal.yaml specification.
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
  QList<QString> const exclude_string_list = exclude_string.split(QRegularExpression("[\\,,;]"), Qt::SplitBehaviorFlags::SkipEmptyParts);
  QList<QString> const include_string_list = include_string.split(QRegularExpression("[\\,,;]"), Qt::SplitBehaviorFlags::SkipEmptyParts);
#else // QT_VERSION
  QList<QString> exclude_string_list = exclude_string.split(QRegularExpression("[\\,,;]"), QString::SplitBehavior::SkipEmptyParts);
  QList<QString> include_string_list = include_string.split(QRegularExpression("[\\,,;]"), QString::SplitBehavior::SkipEmptyParts);
#endif // QT_VERSION

  for (auto& s : exclude_string_list)
  {
    topic_exclude_regexp_list_.push_back(QRegularExpression(s));
  }

  for (auto& s : include_string_list)
  {
    topic_include_regexp_list_.push_back(QRegularExpression(s));
  }
}

void TopicWidget::autoSizeColumns()
{
  eCAL::pb::Topic example_topic_pb;

  example_topic_pb.set_rclock(999999);
  example_topic_pb.set_hname("HOSTNAME____");
  example_topic_pb.set_pid(999999);
  example_topic_pb.set_pname("");
  example_topic_pb.set_uname("ATypicalLongUnitName___");
  example_topic_pb.set_tid("");
  example_topic_pb.set_tname("ATypicalLongTopicName___");
  example_topic_pb.set_direction("subscriber__");
  example_topic_pb.mutable_tdatatype()->set_name("Enc.TypeName.Name____");
  example_topic_pb.mutable_tdatatype()->set_encoding("encdg:");
  //example_topic_pb.mutable_tdatatype()->set_desc("");

  example_topic_pb.mutable_tlayer()->Add()->set_type(eCAL::pb::eTLayerType::tl_ecal_shm);
  example_topic_pb.mutable_tlayer()->Add()->set_type(eCAL::pb::eTLayerType::tl_ecal_udp_mc);
  example_topic_pb.set_tsize(999999);
  example_topic_pb.set_connections_loc(999999);
  example_topic_pb.set_connections_ext(999999);
  example_topic_pb.set_message_drops(999999);
  example_topic_pb.set_dclock(99999999999);
  example_topic_pb.set_dfreq(999999);

  TopicTreeItem* example_topic_item = new TopicTreeItem(example_topic_pb);
  GroupTreeItem* example_group_item = new GroupTreeItem("ATypicalLongGroupName___", "", "", QVariant(), "");

  topic_tree_model_->insertItem(example_group_item);
  auto group_index = topic_tree_model_->index(example_group_item);
  topic_tree_model_->insertItem(example_topic_item, group_index);
  ui_.tree_view->expandAll();

  bool show_all = ui_.show_all_checkbox->isChecked();
  ui_.show_all_checkbox->setChecked(true);

  QList<int> columns_to_resize
  {
    (int)TopicTreeModel::Columns::GROUP,
    (int)TopicTreeModel::Columns::TOPIC_NAME,
    (int)TopicTreeModel::Columns::DIRECTION,
    (int)TopicTreeModel::Columns::UNIT_NAME,
    (int)TopicTreeModel::Columns::HOST_NAME,
    (int)TopicTreeModel::Columns::PID,
    (int)TopicTreeModel::Columns::TOPIC_ENCODING,
    (int)TopicTreeModel::Columns::TOPIC_TYPE,
    (int)TopicTreeModel::Columns::HEARTBEAT,
    (int)TopicTreeModel::Columns::TRANSPORT_LAYER,
    (int)TopicTreeModel::Columns::TOPIC_SIZE,
    (int)TopicTreeModel::Columns::CONNECTIONS_LOCAL,
    (int)TopicTreeModel::Columns::CONNECTIONS_EXTERNAL,
    (int)TopicTreeModel::Columns::MESSAGE_DROPS,
    (int)TopicTreeModel::Columns::DATA_CLOCK,
    (int)TopicTreeModel::Columns::DATA_FREQUENCY,
  };

  for (int column : columns_to_resize)
  {
    ui_.tree_view->resizeColumnToContents(column);
  }


  topic_tree_model_->removeItem(example_topic_item);
  topic_tree_model_->removeItem(example_group_item);

  ui_.show_all_checkbox->setChecked(show_all);
}

void TopicWidget::openReflectionWindowForSelection()
{
  auto selected_proxy_indices = ui_.tree_view->selectionModel()->selectedRows();

  if (selected_proxy_indices.size() > 0)
  {
    QModelIndex index = mapToSource(selected_proxy_indices.at(0));

    QAbstractTreeItem* item = topic_tree_model_->item(index);
    if (item && (item->type() == (int)TreeItemType::Topic))
    {
      TopicTreeItem* topic_item = (TopicTreeItem*)item;

      QString topic_name = topic_item->data(TopicTreeItem::Columns::TNAME, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016

      if (visualisation_windows_.contains(topic_name))
      {
        // Bring an existing Reflection Window to the front
        VisualisationWindow* visualisation_window = visualisation_windows_[topic_name];
        visualisation_window->setWindowState((visualisation_window->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        visualisation_window->raise();  // for MacOS
        visualisation_window->activateWindow(); // for Windows
      }
      else
      {
        QString topic_encoding = topic_item->data(TopicTreeItem::Columns::TENCODING, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016
        QString topic_type = topic_item->data(TopicTreeItem::Columns::TTYPE, (Qt::ItemDataRole)ItemDataRoles::RawDataRole).toString(); //-V1016

        QString combined_topic_encoding_type{ QString::fromStdString(eCAL::Util::CombinedTopicEncodingAndType(topic_encoding.toStdString(), topic_type.toStdString())) };

        // Create a new Reflection Window
        VisualisationWindow* visualisation_window = new VisualisationWindow(topic_name, combined_topic_encoding_type);
        visualisation_window->setAttribute(Qt::WA_DeleteOnClose, true);
        visualisation_window->setParseTimeEnabled(parse_time_);
        visualisation_windows_[topic_name] = visualisation_window;
        connect(this, &EcalmonTreeWidget::topicsUpdated, visualisation_window, &VisualisationWindow::updatePublishers);
        connect(visualisation_window, &VisualisationWindow::destroyed, this, [this, topic_name]() {visualisation_windows_.remove(topic_name); });

        visualisation_window->show();
      }
    }
  }
}

void TopicWidget::fillContextMenu(QMenu& menu, const QList<QAbstractTreeItem*>& selected_items)
{
  EcalmonTreeWidget::fillContextMenu(menu, selected_items);

  QAbstractTreeItem* item = nullptr;
  for (QAbstractTreeItem* selected_item : selected_items)
  {
    // get the first non-group item
    if (selected_item->type() == (int)TreeItemType::Topic)
    {
      item = selected_item;
      break;
    }
  }

  if (item)
  {
    QString topic_name     = item->data((int)TopicTreeItem::Columns::TNAME).toString();
    QString topic_encoding = item->data((int)TopicTreeItem::Columns::TENCODING).toString();
    QString topic_type     = item->data((int)TopicTreeItem::Columns::TTYPE).toString();

    QString combined_topic_encoding_type{ QString::fromStdString(eCAL::Util::CombinedTopicEncodingAndType(topic_encoding.toStdString(), topic_type.toStdString()))};

    QAction* reflection_action = new QAction(tr("Inspect topic \"") + topic_name + "\"", &menu);
    connect(reflection_action, &QAction::triggered, this, &TopicWidget::openReflectionWindowForSelection);
    menu.addSeparator();
    menu.addAction(reflection_action);

    auto reflection_with_menu = menu.addMenu(tr("Inspect topic \"") + topic_name + "\" with");
    for (const auto& matching_plugin_data : PluginManager::getInstance()->getMatchingPluginData(topic_name, combined_topic_encoding_type))
    {
      auto reflection_with_action = reflection_with_menu->addAction(matching_plugin_data.meta_data.name);
      const auto iid = matching_plugin_data.iid;
      connect(reflection_with_action, &QAction::triggered, this, [this, iid, topic_name, combined_topic_encoding_type]() {
        emit requestVisualisationDockWidget(topic_name, combined_topic_encoding_type, iid);
        });
    }  
  }
}

void TopicWidget::resetLayout()
{
  EcalmonTreeWidget::resetLayout();

  // Reset layout of all reflection windows

  QSettings settings;
  settings.beginGroup("reflection_widget");
  settings.setValue("tree_state", QByteArray());  // Reset the settings, so new windows will open resetted
  settings.endGroup();

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  int screen_number = QApplication::desktop()->screenNumber(this);
#else
  int screen_number = 0;
  QScreen* current_screen = this->screen();
  if (current_screen != nullptr)
  {
    screen_number = QApplication::screens().indexOf(current_screen);
    if (screen_number < 0)
      screen_number = 0;
  }
#endif // QT_VERSION < QT_VERSION_CHECK(5, 15, 0)

  for (auto reflection_window : visualisation_windows_.values())
  {
    reflection_window->resetLayout(screen_number);
  }
}
void TopicWidget::setParseTimeEnabled(bool enabled)
{
  parse_time_ = enabled;
  for (VisualisationWindow* visualisation_window : visualisation_windows_.values())
  {
    visualisation_window->setParseTimeEnabled(parse_time_);
  }
}