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

#include "plugin_settings_dialog.h"
#include "ecalmon_globals.h"

#include "widgets/models/plugin_table_model.h"

#include <QSortFilterProxyModel>
#include <QSettings>

PluginSettingsDialog::PluginSettingsDialog(QWidget *parent) :
    QDialog(parent)
{
    ui_.setupUi(this);

    auto source_model = new PluginTableModel(this);
    auto proxy_model = new QSortFilterProxyModel(this);

    proxy_model->setSourceModel(source_model);

    ui_.treeView->setModel(proxy_model);
    ui_.treeView->setSortingEnabled(true);
    
    connect(ui_.closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui_.reloadButton, &QPushButton::clicked, [source_model]()
    {
      source_model->rediscover();
    });

    QSettings settings;
    settings.beginGroup("plugin_settings_dialog");
    ui_.treeView->restoreState(settings.value("tree_state", ui_.treeView->saveState(EcalmonGlobals::Version())).toByteArray(), EcalmonGlobals::Version());
    this->restoreGeometry(settings.value("geometry", this->saveGeometry()).toByteArray());
    settings.endGroup();
}

PluginSettingsDialog::~PluginSettingsDialog()
{
  QSettings settings;
  settings.beginGroup("plugin_settings_dialog");
  settings.setValue("tree_state", ui_.treeView->saveState(EcalmonGlobals::Version()));
  settings.setValue("geometry", this->saveGeometry());
  settings.endGroup();
}
