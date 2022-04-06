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

#include <QWidget>
#include "CustomQt/QStableSortFilterProxyModel.h"
#include "ui_system_information_widget.h"

#include <QUrl>

class SystemInformationWidget : public QWidget
{
  Q_OBJECT

public:
  SystemInformationWidget(QWidget *parent = Q_NULLPTR);
  ~SystemInformationWidget();

protected:
  virtual void changeEvent(QEvent* event) override;

private:
  void setLabelText();
  static QString toHtml(const QString& system_information);

  void openEcalIni(const QUrl& url);
#ifdef __linux__
  bool openEcalIniElevated(const QUrl& url);
#endif

private:
  Ui::SystemInformationWidget ui_;
};
