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

#include "settings_widget.h"

#include "q_ecal_play.h"

#include <QPushButton>
#include <QSettings>

#include <chrono>
#include <cfloat>
#include <math.h>
#include <cmath>

SettingsWidget::SettingsWidget(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);

  // Setup the warning labels
  int label_height = ui_.play_speed_spinbox->sizeHint().height();
  QPixmap warning_icon = QPixmap(":/ecalicons/WARNING").scaled(label_height, label_height, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
  ui_.play_speed_warning_label->setPixmap(warning_icon);
  ui_.play_speed_warning_label->setVisible(false);
  ui_.limit_warning_label->setPixmap(warning_icon);
  ui_.limit_warning_label->setVisible(false);

  // Connect QEcalPlay -> this
  connect(QEcalPlay::instance(), &QEcalPlay::measurementLoadedSignal,                  this, &SettingsWidget::measurementLoaded);
  connect(QEcalPlay::instance(), &QEcalPlay::measurementClosedSignal,                  this, &SettingsWidget::measurementClosed);
  connect(QEcalPlay::instance(), &QEcalPlay::playSpeedChangedSignal,                   this, &SettingsWidget::playSpeedChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::limitPlaySpeedEnabledChangedSignal,       this, &SettingsWidget::limitPlaySpeedEnabledChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::frameDroppingAllowedChangedSignal,        this, &SettingsWidget::frameDroppingAllowedChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::enforceDelayAccuracyEnabledChangedSignal, this, &SettingsWidget::enforceDelayAccuracyEnabledChanged);
  connect(QEcalPlay::instance(), &QEcalPlay::limitIntervalChangedSignal,               this, &SettingsWidget::limitIntervalChanged);

  // Connect this->QEcalPlay
  connect(ui_.play_speed_spinbox,              static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), QEcalPlay::instance(), &QEcalPlay::setPlaySpeed);
  connect(ui_.limit_playback_speed_checkbox,   &QPushButton::clicked, QEcalPlay::instance(), &QEcalPlay::setLimitPlaySpeedEnabled);
  connect(ui_.allow_framedrops_checkbox,       &QCheckBox::toggled,   QEcalPlay::instance(), &QEcalPlay::setFrameDroppingAllowed);
  connect(ui_.enforce_delay_accuracy_checkbox, &QCheckBox::toggled,   QEcalPlay::instance(), &QEcalPlay::setEnforceDelayAccuracyEnabled);

  // Limit settings
  connect(ui_.lower_limit_spinbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &SettingsWidget::lowerLimitSpinboxChanged);
  connect(ui_.upper_limit_spinbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &SettingsWidget::upperLimitSpinboxChanged);

  // Initial GUI settings
  if (QEcalPlay::instance()->isMeasurementLoaded())
  {
    measurementLoaded(QEcalPlay::instance()->measurementPath());
    auto limits      = QEcalPlay::instance()->limitInterval();
    auto time_limits = std::make_pair(QEcalPlay::instance()->timestampOf(limits.first), QEcalPlay::instance()->timestampOf(limits.second));

    if (!((limits.first == 0) && (limits.second == (QEcalPlay::instance()->frameCount() - 1))))
    {
      limitIntervalChanged(limits, time_limits);
    }
  }
  else
  {
    measurementClosed();
  }
  playSpeedChanged(QEcalPlay::instance()->playSpeed());
  frameDroppingAllowedChanged(QEcalPlay::instance()->isFrameDroppingAllowed());
  limitPlaySpeedEnabledChanged(QEcalPlay::instance()->isLimitPlaySpeedEnabled());

  restoreLayout();
}

SettingsWidget::~SettingsWidget()
{
  saveLayout();
}

