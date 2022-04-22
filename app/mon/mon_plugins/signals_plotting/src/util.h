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

#include <QString>
#include <QVariant>
#include <QUuid>
#include <QUrl>
#include <QQueue>

#include "qwt_plot_curve.h"
#include "qwt_scale_div.h"

namespace QtUtil
{
  inline QString variantToString(const QVariant& variant)
  {
    switch ((QMetaType::Type)variant.type())
    {
    case QMetaType::Bool:
      return variant.toBool() ? "True" : "False";
    case QMetaType::QByteArray:
      return variant.toByteArray().toHex();
    case QMetaType::Char:
      return variant.toChar();
    case QMetaType::Double:
      return QString::number(variant.toDouble());
    case QMetaType::Float:
      return QString::number(variant.toFloat());
    case QMetaType::QUuid:
      return variant.toUuid().toString();
    case QMetaType::Int:
      return QString::number(variant.toInt());
    case QMetaType::LongLong:
      return QString::number (variant.toLongLong());
    case QMetaType::QString:
      return variant.toString();
    case QMetaType::UInt:
      return QString::number(variant.toUInt());
    case QMetaType::ULongLong:
      return QString::number(variant.toULongLong());
    case QMetaType::QUrl:
      return variant.toUrl().toString();
    default:
      return "";
    }
  }
}

namespace SignalPlotting
{
  static     QString KMultiTabName                       = QString("MULTI");
  constexpr  double  kDefaultTimeMax                     = 30;
  constexpr  double  kDefaultMaximumFactor               = 1.2;
  constexpr  double  kDefaultMinimumFactor               = 0.8;
  constexpr  double  kDeltaFactor                        = 0.5;
  constexpr  double  kFactorToRescale                    = 0.05;
  constexpr  double  kDefaultCurveWidth                  = 3.0;
  constexpr  int     kPlotWindow                         = 10;
  constexpr  int     kTreeItemBackgroundAlpha            = 150;
  constexpr  double  kMinimumAllowedIntervalDifference   = 1e-306;
  constexpr  double  kFactorToIncreaseIntervalDifference = 1e-305;

  enum class PLUGIN_STATE : int
  {
    UNKNOWN = 0,
    PAUSED,
    RUNNING
  };

  struct Curve
  {
    Curve(QwtPlotCurve& curve, QColor curve_color) :
      qwt_curve_(&curve),
      queue_received_values_(QQueue<double>()),
      queue_time_values_(QQueue<double>()),
      curve_color_(curve_color),
      curve_width_(kDefaultCurveWidth),
      last_received_value_(0),
      has_color_changed_(false),
      has_width_changed(false),
      is_attached_(true),
      has_ecal_received_value_(false),
      forced_minimum_scale_(false),
      forced_maximum_scale_(false)
    {}
    ~Curve()
    {
      delete qwt_curve_;
    }

    QwtPlotCurve* qwt_curve_;

    QQueue<double> queue_received_values_;
    QQueue<double> queue_time_values_;

    QColor curve_color_;

    double curve_width_;
    double last_received_value_;

    bool has_color_changed_;
    bool has_width_changed;

    bool is_attached_;
    bool has_ecal_received_value_;
    bool forced_minimum_scale_;
    bool forced_maximum_scale_;
  };

  struct ChartCustomSettings
  {
    bool compute_scale_XAxis_;
    // variable used to know if the scale was ever modified, then populate spinbox with old value
    bool was_changed_XAxis_Scale_;

    bool compute_scale_YAxis_Minimum_;
    // variable used to know if the scale was ever modified, then populate spinbox with old value
    bool was_changed_YAxis_Scale_Minimum_;

    bool compute_scale_YAxis_Maximum_;
    // variable used to know if the scale was ever modified, then populate spinbox with old value
    bool was_changed_YAxis_Scale_Maximum_;

    bool is_enabled_XAxis_major_grid_lines_;
    bool is_enabled_XAxis_minor_grid_lines_;

    bool is_enabled_YAxis_major_grid_lines_;
    bool is_enabled_YAxis_minor_grid_lines_;

    bool is_enabled_Show_Values_near_cursor_;
    bool is_enabled_Show_Hide_Paused_Resumed_Lines_;


    QwtInterval bounds_YAxis_to_display_;
    double width_XAxis_interval_;

    QwtScaleDiv current_YAxis_bounds_;


    ChartCustomSettings() :
      compute_scale_XAxis_(true),
      was_changed_XAxis_Scale_(false),
      compute_scale_YAxis_Minimum_(true),
      was_changed_YAxis_Scale_Minimum_(false),
      compute_scale_YAxis_Maximum_(true),
      was_changed_YAxis_Scale_Maximum_(false),
      is_enabled_XAxis_major_grid_lines_(false),
      is_enabled_XAxis_minor_grid_lines_(false),
      is_enabled_YAxis_major_grid_lines_(false),
      is_enabled_YAxis_minor_grid_lines_(false),
      is_enabled_Show_Values_near_cursor_(false),
      is_enabled_Show_Hide_Paused_Resumed_Lines_(false),
      bounds_YAxis_to_display_(0.0, 0.0),
      width_XAxis_interval_(kDefaultTimeMax),
      current_YAxis_bounds_(0, 0)
    {}
  };

  struct Color {
    int r, g, b, a;

    Color(const QColor& color)
    {
      r = color.red();
      g = color.green();
      b = color.blue();
      a = color.alpha();
    }

    void clear()
    {
      r = QColor().red();
      g = QColor().green();
      b = QColor().blue();
      a = QColor().alpha();
    }

    QColor toQColor() const
    {
      return QColor(r, g, b, a);
    }

    bool isValid() const
    {
      if (a != kTreeItemBackgroundAlpha)
        return false;
      return true;
    }
  };
}


