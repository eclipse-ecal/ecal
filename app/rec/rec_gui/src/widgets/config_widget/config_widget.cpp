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

#include "config_widget.h"

#include <QDir>
#include "qecalrec.h"

#include <EcalParser/EcalParser.h>

ConfigWidget::ConfigWidget(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);

  connect(ui_.measurement_directory_lineedit, &QLineEdit::textChanged, QEcalRec::instance(), [this]() {QEcalRec::instance()->setMeasRootDir(ui_.measurement_directory_lineedit->text().toStdString()); });
  connect(ui_.measurement_name_lineedit,      &QLineEdit::textChanged, QEcalRec::instance(), [this]() {QEcalRec::instance()->setMeasName   (ui_.measurement_name_lineedit->text().toStdString()); });
  connect(ui_.description_textedit,           &QTextEdit::textChanged, QEcalRec::instance(), [this]() {QEcalRec::instance()->setDescription(ui_.description_textedit->toPlainText().toStdString()); });
  connect(ui_.max_file_size_spinbox, static_cast<void (QSpinBox:: *)(int)>(&QSpinBox::valueChanged), QEcalRec::instance(), [](int megabytes) {QEcalRec::instance()->setMaxFileSizeMib(megabytes); });

  connect(ui_.refresh_path_preview_button,    &QAbstractButton::clicked,            QEcalRec::instance(), [this]() { updatePathPreview(); });

  connect(QEcalRec::instance(), &QEcalRec::measRootDirChangedSignal,    this, &ConfigWidget::measurementRootDirectoryChanged);
  connect(QEcalRec::instance(), &QEcalRec::measNameChangedSignal,       this, &ConfigWidget::measurementNameChanged);
  connect(QEcalRec::instance(), &QEcalRec::maxFileSizeMibChangedSignal, this, &ConfigWidget::maxFileSizeChanged);
  connect(QEcalRec::instance(), &QEcalRec::descriptionChangedSignal,    this, &ConfigWidget::descriptionChanged);

  measurementRootDirectoryChanged(QEcalRec::instance()->measRootDir());
  measurementNameChanged         (QEcalRec::instance()->measName());
  descriptionChanged             (QEcalRec::instance()->description());
  maxFileSizeChanged             (QEcalRec::instance()->maxFileSizeMib());
  updatePathPreview              ();
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::maxFileSizeChanged(unsigned int megabytes)
{
  if ((int)megabytes != ui_.max_file_size_spinbox->value())
  {
    ui_.max_file_size_spinbox->blockSignals(true);
    ui_.max_file_size_spinbox->setValue((int)megabytes);
    ui_.max_file_size_spinbox->blockSignals(false);
  }
}

void ConfigWidget::measurementRootDirectoryChanged(const std::string& root_dir)
{
  if (root_dir.c_str() != ui_.measurement_directory_lineedit->text())
  {
    ui_.measurement_directory_lineedit->blockSignals(true);
    ui_.measurement_directory_lineedit->setText(root_dir.c_str());
    ui_.measurement_directory_lineedit->blockSignals(false);
  }
  updatePathPreview();
}

void ConfigWidget::measurementNameChanged(const std::string& name)
{
  if (name.c_str() != ui_.measurement_name_lineedit->text())
  {
    ui_.measurement_name_lineedit->blockSignals(true);
    ui_.measurement_name_lineedit->setText(name.c_str());
    ui_.measurement_name_lineedit->blockSignals(false);
  }
  updatePathPreview();
}

void ConfigWidget::updatePathPreview()
{
  auto now = std::chrono::system_clock::now();

  const std::string unparsed_meas_root = QEcalRec::instance()->measRootDir();
  const std::string unparsed_meas_name = QEcalRec::instance()->measName();

  const std::string parsed_meas_root = EcalParser::Evaluate(unparsed_meas_root, true, now);
  const std::string parsed_meas_name = EcalParser::Evaluate(unparsed_meas_name, true, now);

  std::string complete_path = parsed_meas_root;

  if ((!complete_path.empty() && (complete_path[complete_path.size() - 1] != '\\') && (complete_path[complete_path.size() - 1] != '/'))
    && (!parsed_meas_name.empty() && (parsed_meas_name[0] != '\\') && (parsed_meas_name[0] != '/')))
  {
    complete_path += "/";
  }
  complete_path += parsed_meas_name;

  QString q_path = QString::fromStdString(complete_path);
  ui_.path_preview_label->setText(q_path);
  ui_.path_preview_label->setToolTip(q_path);
}

void ConfigWidget::descriptionChanged(const std::string& description)
{
  QString q_description = QString(description.c_str());
  if (q_description != ui_.description_textedit->toPlainText())
  {
    ui_.description_textedit->blockSignals(true);
    ui_.description_textedit->setPlainText(q_description);
    ui_.description_textedit->blockSignals(false);
  }
}