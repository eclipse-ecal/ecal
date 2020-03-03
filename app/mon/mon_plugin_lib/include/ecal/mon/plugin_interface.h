#pragma once 

#include <ecal/mon/plugin_widget_interface.h>

#include <QString>
#include <QWidget>

namespace eCAL
{
  namespace mon
  {
    class PluginInterface
    {
    public:
      virtual PluginWidgetInterface* create(const QString& topic_name, const QString& topic_type, QWidget* parent = Q_NULLPTR) = 0;
    };
  }
}

#define PluginInterface_iid "de.conti.ecal.mon.plugin.interface"

Q_DECLARE_INTERFACE(eCAL::mon::PluginInterface, PluginInterface_iid)