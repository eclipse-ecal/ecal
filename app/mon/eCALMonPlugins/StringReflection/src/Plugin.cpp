#include "PluginWidget.h"
#include "Plugin.h"

using namespace eCAL::mon;

PluginWidgetInterface* Plugin::create(const QString& topic_name, const QString& topic_type, QWidget* parent)
{
  return new PluginWidget(topic_name, topic_type, parent);
}