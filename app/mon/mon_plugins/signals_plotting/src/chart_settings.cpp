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

#include "chart_settings.h"
#include "ui_chart_settings.h"

ChartSettings::ChartSettings(SignalPlotting::ChartCustomSettings& chart_settings, QMap<QString, SignalPlotting::Curve*>& curves, QWidget* parent) :
  QDialog(parent),
  ui_(new Ui::ChartSettings),
  chart_settings_(&chart_settings),
  curves_(&curves)
{
  ui_->setupUi(this);

  // change buttons order
  ui_->buttonBox->button(QDialogButtonBox::Apply)->setDefault(true);
  ui_->buttonBox->setStyleSheet("* { button-layout: 2 }");

  ui_->autoScaleXAxis_checkbox->setChecked(chart_settings_->compute_scale_XAxis_);
  emit scaleXAxisChanged(chart_settings_->compute_scale_XAxis_);

  ui_->autoScaleMinimum_checkbox->setChecked(chart_settings_->compute_scale_YAxis_Minimum_);
  emit scaleYAxisMinimumChanged(chart_settings_->compute_scale_YAxis_Minimum_);

  ui_->autoScaleMaximum_checkbox->setChecked(chart_settings_->compute_scale_YAxis_Maximum_);
  emit scaleYAxisMaximumChanged(chart_settings_->compute_scale_YAxis_Maximum_);

  ui_->enableGridMajorX_CB->setChecked(chart_settings_->is_enabled_XAxis_major_grid_lines_);
  ui_->enableGridMinorX_CB->setChecked(chart_settings_->is_enabled_XAxis_minor_grid_lines_);
  emit enableXAxisMajorLines(chart_settings_->is_enabled_XAxis_major_grid_lines_);

  ui_->enableGridMajorY_CB->setChecked(chart_settings_->is_enabled_YAxis_major_grid_lines_);
  ui_->enableGridMinorY_CB->setChecked(chart_settings_->is_enabled_YAxis_minor_grid_lines_);
  emit enableYAxisMajorLines(chart_settings_->is_enabled_YAxis_major_grid_lines_);

  ui_->minimum_SB->setValue(chart_settings_->bounds_YAxis_to_display_.minValue());

  ui_->enableShowValuesCheckBox->setChecked(chart_settings_->is_enabled_Show_Values_near_cursor_);
  emit cursorChanged(chart_settings_->is_enabled_Show_Values_near_cursor_);

  ui_->show_hidePausedResumedLinescheckBox->setChecked(chart_settings_->is_enabled_Show_Hide_Paused_Resumed_Lines_);
  emit linesChanged(chart_settings_->is_enabled_Show_Hide_Paused_Resumed_Lines_);


  // if maximum is autoplotted, set maximum value for minimum spinbox, the current maximum
  // if maximum is not autoplotted, set maximum value for minimum spinbox, the specified value
  if (chart_settings_->compute_scale_YAxis_Maximum_)
  {
    ui_->minimum_SB->setMaximum(chart_settings_->current_YAxis_bounds_.upperBound());
  }
  else
  {
    ui_->minimum_SB->setMaximum(chart_settings_->bounds_YAxis_to_display_.maxValue());
  }

  ui_->maximum_SB->setValue(chart_settings_->bounds_YAxis_to_display_.maxValue());

  // if minimum is autoplotted, set minimum value for maximum spinbox, the current minimum
  // if minimum is not autoplotted, set minimum value for maximum spinbox, the specied value
  if (chart_settings_->compute_scale_YAxis_Minimum_)
  {
    ui_->maximum_SB->setMinimum(chart_settings_->current_YAxis_bounds_.lowerBound());
  }
  else
  {
    ui_->maximum_SB->setMinimum(chart_settings_->bounds_YAxis_to_display_.minValue());
  }

  ui_->timeFrame_SB->setValue(chart_settings_->width_XAxis_interval_);

  // Curve settings
  connect(ui_->curves_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ChartSettings::currentCurveChanged);
  ui_->curves_comboBox->addItems(curves_->keys());
  ui_->curves_comboBox->setCurrentIndex(0);

  connect(ui_->color_PushButton, &QPushButton::clicked, this, &ChartSettings::openColorDialog);

  color_dialog_ = new QColorDialog(this);
  color_dialog_->setOptions(QColorDialog::DontUseNativeDialog);
  connect(color_dialog_, &QColorDialog::colorSelected, this, &ChartSettings::curveColorChanged);

  connect(ui_->curveWidth_SB, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartSettings::curveWidthChanged);

  connect(ui_->autoScaleXAxis_checkbox, &QCheckBox::stateChanged, this, &ChartSettings::scaleXAxisChanged);
  connect(ui_->autoScaleMinimum_checkbox, &QCheckBox::stateChanged, this, &ChartSettings::scaleYAxisMinimumChanged);
  connect(ui_->autoScaleMaximum_checkbox, &QCheckBox::stateChanged, this, &ChartSettings::scaleYAxisMaximumChanged);
  connect(ui_->enableShowValuesCheckBox, &QCheckBox::stateChanged, this, &ChartSettings::cursorChanged);
  connect(ui_->show_hidePausedResumedLinescheckBox, &QCheckBox::stateChanged, this, &ChartSettings::linesChanged);

  connect(ui_->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &ChartSettings::cancelDialog);
  connect(ui_->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ChartSettings::applyChanges);

  connect(ui_->enableGridMajorX_CB, &QCheckBox::stateChanged, this, &ChartSettings::enableXAxisMajorLines);
  connect(ui_->enableGridMajorY_CB, &QCheckBox::stateChanged, this, &ChartSettings::enableYAxisMajorLines);
}


