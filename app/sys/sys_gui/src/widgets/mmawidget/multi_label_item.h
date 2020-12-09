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
#include <QLabel>

class MultiLabelItem : public QWidget
{
  Q_OBJECT

public:
  MultiLabelItem(unsigned int number_of_labels, QWidget *parent = Q_NULLPTR);
  ~MultiLabelItem();

  void setText(unsigned int number, const QString& text);
  void setPixmap(unsigned int number, const QPixmap& pixmap);
  void setMinimumWidth(unsigned int number, int width);

  void setFont(unsigned int number, const QFont& font);
  void setStyleSheet(unsigned int number, const QString& stylesheet);

private:
  QVector<QLabel*> label_list_;
  QVector<int> default_widths_;
};
