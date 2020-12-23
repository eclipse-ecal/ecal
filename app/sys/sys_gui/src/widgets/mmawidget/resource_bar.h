/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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
#include "ui_resource_bar.h"

class ResourceBar : public QWidget, public Ui::ResourceBar
{
    Q_OBJECT

public:
  ResourceBar(QWidget *parent = Q_NULLPTR) : QWidget(parent) 
  { 
    setupUi(this); 
#ifdef WIN32
    // On Windows, the progress bar is green. The default white text is very hard to read, so we modify that color.
    QPalette pal = bar->palette();
    pal.setColor(QPalette::HighlightedText, pal.color(QPalette::Text));
    bar->setPalette(pal);
#endif
  }
  ~ResourceBar() {}

public slots:
  void setRelativeValue(double percentage)
  {
    int value = (int)((double)(bar->maximum() - bar->minimum()) * percentage + 0.5) + bar->minimum();
    bar->setValue(value);
  }
};
