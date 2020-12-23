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

#include <QStyledItemDelegate>

class SeverityPickerDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
  SeverityPickerDelegate(QObject *parent = 0);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private slots:
  void saveData();

private:
  QString tooltip =
    "<table>\n"
    "<tbody>\n"
    "<tr>\n"
    "<td style=\"text-align: center;\">&nbsp;Healthy&nbsp;Lv1&nbsp;</td>\n"
    "<td style=\"text-align: center;\">&nbsp;\342\211\253&nbsp;</td>\n"
    "<td style=\"text-align: center;\">&nbsp;Healthy&nbsp;Lv2&nbsp;</td>\n"
    "<td style=\"text-align: center;\">&nbsp;\342\211\253&nbsp;</td>\n"
    "<td style=\"text-align: center;\">&nbsp;...&nbsp;</td>\n"
    "<td style=\"text-align: center;\">&nbsp;\342\211\253&nbsp;</td>\n"
    "<td style=\"text-align: center;\">&nbsp;Failed&nbsp;Lv4&nbsp;</td>\n"
    "<td style=\"text-align: center;\">&nbsp;\342\211\253&nbsp;</td>\n"
    "<td style=\"text-align: center;\">&nbsp;Failed&nbsp;Lv5&nbsp;</td>\n"
    "</tr>\n"
    "<tr>\n"
    "<td style=\"text-align: center;\"><strong>Best</strong></td>\n"
    "<td style=\"text-align: center;\">\342\211\253</td>\n"
    "<td style=\"text-align: center;\">...</td>\n"
    "<td style=\"text-align: center;\">\342\211\253</td>\n"
    "<td style=\"text-align: center;\">...</td>\n"
    "<td style=\"text-align: center;\">\342\211\253</td>\n"
    "<td style=\"text-align: cente"
    "r;\">...</td>\n"
    "<td style=\"text-align: center;\">\342\211\253</td>\n"
    "<td style=\"text-align: center;\"><strong>Worst</strong></td>\n"
    "</tr>\n"
    "</tbody>\n"
    "</table>";
};