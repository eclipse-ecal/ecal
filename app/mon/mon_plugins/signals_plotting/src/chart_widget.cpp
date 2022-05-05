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

#include "chart_widget.h"


ChartWidget::ChartWidget(SignalPlotting::PLUGIN_STATE plugin_state, QWidget* parent, const QString& tab_name)
  : QWidget(parent),
  plugin_state_(plugin_state),
  chart_settings_(new SignalPlotting::ChartCustomSettings()),
  time_interval_(0.0, SignalPlotting::kDefaultTimeMax),
  current_time_point_(0),
  timer_first_time_(true),
  queue_size_max_(static_cast<int>(SignalPlotting::kDefaultTimeMax * 1000.0 / SignalPlotting::kPlotWindow)),
  zoomed_label_(nullptr),
  normal_view_label_(nullptr)
{
  resetScale();

  // Plot related logic
  qwt_plot_ = new QwtPlot(this);
  qwt_plot_->setMinimumSize(QSize(10,10));
  qwt_plot_->setAxisTitle(QwtPlot::xBottom, QString::fromUtf8("Time / seconds"));
  qwt_plot_->setAxisScale(QwtPlot::xBottom, time_interval_.minValue(), time_interval_.maxValue());
  qwt_plot_->setAxisTitle(QwtPlot::yLeft, tab_name);
  qwt_plot_->setAxisScale(QwtPlot::yLeft, 0.0, 100.0);

  //zoom related code : left mouse click to select a rectangle for zoom in, rigth mouse click to zoom out
  zoomer_ = new QwtPlotZoomer(qwt_plot_->canvas());
  zoomer_state_ = false;
  zoomer_->setTrackerMode(QwtPlotPicker::AlwaysOff);
  picker_ = new QwtPlotPicker(qwt_plot_->canvas());
  picker_->setStateMachine(new QwtPickerDragRectMachine());
  picker_->setRubberBand(QwtPlotPicker::RectRubberBand);
  picker_->setRubberBandPen(QPen(Qt::red, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  picker_->setTrackerPen(QPen(Qt::green, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

  //set view labels
  zoomed_label_ = new QwtPlotTextLabel();
  QwtText zoomed("Zoomed view (right click to zoom out)");
  zoomed.setRenderFlags(Qt::AlignHCenter | Qt::AlignTop);
  QFont font;
  font.setBold(true);
  zoomed.setFont(font);
  zoomed_label_->setText(zoomed);

  normal_view_label_ = new QwtPlotTextLabel();
  QwtText normal_view("Normal view (select a rectangle to zoom in)");
  normal_view.setRenderFlags(Qt::AlignHCenter | Qt::AlignTop);
  normal_view.setFont(font);
  normal_view_label_->setText(normal_view);
  normal_view_label_->attach(qwt_plot_);

  connect(picker_, QOverload<const QRectF&>::of(&QwtPlotPicker::selected), this, &ChartWidget::slt_picker);
  connect(zoomer_, &QwtPlotZoomer::zoomed, this, &ChartWidget::slt_zoomed);

  // Plot timer
  plot_timer_.setInterval(SignalPlotting::kPlotWindow);
  plot_timer_.setSingleShot(false);
  connect(&plot_timer_, &QTimer::timeout, this, &ChartWidget::onPlotTimeOut);
  plot_timer_.start();

  recalculate_scale_timer_.setInterval(time_interval_.width() / 10);
  recalculate_scale_timer_.setSingleShot(false);
  connect(&recalculate_scale_timer_, &QTimer::timeout, this, &ChartWidget::onRecalculateScaleTimeOut);
  recalculate_scale_timer_.start();

  QVBoxLayout* tab_layout = new QVBoxLayout(this);
  QHBoxLayout* button_layout = new QHBoxLayout();
  this->setLayout(tab_layout);

  chart_settings_button_ = new QPushButton(this);
  chart_settings_button_->setText("Chart settings");
  chart_settings_button_->setMinimumWidth(10);

  close_button_ = new QToolButton(this);
  close_button_->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextOnly);
  close_button_->setText("Close");
  close_button_->setMinimumWidth(10);
  close_button_->setFixedHeight(chart_settings_button_->sizeHint().height());

  button_layout->addWidget(chart_settings_button_);
  button_layout->addWidget(close_button_);
  button_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

  tab_layout->addLayout(button_layout);
  tab_layout->addWidget(qwt_plot_);
}

void ChartWidget::onPlotTimeOut()
{
  // only plot/replot if we have curves
  if (curves_.count() > 0)
  {
    auto time_elapsed_in_miliseconds = 0.0;
    // do not calculate elapsed time if it's the first time entering this function
    if (!timer_first_time_)
    {
      time_elapsed_in_miliseconds = elapsed_timer_.elapsed();
      current_time_point_ += time_elapsed_in_miliseconds / 1000.0;
    }
    timer_first_time_ = false;
    elapsed_timer_.restart();

    for (auto curve : curves_.values())
    {
      // only do computations if data is received from eCAL
      if (curve->has_ecal_received_value_)
      {
        // only take in consideration minimum and maximum if curve is attached and no other setting was made
        if (curve->is_attached_ && (chart_settings_->compute_scale_YAxis_Maximum_ || chart_settings_->compute_scale_YAxis_Minimum_))
        {
          double minimum_value_to_set = qwt_plot_->axisScaleDiv(QwtPlot::yLeft).interval().minValue();
          double maximum_value_to_set = qwt_plot_->axisScaleDiv(QwtPlot::yLeft).interval().maxValue();

          if (chart_settings_->compute_scale_YAxis_Maximum_)
          {
            if (maximum_received_point_ < curve->last_received_value_)
            {
              maximum_received_point_ = curve->last_received_value_;
            }

            if (std::fabs(minimum_received_point_ - DBL_MAX) > DBL_EPSILON)
            {
              auto current_delta = std::abs(maximum_received_point_ - minimum_received_point_);
              if (FP_ZERO == std::fpclassify(current_delta))
              {
                curve->forced_maximum_scale_ = true;
                maximum_value_to_set = maximum_received_point_ < 0 ? maximum_received_point_ * SignalPlotting::kDefaultMinimumFactor : maximum_received_point_ * SignalPlotting::kDefaultMaximumFactor;
              }
              else
              {
                if (isPointTriggeringRescale(curve->last_received_value_) || curve->forced_maximum_scale_)
                {
                  curve->forced_maximum_scale_ = false;
                  maximum_value_to_set =  maximum_received_point_ + (current_delta * SignalPlotting::kDeltaFactor);
                }
              }
            }
          }

          if (chart_settings_->compute_scale_YAxis_Minimum_)
          {
            if (minimum_received_point_ > curve->last_received_value_)
            {
              minimum_received_point_ = curve->last_received_value_;
            }

            auto current_delta = std::abs(maximum_received_point_ - minimum_received_point_);
            if (FP_ZERO == std::fpclassify(current_delta))
            {
              curve->forced_minimum_scale_ = true;
              minimum_value_to_set = minimum_received_point_ > 0 ? minimum_received_point_ * SignalPlotting::kDefaultMinimumFactor : minimum_received_point_ * SignalPlotting::kDefaultMaximumFactor;
            }
            else
            {
              if (isPointTriggeringRescale(curve->last_received_value_) || curve->forced_minimum_scale_)
              {
                curve->forced_minimum_scale_ = false;
                minimum_value_to_set = minimum_received_point_ - (current_delta * SignalPlotting::kDeltaFactor);
              }
               
            }
          }
          auto original_minimum = minimum_value_to_set;
          auto original_maximum = maximum_value_to_set;

          // if the difference between minimum and maximum is lower than the allowed value, modify it
          if (std::abs(original_maximum - original_minimum) < SignalPlotting::kMinimumAllowedIntervalDifference)
          {
            minimum_value_to_set = (original_maximum < 0) || (original_minimum < 0) ? minimum_value_to_set - SignalPlotting::kFactorToIncreaseIntervalDifference : 0;
            maximum_value_to_set = (original_maximum > 0) ? maximum_value_to_set + SignalPlotting::kFactorToIncreaseIntervalDifference : 0;

          }
          if (!zoomer_state_)
          {
            qwt_plot_->setAxisScale(QwtPlot::yLeft, minimum_value_to_set, maximum_value_to_set);
            if (plugin_state_ != SignalPlotting::PLUGIN_STATE::PAUSED)
            {
              qwt_plot_->replot();
            }
          }
        }
        // enqueue time point
        curve->queue_time_values_.enqueue(current_time_point_);

        // remove unnecessary time points
        while (curve->queue_time_values_.size() > queue_size_max_)
        {
          curve->queue_time_values_.dequeue();
        }

        // enqueue signal value
        curve->queue_received_values_.enqueue(curve->last_received_value_);

        // remove unnecessary signal values
        while (curve->queue_received_values_.size() > queue_size_max_)
        {
          curve->queue_received_values_.dequeue();
        }

        if (curve->is_attached_ && plugin_state_ != SignalPlotting::PLUGIN_STATE::PAUSED)
        {
          curve->qwt_curve_->setSamples(curve->queue_time_values_.toVector(), curve->queue_received_values_.toVector());
        }
      }
    }

    if (current_time_point_ > (time_interval_.minValue() + (time_interval_.width() * SignalPlotting::kDefaultMinimumFactor)))
    {
      time_interval_.setInterval(time_interval_.minValue() + (time_elapsed_in_miliseconds / 1000.0), time_interval_.maxValue() + (time_elapsed_in_miliseconds / 1000.0));
      if (!zoomer_state_)
      {
        qwt_plot_->setAxisScale(QwtPlot::xBottom, time_interval_.minValue(), time_interval_.maxValue());
      }
    }
    if (plugin_state_ != SignalPlotting::PLUGIN_STATE::PAUSED)
    {
      qwt_plot_->replot();
    }
  }
}

void ChartWidget::onRecalculateScaleTimeOut()
{
  double current_minimum = DBL_MAX;
  double current_maximum = -DBL_MAX;

  if (!curves_.isEmpty())
  {
    for (auto curve : curves_.values())
    {
      if (!curve->queue_received_values_.isEmpty() && curve->is_attached_)
      {
        auto queue_to_vector = curve->queue_received_values_.toVector();

        double min_value = *std::min_element(queue_to_vector.constBegin(), queue_to_vector.constEnd());
        double max_value = *std::max_element(queue_to_vector.constBegin(), queue_to_vector.constEnd());

        if (min_value < current_minimum)
          current_minimum = min_value;
        if (max_value > current_maximum)
          current_maximum = max_value;
      }
    }

    if (std::fabs(minimum_received_point_ - current_minimum) > DBL_EPSILON)
    {
      minimum_received_point_ = current_minimum;
      forceAllCurvesToRecalculateMinimum();
    }

    if (std::fabs(maximum_received_point_ - current_maximum) > DBL_EPSILON)
    {
      maximum_received_point_ = current_maximum;
      forceAllCurvesToRecalculateMaximum();
    }
  }
}

ChartWidget::~ChartWidget()
{
  delete chart_settings_;
}

void ChartWidget::detachCurve(const QString& curve_name)
{
  curves_.find(curve_name).value()->qwt_curve_->detach();
  curves_.find(curve_name).value()->is_attached_ = false;
}

bool ChartWidget::hasCurve(const QString& curve_name)
{
  return curves_.contains(curve_name);
}

void ChartWidget::attachNewCurve(const QString& curve_name, const QColor& curve_color, double curve_width)
{
  QwtPlotCurve* curve = new QwtPlotCurve();
  curve->setTitle(curve_name);
  curve->setPen(curve_color, curve_width);
  curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
  curve->attach(qwt_plot_);

  // Add curve to list of curves
  curves_.insert(curve_name, new SignalPlotting::Curve(*curve, curve_color));
}

QColor ChartWidget::getColorForCurve(const QString& curve_name)
{
  return curves_.find(curve_name).value()->curve_color_;
}

double ChartWidget::getWidthForCurve(const QString& curve_name)
{
  return curves_.find(curve_name).value()->curve_width_;
}

void ChartWidget::reattachCurve(const QString& curve_name)
{
  curves_.find(curve_name).value()->qwt_curve_->attach(qwt_plot_);
  curves_.find(curve_name).value()->is_attached_ = true;
}

void ChartWidget::setLastReceivedValue(const QString& curve_name, double last_received_value)
{
  curves_.find(curve_name).value()->last_received_value_ = last_received_value;
  curves_.find(curve_name).value()->has_ecal_received_value_ = true;
}

void ChartWidget::resetScale()
{
  minimum_received_point_ = DBL_MAX;
  maximum_received_point_ = -DBL_MAX;

  forceAllCurvesToRecalculateMinimum();
  forceAllCurvesToRecalculateMinimum();
}

QwtScaleDiv ChartWidget::getCurrentPlotYAxisBounds()
{
  return qwt_plot_->axisScaleDiv(QwtPlot::yLeft);
}

void ChartWidget::stopTimer()
{
  plot_timer_.stop();
}

void ChartWidget::startTimer()
{
  plot_timer_.start();
  timer_first_time_ = true;
}

void ChartWidget::chartSettingsChanged()
{
  // detach previous grid
  qwt_plot_->detachItems(QwtPlotItem::Rtti_PlotGrid);

  QwtPlotGrid* grid = new QwtPlotGrid();
  grid->enableX(chart_settings_->is_enabled_XAxis_major_grid_lines_);
  grid->enableXMin(chart_settings_->is_enabled_XAxis_minor_grid_lines_);

  grid->enableY(chart_settings_->is_enabled_YAxis_major_grid_lines_);
  grid->enableYMin(chart_settings_->is_enabled_YAxis_minor_grid_lines_);

  grid->setMajorPen(QPen(Qt::black, 1.5, Qt::DotLine));
  grid->setMinorPen(QPen(Qt::gray, 0, Qt::DotLine));

  grid->attach(qwt_plot_);

  QwtScaleDiv bounds_YAxis = getCurrentPlotYAxisBounds();

  if (!chart_settings_->compute_scale_YAxis_Minimum_)
    bounds_YAxis.setLowerBound(chart_settings_->bounds_YAxis_to_display_.minValue());

  if (!chart_settings_->compute_scale_YAxis_Maximum_)
    bounds_YAxis.setUpperBound(chart_settings_->bounds_YAxis_to_display_.maxValue());

  // only set axis scale if maximim is greater than minimum
  if (bounds_YAxis.isIncreasing())
  {
    qwt_plot_->setAxisScale(QwtPlot::yLeft, bounds_YAxis.lowerBound(), bounds_YAxis.upperBound());
  }

  if (!chart_settings_->compute_scale_XAxis_)
  {
    auto current_time_interval = time_interval_.maxValue() - time_interval_.minValue();
    if (chart_settings_->width_XAxis_interval_ > current_time_interval)
    {
      // if the new interval is bigger, modify max value 
      time_interval_.setMaxValue(time_interval_.maxValue() - current_time_interval + chart_settings_->width_XAxis_interval_);
    }
    else if (chart_settings_->width_XAxis_interval_ < current_time_interval)
    {
      // if the new interval is lower, modify it so that current point is in the middle of the interval
      time_interval_.setMinValue(current_time_point_ - (chart_settings_->width_XAxis_interval_ / 2.0));
      time_interval_.setMaxValue(current_time_point_ + (chart_settings_->width_XAxis_interval_ / 2.0));
    }

    qwt_plot_->setAxisScale(QwtPlot::xBottom, time_interval_.minValue(), time_interval_.maxValue());

    // also modify maximum number of points
    queue_size_max_ = static_cast<int>(chart_settings_->width_XAxis_interval_ * 1000.0 / SignalPlotting::kPlotWindow);
  }
  else
  {
    time_interval_.setMaxValue(time_interval_.minValue() + SignalPlotting::kDefaultTimeMax);
    queue_size_max_ = static_cast<int>(SignalPlotting::kDefaultTimeMax * 1000.0 / SignalPlotting::kPlotWindow);
  }

  qwt_plot_->setAxisScale(QwtPlot::xBottom, time_interval_.minValue(), time_interval_.maxValue());
  recalculate_scale_timer_.setInterval(time_interval_.width() / 10);

  
  if(chart_settings_->is_enabled_Show_Values_near_cursor_)
    zoomer_->setTrackerMode(QwtPlotPicker::AlwaysOn);
  else
    zoomer_->setTrackerMode(QwtPlotPicker::AlwaysOff);

  for (auto curve : curves_)
  {
    curve->qwt_curve_->setPen(curve->curve_color_, curve->curve_width_);
  }
  if (!chart_settings_->is_enabled_Show_Hide_Paused_Resumed_Lines_)
  {
    qwt_plot_->detachItems(QwtPlotItem::Rtti_PlotMarker);
  }
  qwt_plot_->replot();
}

void ChartWidget::slt_picker(const QRectF& rect)
{
  zoomer_->setZoomBase(rect);
}

void ChartWidget::slt_zoomed(const QRectF& /*rect*/)
{
  if (zoomer_->zoomRectIndex() == 0 && zoomer_state_ == false)
    return;
  if (zoomer_->zoomRectIndex() == 0 && zoomer_state_ == true)
  {
    zoomer_state_ = false;
    zoomed_label_->detach();
    normal_view_label_->attach(qwt_plot_);
    onPlotTimeOut();
    if (current_time_point_ <= 30)
    {
      qwt_plot_->setAxisScale(QwtPlot::xBottom, 0, 30);
    }
    qwt_plot_->replot();
    zoomer_->setZoomBase(0);
  }
  else if (zoomer_->zoomRectIndex() != 0 && zoomer_state_ == false)
  {
    normal_view_label_->detach();
    zoomed_label_->attach(qwt_plot_);
    zoomer_state_ = true;
  }
}

QMap<QString, SignalPlotting::Curve*>* ChartWidget::getAllCurves()
{
  return &curves_;
}

void ChartWidget::addPauseLine()
{
  if (chart_settings_->is_enabled_Show_Hide_Paused_Resumed_Lines_)
  {
    QwtPlotMarker* pause_marker = new QwtPlotMarker();
    pause_marker->setLabel(QwtText("Paused"));
    pause_marker->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
    pause_marker->setLabelOrientation(Qt::Vertical);
    pause_marker->setLineStyle(QwtPlotMarker::VLine);
    pause_marker->setLinePen(QPen(Qt::darkRed, 3, Qt::DashDotLine));
    pause_marker->setXValue(current_time_point_);
    pause_marker->attach(qwt_plot_);
    qwt_plot_->replot();
  }
}

void ChartWidget::addResumeLine()
{
  if (chart_settings_->is_enabled_Show_Hide_Paused_Resumed_Lines_)
  {
    QwtPlotMarker* resumed_marker = new QwtPlotMarker();
    resumed_marker->setLabel(QwtText("Resumed"));
    resumed_marker->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
    resumed_marker->setLabelOrientation(Qt::Vertical);
    resumed_marker->setLineStyle(QwtPlotMarker::VLine);
    resumed_marker->setLinePen(QPen(Qt::darkGreen, 3, Qt::DashDotLine));
    resumed_marker->setXValue(current_time_point_);
    resumed_marker->attach(qwt_plot_);
    qwt_plot_->replot();
  }
}

void ChartWidget::setPluginState(SignalPlotting::PLUGIN_STATE plugin_state)
{
  plugin_state_ = plugin_state;
}

bool ChartWidget::isPointTriggeringRescale(double point)
{
  auto current_scale_interval = qwt_plot_->axisScaleDiv(QwtPlot::yLeft).interval();
  double interval_width = std::abs(current_scale_interval.maxValue() - current_scale_interval.minValue());

  // if point is not in range, trigger rescale
  if (!current_scale_interval.contains(point))
    return true;

  if ((point < (current_scale_interval.minValue() + (interval_width * SignalPlotting::kFactorToRescale))) || 
     (point > (current_scale_interval.maxValue() - (interval_width * SignalPlotting::kFactorToRescale))))
  {
    return true;
  }
  return false;
}

void ChartWidget::forceAllCurvesToRecalculateMinimum()
{
  for (auto curve : curves_.values())
  {
    curve->forced_minimum_scale_ = true;
  }
}

void ChartWidget::forceAllCurvesToRecalculateMaximum()
{
  for (auto curve : curves_.values())
  {
    curve->forced_maximum_scale_ = true;
  }
}
