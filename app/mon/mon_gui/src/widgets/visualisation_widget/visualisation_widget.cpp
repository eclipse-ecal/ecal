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

#include "visualisation_widget.h"

#include <CustomQt/QStandardTreeItem.h>

#include <QTabWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QTimer>

#ifndef NDEBUG
  #ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4251 4800) // disable QDebug Warnings
  #endif // _MSC_VER

  #include <QDebug>

  #ifdef _MSC_VER
    #pragma warning(pop)
  #endif // _MSC_VER
#endif // NDEBUG

#include <QLibrary>
#include <QPluginLoader>
#include <QJsonArray>

#include <chrono>

#include "ecalmon_globals.h"

#include "ecal/ecal.h"

using namespace eCAL::mon;

VisualisationWidget::VisualisationWidget(const QString& topic_name, const QString& topic_type, QWidget *parent)
  : QWidget(parent)
  , topic_name_(topic_name)
  , topic_type_(topic_type)
{
  ui_.setupUi(this);
  int label_height = ui_.label_publishers->sizeHint().height();
  QPixmap warning_icon = QPixmap(":/ecalicons/WARNING").scaled(label_height, label_height, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
  ui_.warning_label->setPixmap(warning_icon);
  ui_.warning_label->hide();
  ui_.label_publishers->setText("");

  // Set the headline
  QString header = topic_name;
  if (!topic_type.isEmpty())
  {
    header += (" (" + topic_type + ")");
  }
  ui_.topic_name_label->setText(header);

  auto supported_plugins_ = PluginManager::getInstance()->CreatePlugins(topic_name, topic_type, this);

  // add suitable plugin widgets to the tab view
  for (const auto& plugin : supported_plugins_)
  {
    auto plugin_info = plugin.first;
    auto plugin_widget = plugin.second;
    plugin_widgets_.push_back(plugin_widget);
    ui_.tab_widget->addTab(plugin_widget->getWidget(), plugin_info.meta_data.name);

    connect(ui_.pause_button, &QPushButton::toggled, [plugin_widget](bool checked) { if (checked) plugin_widget->onPause(); else plugin_widget->onResume(); });

    /*
      * Workaround for a bug in the Qt Fusion theme that is used in Linux and
      * Windows Dark mode by default. The Fusion theme will (at least in Qt 5.15)
      * ignore the ON Icon. It is fixed in Qt 6.10 and up.
      * 
      * https://forum.qt.io/topic/129728/different-behaviour-of-icons-in-vista-style-vs-fusion-style
      * https://codereview.qt-project.org/c/qt/qtbase/+/327734
    */
    connect(ui_.pause_button, &QPushButton::toggled, this,
                        [this]()
                        {
                          if(ui_.pause_button->isChecked())
                            ui_.pause_button->setIcon(QIcon(":ecalicons/START"));
                          else
                            ui_.pause_button->setIcon(QIcon(":ecalicons/PAUSE"));
                        });

    connect(&update_timer_, &QTimer::timeout, [this]() {if (!isPaused()) plugin_widgets_.at(ui_.tab_widget->currentIndex())->onUpdate(); });
  }

  // force update if tab view has changed
  connect(ui_.tab_widget, &QTabWidget::currentChanged, [this](auto index) {plugin_widgets_.at(index)->onUpdate(); });

  // should never happens because default plugins are always available
  if (plugin_widgets_.empty())
    return;

  // initialize the update timer and connect the update signal
  update_timer_.setSingleShot(false);
  update_timer_.setInterval(UPDATE_INTERVAL_MS);

  // Button connections
  connect(ui_.pause_button, &QPushButton::toggled,
    [this](bool checked)
  {
    if (checked)
    {
      ui_.pause_button->setText(tr("Resume"));
      update_timer_.stop();
    }
    else
    {
      ui_.pause_button->setText(tr("Pause"));
      update_timer_.start();
    }
  });

  //hide publishers frame
  ui_.textEdit->hide();
  //ui_.frame->hide();

  //connect show/hide publishers button
  connect(ui_.show_hide_textEditpushButton, &QPushButton::toggled,
    [this](bool checked)
  {
    if (!checked)
    {
      ui_.show_hide_textEditpushButton->setIcon(QIcon(":/ecalicons/HIDE"));
      ui_.textEdit->hide();
    }
    else
    {
      ui_.show_hide_textEditpushButton->setIcon(QIcon(":/ecalicons/SHOW"));
      ui_.textEdit->show();
    }
  });
  update_timer_.start();
}

VisualisationWidget::~VisualisationWidget()
{
#ifndef NDEBUG
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  qDebug().nospace() << "[" << VisualisationWidget::metaObject()->className() << "]: Deleting Widget for topic " << topic_name_;
#endif // NDEBUG
}


void VisualisationWidget::setParseTimeEnabled(bool)
{
}


////////////////////////////////////////////////////////////////////////////////
//// Getter                                                                 ////
////////////////////////////////////////////////////////////////////////////////

bool VisualisationWidget::isPaused() const
{
  return ui_.pause_button->isChecked();
}

void VisualisationWidget::checkForMorePublishersWithSameTopic(const eCAL::pb::Monitoring & monitoring_pb)
{
  QStringList publishers = {};
  for (const auto& topic : monitoring_pb.topics())
  {
    if (!topic_name_.isNull())
    {
      if ((topic.topic_name() == topic_name_.toStdString()) && (topic.direction() == std::string("publisher")))
      {
        publishers.append(QString(QString(topic.host_name().c_str()) + QString(": ") + QString(topic.process_name().c_str())));
      }
    }
  }
  switch (publishers.size())
  {
  case 0:
  {
    ui_.nr_of_publisers_label->setText("0");
    ui_.nr_of_publisers_label->setStyleSheet("color: red;");
    ui_.label_publishers->setStyleSheet("color: red;");
    ui_.label_publishers->setText("Publishers");
    ui_.warning_label->setVisible(true);
    break;
  }
  case 1:
  {
    ui_.nr_of_publisers_label->setText("1");
    ui_.nr_of_publisers_label->setStyleSheet("");
    ui_.label_publishers->setText("Publisher");
    ui_.label_publishers->setStyleSheet("");
    ui_.warning_label->setVisible(false);
    break;
  }
  default:
  {
    ui_.nr_of_publisers_label->setText(QString::number(publishers.size()));
    ui_.nr_of_publisers_label->setStyleSheet("");
    ui_.label_publishers->setStyleSheet("");
    ui_.label_publishers->setText("Publishers");
    ui_.warning_label->setVisible(false);
    break;
  }
  }
  ui_.textEdit->clear();
  for (const auto& publisher : publishers)
    ui_.textEdit->append(publisher);
}
