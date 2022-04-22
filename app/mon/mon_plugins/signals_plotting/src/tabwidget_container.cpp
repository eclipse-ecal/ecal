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

#include "tabwidget_container.h"

TabWidgetContainer::TabWidgetContainer(QObject* parent)
  : QObject(parent)
{
}

TabWidgetContainer::~TabWidgetContainer()
{
  tab_container_.clear();
}

void TabWidgetContainer::hideTabWidget(ChartWidget* signal_tab, int index, const QString& signal_tab_name)
{
  if (tab_container_.contains(signal_tab_name))
  {
    updateTabWidgetIndex(signal_tab_name, index);
    return;
  }
  tab_container_.insert(signal_tab_name, QPair <int, ChartWidget*>(index, signal_tab));
}

ChartWidget* TabWidgetContainer::createTabWidget(SignalPlotting::PLUGIN_STATE plugin_state, QString& signal_tab_name, QTabWidget* parent)
{
  ChartWidget* chart_widget = new ChartWidget(plugin_state, parent, signal_tab_name);
  tab_container_.insert(signal_tab_name, QPair <int, ChartWidget*>(0, chart_widget));
  return chart_widget;
}

void TabWidgetContainer::updateTabWidgetIndex(const QString& signal_tab_name, int index)
{
  tab_container_[signal_tab_name].first = index;
}

void TabWidgetContainer::updateAllTabWidgetsIndex(MathOPeration operation, int removed_tab_index)
{
  auto key_values = tab_container_.keys();
  switch (operation)
  {
  case TabWidgetContainer::MathOPeration::Increase:
  {
    for (auto key : key_values)
    {
      if (key == SignalPlotting::KMultiTabName)
        continue;
      tab_container_[key].first++;
    }
    break;
  }
  case TabWidgetContainer::MathOPeration::Decrease:
  {
    for (auto key : key_values)
    {
      if (key == SignalPlotting::KMultiTabName)
        continue;
      if (tab_container_[key].first > removed_tab_index)
      {
        tab_container_[key].first -= 1;
      }
    }
    break;
  }
  default:
    break;
  }

}

int TabWidgetContainer::getTabWidgetIndex(const QString& signal_tab_name)
{
  return tab_container_[signal_tab_name].first;
}

ChartWidget* TabWidgetContainer::getTabWidget(QString& signal_tab_name) const
{
  return tab_container_[signal_tab_name].second;
}

bool TabWidgetContainer::hasTab(const QString& signal_tab_name)
{
  return tab_container_.contains(signal_tab_name);
}

QList<ChartWidget*> TabWidgetContainer::getAllTabWidgets()
{
  QList<ChartWidget*> tab_widgets;

  for (auto tab : tab_container_)
  {
    tab_widgets.append(tab.second);
  }
  return tab_widgets;
}
