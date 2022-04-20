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

#pragma once

#include <QtCore>

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 4996) // Qt5.15 uses functions that it also deprecated itself
#endif
  #include <QtWidgets>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#include <QtGui>
#include <QObject>

#include "chart_widget.h"

class TabWidgetContainer : public QObject
{
  Q_OBJECT

public:
  TabWidgetContainer(QObject* parent = nullptr);
  ~TabWidgetContainer();

  enum class MathOPeration : int
  {
    Increase,
    Decrease
  };

  void hideTabWidget(ChartWidget* signal_tab, int index, const QString& signal_tab_name);
  ChartWidget* createTabWidget(SignalPlotting::PLUGIN_STATE plugin_state, QString& signal_tab_name, QTabWidget* parent);
  void updateTabWidgetIndex(const QString& signal_tab_name, int index);
  void updateAllTabWidgetsIndex(MathOPeration operation, int removed_tab_index);
  int getTabWidgetIndex(const QString& signal_tab_name);
  ChartWidget* getTabWidget(QString& signal_tab_name) const;
  bool hasTab(const QString& signal_tab_name);
  QList<ChartWidget*> getAllTabWidgets();

private:
  QMap <QString, QPair <int, ChartWidget*> > tab_container_;
};

