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

#include <QStringList>

#include "CustomQt/QAbstractTreeItem.h"

QAbstractTreeItem::QAbstractTreeItem()
  :parent_item_(nullptr)
{}

QAbstractTreeItem::~QAbstractTreeItem()
{
  qDeleteAll(child_items_);
}

void QAbstractTreeItem::insertChild(QAbstractTreeItem* child, int row)
{
  if (row < 0)
  {
    child_items_.push_back(child);
  }
  else
  {
    child_items_.insert(child_items_.begin() + row, child);
  }
  child->parent_item_ = this;
}

void QAbstractTreeItem::removeChild(int row)
{
  QAbstractTreeItem* item_to_remove = child_items_.at(row);
  child_items_.removeAt(row);
  delete item_to_remove;
}

void QAbstractTreeItem::removeAllChildren()
{
  qDeleteAll(child_items_);
  child_items_.clear();
}

QAbstractTreeItem* QAbstractTreeItem::detachChild(int row)
{
  QAbstractTreeItem* item_to_remove = child_items_.at(row);
  item_to_remove->parent_item_ = nullptr;
  child_items_.removeAt(row);
  return item_to_remove;
}

void QAbstractTreeItem::detachFromParent()
{
  Q_ASSERT(parent_item_ != nullptr);
  parent_item_->detachChild(this->row());
}

void QAbstractTreeItem::moveChild(int from, int to)
{
  child_items_.move(from, to);
}

QList<QAbstractTreeItem*> QAbstractTreeItem::findChildren(std::function<bool(QAbstractTreeItem*)> p)
{
  QList<QAbstractTreeItem*> child_list;
  for (QAbstractTreeItem* child_item : child_items_)
  {
    if (p(child_item))
    {
      child_list.push_back(child_item);
    }
    child_list.append(child_item->findChildren(p));
  }
  return child_list;
}

QAbstractTreeItem *QAbstractTreeItem::child(int row) const
{
  return child_items_.value(row);
}

int QAbstractTreeItem::childCount() const
{
  return child_items_.count();
}

QVariant QAbstractTreeItem::data(int /*column*/, Qt::ItemDataRole /*role*/) const
{
  return QVariant();
}

bool QAbstractTreeItem::setData(int /*column*/, const QVariant& /*data*/, Qt::ItemDataRole /*role*/)
{
  return false;
}

Qt::ItemFlags QAbstractTreeItem::flags(int /*column*/) const
{
  return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}

QAbstractTreeItem *QAbstractTreeItem::parentItem() const
{
  return parent_item_;
}

int QAbstractTreeItem::row() const
{
  if (parent_item_)
    return parent_item_->child_items_.indexOf(const_cast<QAbstractTreeItem*>(this));

  return 0;
}

int QAbstractTreeItem::type() const
{
  return -1;
}
