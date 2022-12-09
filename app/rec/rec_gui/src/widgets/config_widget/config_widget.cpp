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
#include <QFileDialog>
#include "qecalrec.h"

#include <EcalParser/EcalParser.h>

ConfigWidget::ConfigWidget(QWidget *parent)
  : QWidget(parent)
  , first_show_event_(true)
{
  ui_.setupUi(this);

  connect(ui_.measurement_directory_lineedit, &QLineEdit::textChanged,  QEcalRec::instance(), [this]() {QEcalRec::instance()->setMeasRootDir(ui_.measurement_directory_lineedit->text().toStdString()); });
  connect(ui_.measurement_name_lineedit,      &QLineEdit::textChanged,  QEcalRec::instance(), [this]() {QEcalRec::instance()->setMeasName   (ui_.measurement_name_lineedit->text().toStdString()); });
  connect(ui_.description_textedit,           &QTextEdit::textChanged,  QEcalRec::instance(), [this]() {QEcalRec::instance()->setDescription(ui_.description_textedit->toPlainText().toStdString()); });
  connect(ui_.max_file_size_spinbox, static_cast<void (QSpinBox:: *)(int)>(&QSpinBox::valueChanged), QEcalRec::instance(), [](int megabytes) {QEcalRec::instance()->setMaxFileSizeMib(megabytes); });
  connect(ui_.one_file_per_topic_checkbox,    &QCheckBox::stateChanged, QEcalRec::instance(), [this]() {QEcalRec::instance()->setOneFilePerTopicEnabled(ui_.one_file_per_topic_checkbox->isChecked()); });
  connect(ui_.user_meass_rec_path_toolButton, &QToolButton::clicked, this, &ConfigWidget::userRecPathButtonPressed);

  connect(ui_.refresh_path_preview_button,    &QAbstractButton::clicked,            QEcalRec::instance(), [this]() { updatePathPreviewAndWarningLabel(); });

  connect(QEcalRec::instance(), &QEcalRec::measRootDirChangedSignal,    this, &ConfigWidget::measurementRootDirectoryChanged);
  connect(QEcalRec::instance(), &QEcalRec::measNameChangedSignal,       this, &ConfigWidget::measurementNameChanged);
  connect(QEcalRec::instance(), &QEcalRec::maxFileSizeMibChangedSignal, this, &ConfigWidget::maxFileSizeChanged);
  connect(QEcalRec::instance(), &QEcalRec::descriptionChangedSignal,    this, &ConfigWidget::descriptionChanged);
  connect(QEcalRec::instance(), &QEcalRec::oneFilePerTopicEnabledChangedSignal,    this, &ConfigWidget::oneFilePerTopicEnabledChanged);


  measurementRootDirectoryChanged (QEcalRec::instance()->measRootDir());
  measurementNameChanged          (QEcalRec::instance()->measName());
  descriptionChanged              (QEcalRec::instance()->description());
  maxFileSizeChanged              (QEcalRec::instance()->maxFileSizeMib());
  updatePathPreviewAndWarningLabel();
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

void ConfigWidget::oneFilePerTopicEnabledChanged(bool enabled)
{
  if (enabled != ui_.one_file_per_topic_checkbox->isChecked())
  {
    ui_.one_file_per_topic_checkbox->blockSignals(true);
    ui_.one_file_per_topic_checkbox->setChecked(enabled);
    ui_.one_file_per_topic_checkbox->blockSignals(false);
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
  updatePathPreviewAndWarningLabel();
}

void ConfigWidget::measurementNameChanged(const std::string& name)
{
  if (name.c_str() != ui_.measurement_name_lineedit->text())
  {
    ui_.measurement_name_lineedit->blockSignals(true);
    ui_.measurement_name_lineedit->setText(name.c_str());
    ui_.measurement_name_lineedit->blockSignals(false);
  }
  updatePathPreviewAndWarningLabel();
}

void ConfigWidget::updatePathPreviewAndWarningLabel()
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

  ui_.measurement_name_warning->setVisible(parsed_meas_name.empty());
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

void ConfigWidget::userRecPathButtonPressed()
{
  auto parsed_meas_root = QString::fromStdString(EcalParser::Evaluate(ui_.measurement_directory_lineedit->text().toStdString(), true, std::chrono::system_clock::now()));
  QString root_path;
  QDir root(parsed_meas_root);

  if (root.exists() && !parsed_meas_root.isEmpty())
  {
    if (parsed_meas_root[parsed_meas_root.size() - 1] != "\\" && parsed_meas_root[parsed_meas_root.size() - 1] != "/")
      root_path = parsed_meas_root + "/";
    else
      root_path = parsed_meas_root;
  }
  else
  {
    auto current_dir = QDir::currentPath().split("/");
#ifdef __linux__
    current_dir.removeAll("");
    root_path = "/" + current_dir[0];
#else //linux
    root_path = current_dir[0].append("/");
#endif
  }
  QString user_path = QFileDialog::getExistingDirectory(this, "Choose root recording folder...", root_path, QFileDialog::ShowDirsOnly);
  if (!user_path.isEmpty())
  {
    ui_.measurement_directory_lineedit->setText(user_path);
    measurementRootDirectoryChanged(user_path.toStdString());
  }
}

void ConfigWidget::showEvent(QShowEvent* /*event*/)
{
  if (first_show_event_)
  {
    ui_.measurement_name_warning->setPixmap(QPixmap(":/ecalicons/WARNING").scaled(ui_.measurement_name_label->sizeHint().height(), ui_.measurement_name_label->sizeHint().height(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    first_show_event_ = false;
  }
}