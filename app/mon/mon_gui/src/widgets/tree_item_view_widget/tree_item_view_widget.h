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
#include "ui_tree_item_view_widget.h"

#include "widgets/models/item_view_model.h"

class TreeItemViewWidget : public QWidget
{
  Q_OBJECT

public:
  TreeItemViewWidget(const QVector<QPair<int, QString>> & column_name_map, QWidget *parent = Q_NULLPTR);
  ~TreeItemViewWidget();

  void setTreeItem(QAbstractTreeItem* tree_item);
  QAbstractTreeItem* treeItem();

public slots:
  void update();

private:
  Ui::TreeItemViewWidget ui_;

  ItemViewModel* item_view_model_;
};
