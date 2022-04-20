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

#include "signal_tree_item.h"

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QToolButton>
#include <QQueue>
#include <QLayout>
#include <QElapsedTimer>
#include <QDoubleValidator>
#include <QPen>

#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_zoomer.h"
#include "qwt_picker_machine.h"
#include "qwt_plot_item.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_textlabel.h"
#include "qwt_text.h"

#include <cfloat>
#include <cmath>
#include <algorithm>


class ChartWidget : public QWidget
{
  Q_OBJECT
public:
  ChartWidget(SignalPlotting::PLUGIN_STATE plugin_state = SignalPlotting::PLUGIN_STATE::UNKNOWN, QWidget* parent = 0, const QString& tab_name = 0);
  ~ChartWidget();

  // methods related to curve manipulation
  void        attachNewCurve(const QString& curve_name = 0, const QColor& curve_color = QColor(), double width = 3);
  void        detachCurve(const QString& curve_name = 0);
  void        reattachCurve(const QString& curve_name = 0);

  bool        hasCurve(const QString& curve_name = 0);
  QColor      getColorForCurve(const QString& curve_name = 0);
  double      getWidthForCurve(const QString& curve_name = 0);
  void        setLastReceivedValue(const QString& curve_name = 0, double last_received_value = 0);

  // methods related to the plot
  void        resetScale();
  QwtScaleDiv getCurrentPlotYAxisBounds();
  void        addPauseLine();
  void        addResumeLine();

  void        setPluginState(SignalPlotting::PLUGIN_STATE plugin_state);

  QMap<QString, SignalPlotting::Curve*>* getAllCurves();
 
  SignalPlotting::PLUGIN_STATE                       plugin_state_;
  SignalPlotting::ChartCustomSettings*               chart_settings_;

  QToolButton*                                       close_button_;
  QPushButton*                                       chart_settings_button_;

public slots:
  void onPlotTimeOut();
  void onRecalculateScaleTimeOut();
  void chartSettingsChanged();
  void stopTimer();
  void startTimer();
  void slt_picker(const QRectF& rect);
  void slt_zoomed(const QRectF& rect);

private:
  // timer used to calculate the actual time passed between redrawing the plot
  QElapsedTimer elapsed_timer_;
  QTimer plot_timer_;
  QwtInterval time_interval_;
  double current_time_point_;
  // variable used to check if it is the first time ploting, do not calculate elapsed time
  bool timer_first_time_;

  int queue_size_max_;

  QMap<QString, SignalPlotting::Curve*> curves_;
  double maximum_received_point_;
  double minimum_received_point_;

  QwtPlot* qwt_plot_;
  // timer used to check every 3 seconds if minimum and maximum are still in the queue
  QTimer recalculate_scale_timer_;  
  QwtPlotZoomer* zoomer_;
  bool zoomer_state_;
  QwtPlotPicker* picker_;
  QwtPlotTextLabel* zoomed_label_;
  QwtPlotTextLabel* normal_view_label_;
  
  bool isPointTriggeringRescale(double value);
  void forceAllCurvesToRecalculateMinimum();
  void forceAllCurvesToRecalculateMaximum();
};