void SettingsWidget::measurementLoaded(const QString& /*path*/)
{
  measurement_boundaries_ = QEcalPlay::instance()->measurementBoundaries();

  ui_.lower_limit_spinbox->setMaximum(std::chrono::duration_cast<std::chrono::duration<double>>(measurement_boundaries_.second - measurement_boundaries_.first).count());
  ui_.upper_limit_spinbox->setMaximum(std::chrono::duration_cast<std::chrono::duration<double>>(measurement_boundaries_.second - measurement_boundaries_.first).count() + 0.005);

  ui_.lower_limit_spinbox->blockSignals(true);
  ui_.upper_limit_spinbox->blockSignals(true);
  ui_.lower_limit_spinbox->setValue(0.0);
  ui_.upper_limit_spinbox->setValue(ui_.upper_limit_spinbox->maximum());
  ui_.lower_limit_spinbox->blockSignals(false);
  ui_.upper_limit_spinbox->blockSignals(false);

  ui_.lower_limit_spinbox->setEnabled(true);
  ui_.upper_limit_spinbox->setEnabled(true);
}

void SettingsWidget::measurementClosed()
{
  measurement_boundaries_.first = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));
  measurement_boundaries_.second = eCAL::Time::ecal_clock::time_point(std::chrono::nanoseconds(0));

  ui_.lower_limit_spinbox->blockSignals(true);
  ui_.upper_limit_spinbox->blockSignals(true);
  ui_.lower_limit_spinbox->setMaximum(0.0);
  ui_.upper_limit_spinbox->setMaximum(0.0);
  ui_.lower_limit_spinbox->blockSignals(false);
  ui_.upper_limit_spinbox->blockSignals(false);

  ui_.lower_limit_spinbox->setEnabled(false);
  ui_.upper_limit_spinbox->setEnabled(false);
}

void SettingsWidget::playSpeedChanged(double play_speed)
{
  if (std::fabs(ui_.play_speed_spinbox->value() - play_speed) > DBL_EPSILON)
  {
    ui_.play_speed_spinbox->blockSignals(true);
    ui_.play_speed_spinbox->setValue(play_speed);
    ui_.play_speed_spinbox->blockSignals(false);
  }

  ui_.play_speed_warning_label->setVisible(std::fabs(play_speed) < DBL_EPSILON);
}

void SettingsWidget::limitPlaySpeedEnabledChanged(bool enabled)
{
  if (ui_.limit_playback_speed_checkbox->isChecked() != enabled)
  {
    ui_.limit_playback_speed_checkbox->blockSignals(true);
    ui_.limit_playback_speed_checkbox->setChecked(enabled);
    ui_.limit_playback_speed_checkbox->blockSignals(false);
  }
  ui_.play_speed_spinbox->setEnabled(enabled);
}

void SettingsWidget::frameDroppingAllowedChanged(bool allowed)
{
  if (ui_.allow_framedrops_checkbox->isChecked() != allowed)
  {
    ui_.allow_framedrops_checkbox->blockSignals(true);
    ui_.allow_framedrops_checkbox->setChecked(allowed);
    ui_.allow_framedrops_checkbox->blockSignals(false);
  }
}

void SettingsWidget::enforceDelayAccuracyEnabledChanged(bool enabled)
{
  if (ui_.enforce_delay_accuracy_checkbox->isChecked() != enabled)
  {
    ui_.enforce_delay_accuracy_checkbox->blockSignals(true);
    ui_.enforce_delay_accuracy_checkbox->setChecked(enabled);
    ui_.enforce_delay_accuracy_checkbox->blockSignals(false);
  }
}

void SettingsWidget::limitIntervalChanged(const std::pair<long long, long long> /*indexes*/, const std::pair<eCAL::Time::ecal_clock::time_point, eCAL::Time::ecal_clock::time_point>& time_points)
{
  ui_.lower_limit_spinbox->blockSignals(true);
  ui_.upper_limit_spinbox->blockSignals(true);

  double lower_limit = std::chrono::duration_cast<std::chrono::duration<double>>(time_points.first - measurement_boundaries_.first).count();
  double upper_limit = std::chrono::duration_cast<std::chrono::duration<double>>(time_points.second - measurement_boundaries_.first).count();

  ui_.lower_limit_spinbox->setValue(lower_limit);
  ui_.upper_limit_spinbox->setValue(upper_limit);

  ui_.lower_limit_spinbox->blockSignals(false);
  ui_.upper_limit_spinbox->blockSignals(false);


  ui_.limit_warning_label->setVisible(QEcalPlay::instance()->isMeasurementLoaded() && (upper_limit <= lower_limit));

}

