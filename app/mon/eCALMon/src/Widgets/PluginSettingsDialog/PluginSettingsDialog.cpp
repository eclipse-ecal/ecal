#include "PluginSettingsDialog.h"
#include "EcalmonGlobals.h"

#include "Widgets/Models/PluginTableModel.h"

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
