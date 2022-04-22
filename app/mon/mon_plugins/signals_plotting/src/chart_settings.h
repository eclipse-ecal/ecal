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

#ifndef CHARTSETTINGS_H
#define CHARTSETTINGS_H

#include "chart_widget.h"

#include <QDialog>
#include <QPushButton>
#include <QColorDialog>

namespace Ui {
  class ChartSettings;
}

class ChartSettings : public QDialog
{
  Q_OBJECT
public:
  explicit ChartSettings(SignalPlotting::ChartCustomSettings& chart_settings, QMap<QString, SignalPlotting::Curve*>& curves, QWidget* parent = 0);
  ~ChartSettings();

signals:
  void updateChartSettings();

private slots:
  void scaleXAxisChanged(int state);
  void scaleYAxisMinimumChanged(int state);
  void scaleYAxisMaximumChanged(int state);
  void cursorChanged(int state);
  void linesChanged(int state);

  void enableXAxisMajorLines(int state);
  void enableYAxisMajorLines(int state);

  void currentCurveChanged(int index);
  void openColorDialog();
  void curveColorChanged(const QColor& color);
  void curveWidthChanged(double width);

  void cancelDialog();
  void applyChanges();

private:
  Ui::ChartSettings* ui_;
  QColorDialog* color_dialog_;
  SignalPlotting::ChartCustomSettings* chart_settings_;
  QMap<QString, SignalPlotting::Curve*>* curves_;
};

#endif // CHARTSETTINGS_H
