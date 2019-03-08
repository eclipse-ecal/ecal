#pragma once

#include <ecal/mon/plugin.h> 

class Plugin : public QObject, public eCAL::mon::PluginInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "de.conti.ecal.monitor.plugin.dummy" FILE "metadata.json")
  Q_INTERFACES(eCAL::mon::PluginInterface)
public:
  virtual eCAL::mon::PluginWidgetInterface* create(const QString& topic_name, const QString& topic_type, QWidget* parent = Q_NULLPTR);
};