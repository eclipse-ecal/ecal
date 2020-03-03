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

#include "tree_item_view_widget.h"

TreeItemViewWidget::TreeItemViewWidget(const QVector<QPair<int, QString>> & column_name_map, QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);

  item_view_model_ = new ItemViewModel(nullptr, column_name_map, this);
  ui_.treeView->setModel(item_view_model_);
  ui_.treeView->resizeColumnToContents(0);
}

TreeItemViewWidget::~TreeItemViewWidget()
{
}

void TreeItemViewWidget::setTreeItem(QAbstractTreeItem* tree_item)
{
  item_view_model_->setTreeItem(tree_item);
}

QAbstractTreeItem* TreeItemViewWidget::treeItem()
{
  return item_view_model_->treeItem();
}

void TreeItemViewWidget::update()
{
  item_view_model_->update();
}