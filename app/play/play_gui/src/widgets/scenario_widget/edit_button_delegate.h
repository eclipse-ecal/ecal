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

#include <QStyledItemDelegate>

#include <QStandardItemModel>

#include <functional>

class EditButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

//////////////////////////////////////////
// Constructor & Destructor
//////////////////////////////////////////
public:
  EditButtonDelegate(QAbstractItemView* parent);
  ~EditButtonDelegate();

//////////////////////////////////////////
// QStyledItemDelegate overrides
//////////////////////////////////////////
public:
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

protected:
  bool eventFilter(QObject* obj, QEvent *event) override;

//////////////////////////////////////////
// API of EditButtonDelegate
//////////////////////////////////////////
signals:
  void buttonStateChanged(const QModelIndex& index) const;
  void buttonClicked(const QModelIndex& index) const;

private:
  QRect getButtonRect(const QStyleOptionViewItem &option) const;

//////////////////////////////////////////
// Member Variables
//////////////////////////////////////////
private:
  QRect button_rect_;
  bool mouse_down_;

  QModelIndex last_time_hovered_index_;
  bool        last_time_button_hover_;

  QModelIndex index_currently_editing_;
};
