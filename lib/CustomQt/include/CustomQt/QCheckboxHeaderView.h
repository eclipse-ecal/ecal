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

#include <QHeaderView>

#include <QPainter>
#include <QMouseEvent>

class QCheckboxHeaderView : public QHeaderView
{
  Q_OBJECT

public:
  QCheckboxHeaderView(QWidget* parent = Q_NULLPTR);
  ~QCheckboxHeaderView();

  void setCheckable(int column, bool checkable = true);
  void setCheckboxEnabled(int column, bool enabled = true);
  void setChecked(int column, bool checked = true);
  void setCheckState(int column, Qt::CheckState checked = Qt::CheckState::Checked);

  bool isCheckable(int column) const;
  bool isCheckboxEnabled(int column) const;
  bool isChecked(int column) const;
  Qt::CheckState checkState(int column) const;

signals:
  void stateChanged(int column, Qt::CheckState state);
  void clicked(int column, bool checked);

protected:
  void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;

  void mousePressEvent(QMouseEvent *event);

private:
  std::map<int, std::tuple<bool, bool, Qt::CheckState>> checkbox_data_;         /** (checkable, enabled, state) */
  QSize default_checkbox_size;
};