ChartSettings::~ChartSettings()
{
  delete ui_;
}

void ChartSettings::scaleXAxisChanged(int /*state*/)
{
  ui_->timeFrame_SB->setEnabled(!ui_->autoScaleXAxis_checkbox->isChecked());
}

void ChartSettings::scaleYAxisMinimumChanged(int /*state*/)
{
  ui_->minimum_SB->setEnabled(!ui_->autoScaleMinimum_checkbox->isChecked());
}

void ChartSettings::scaleYAxisMaximumChanged(int /*state*/)
{
  ui_->maximum_SB->setEnabled(!ui_->autoScaleMaximum_checkbox->isChecked());
}

void ChartSettings::cursorChanged(int state)
{
  if (state == Qt::Unchecked)
  {
    ui_->enableShowValuesCheckBox->setChecked(state);
  }
}

void ChartSettings::linesChanged(int state)
{
  if (state == Qt::Unchecked)
  {
    ui_->show_hidePausedResumedLinescheckBox->setChecked(state);
  }
}

void ChartSettings::enableXAxisMajorLines(int state)
{
  // if major grid lines are disabled, uncheck minor and disable
  if (state == Qt::Unchecked)
  {
    ui_->enableGridMinorX_CB->setChecked(state);
  }
  ui_->enableGridMinorX_CB->setEnabled(state);
}

void ChartSettings::enableYAxisMajorLines(int state)
{
  // if major grid lines are disabled, uncheck minor and disable
  if (state == Qt::Unchecked)
  {
    ui_->enableGridMinorY_CB->setChecked(state);
  }
  ui_->enableGridMinorY_CB->setEnabled(state);
}

void ChartSettings::currentCurveChanged(int index)
{
  QString qss = QString("background-color: %1").arg(curves_->values().at(index)->curve_color_.name());
  ui_->color_PushButton->setStyleSheet(qss);
  ui_->curveWidth_SB->setValue(curves_->values().at(index)->curve_width_);
}

void ChartSettings::openColorDialog()
{
  color_dialog_->setCurrentColor(curves_->values().at(ui_->curves_comboBox->currentIndex())->curve_color_);
  color_dialog_->show();
}

void ChartSettings::curveColorChanged(const QColor& color)
{
  curves_->values().at(ui_->curves_comboBox->currentIndex())->curve_color_ = color;
  curves_->values().at(ui_->curves_comboBox->currentIndex())->has_color_changed_ = true;
  currentCurveChanged(ui_->curves_comboBox->currentIndex());
}

void ChartSettings::curveWidthChanged(double width)
{
  curves_->values().at(ui_->curves_comboBox->currentIndex())->curve_width_ = width;
  curves_->values().at(ui_->curves_comboBox->currentIndex())->has_width_changed = true;
}

void ChartSettings::cancelDialog()
{
  this->reject();
}

void ChartSettings::applyChanges()
{
  chart_settings_->compute_scale_XAxis_ = ui_->autoScaleXAxis_checkbox->isChecked();
  chart_settings_->compute_scale_YAxis_Minimum_ = ui_->autoScaleMinimum_checkbox->isChecked();
  chart_settings_->compute_scale_YAxis_Maximum_ = ui_->autoScaleMaximum_checkbox->isChecked();

  chart_settings_->is_enabled_XAxis_major_grid_lines_ = ui_->enableGridMajorX_CB->isChecked();
  chart_settings_->is_enabled_XAxis_minor_grid_lines_ = ui_->enableGridMinorX_CB->isChecked();

  chart_settings_->is_enabled_YAxis_major_grid_lines_ = ui_->enableGridMajorY_CB->isChecked();
  chart_settings_->is_enabled_YAxis_minor_grid_lines_ = ui_->enableGridMinorY_CB->isChecked();

  if (!ui_->autoScaleXAxis_checkbox->isChecked())
  {
    chart_settings_->was_changed_XAxis_Scale_ = true;
  }

  if (!ui_->autoScaleMinimum_checkbox->isChecked())
  {
    chart_settings_->was_changed_YAxis_Scale_Minimum_ = true;
  }

  if (!ui_->autoScaleMaximum_checkbox->isChecked())
  {
    chart_settings_->was_changed_YAxis_Scale_Maximum_ = true;
  }

  chart_settings_->bounds_YAxis_to_display_.setInterval(ui_->minimum_SB->value(), ui_->maximum_SB->value());
  chart_settings_->width_XAxis_interval_ = ui_->timeFrame_SB->value();

  chart_settings_->is_enabled_Show_Values_near_cursor_ = ui_->enableShowValuesCheckBox->isChecked();
  chart_settings_->is_enabled_Show_Hide_Paused_Resumed_Lines_ = ui_->show_hidePausedResumedLinescheckBox->isChecked();

  emit updateChartSettings();
  this->accept();
}