void SettingsWidget::lowerLimitSpinboxChanged(double value)
{
  if (ui_.upper_limit_spinbox->value() < value)
  {
    ui_.upper_limit_spinbox->blockSignals(true);
    ui_.upper_limit_spinbox->setValue(value);
    ui_.upper_limit_spinbox->blockSignals(false);
  }

  auto lower_limit = measurement_boundaries_.first + std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(value));
  auto upper_limit = measurement_boundaries_.first + std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(ui_.upper_limit_spinbox->value()));

  QEcalPlay::instance()->setLimitInterval(std::make_pair(lower_limit, upper_limit));
}

void SettingsWidget::upperLimitSpinboxChanged(double value)
{
  if (ui_.lower_limit_spinbox->value() > value)
  {
    ui_.lower_limit_spinbox->blockSignals(true);
    ui_.lower_limit_spinbox->setValue(value);
    ui_.lower_limit_spinbox->blockSignals(false);
  }

  auto lower_limit = measurement_boundaries_.first + std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(ui_.lower_limit_spinbox->value()));
  auto upper_limit = measurement_boundaries_.first + std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(value));

  QEcalPlay::instance()->setLimitInterval(std::make_pair(lower_limit, upper_limit));
}

////////////////////////////////////////////////////////////////////////////////
//// Save Layout                                                            ////
////////////////////////////////////////////////////////////////////////////////

void SettingsWidget::saveLayout()
{
  QSettings settings;
  settings.beginGroup("settings");

  settings.setValue("limit_play_speed_enabled", QEcalPlay::instance()->isLimitPlaySpeedEnabled());
  settings.setValue("speed",                    QEcalPlay::instance()->playSpeed());
  settings.setValue("framedropping_allowed",    QEcalPlay::instance()->isFrameDroppingAllowed());
  settings.setValue("enforce_delay_accuracy",   QEcalPlay::instance()->isEnforceDelayAccuracyEnabled());
  settings.setValue("repeat",                   QEcalPlay::instance()->isRepeatEnabled());

  settings.endGroup();
}

void SettingsWidget::restoreLayout()
{
  QSettings settings;
  settings.beginGroup("settings");

  QVariant limit_play_speed_enabled_variant = settings.value("limit_play_speed_enabled");
  QVariant speed_variant                    = settings.value("speed");
  QVariant framedropping_allowed_variant    = settings.value("framedropping_allowed");
  QVariant enforce_delay_accuracy_variant   = settings.value("enforce_delay_accuracy");
  QVariant repeat_variant                   = settings.value("repeat");

  if (limit_play_speed_enabled_variant.isValid())
  {
    QEcalPlay::instance()->setLimitPlaySpeedEnabled(limit_play_speed_enabled_variant.toBool());
  }
  if (speed_variant.isValid())
  {
    QEcalPlay::instance()->setPlaySpeed(speed_variant.toDouble());
  }
  if (framedropping_allowed_variant.isValid())
  {
    QEcalPlay::instance()->setFrameDroppingAllowed(framedropping_allowed_variant.toBool());
  }
  if (enforce_delay_accuracy_variant.isValid())
  {
    QEcalPlay::instance()->setEnforceDelayAccuracyEnabled(enforce_delay_accuracy_variant.toBool());
  }
  if (repeat_variant.isValid())
  {
    QEcalPlay::instance()->setRepeatEnabled(repeat_variant.toBool());
  }

  settings.endGroup();
}

void SettingsWidget::resetLayout()
{
  QEcalPlay::instance()->setLimitInterval              (measurement_boundaries_);
  QEcalPlay::instance()->setLimitPlaySpeedEnabled      (true);
  QEcalPlay::instance()->setPlaySpeed                  (1.0);
  QEcalPlay::instance()->setFrameDroppingAllowed       (false);
  QEcalPlay::instance()->setEnforceDelayAccuracyEnabled(false);
  QEcalPlay::instance()->setRepeatEnabled              (false);
}
