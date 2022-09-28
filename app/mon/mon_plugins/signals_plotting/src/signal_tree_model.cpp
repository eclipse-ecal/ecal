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

#include "signal_tree_model.h"

SignalTreeModel::SignalTreeModel(QObject* parent)
  : QAbstractTreeModel(parent)
{
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  SignalTreeModel::setRoot(new SignalTreeItem(-1, this));
}

SignalTreeModel::~SignalTreeModel()
{}

int SignalTreeModel::columnCount(const QModelIndex&/*parent*/) const
{
  return (int)Columns::COLUMN_COUNT;
}

QVariant SignalTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
  {
    return column_labels_.at((Columns)section);
  }
  return QAbstractTreeModel::headerData(section, orientation, role);
}

int SignalTreeModel::mapColumnToItem(int model_column, int tree_item_type) const
{
  switch (tree_item_type)
  {
  case kMonitoringContent:
    return item_column_mapping_.at((Columns)(model_column));
  default:
    return QAbstractTreeModel::mapColumnToItem(model_column, tree_item_type);
  }
}

void SignalTreeModel::setRoot(QAbstractTreeItem* root)
{
  // This function just checks if the user tries to set an inappropriate root item.
  Q_ASSERT(root->type() == kMonitoringContent);
  QAbstractTreeModel::setRoot(root);
}

Qt::ItemFlags SignalTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::ItemFlag();
  int column = index.column();
  SignalTreeItem* tree_item = static_cast<SignalTreeItem*>(item(index));
  return tree_item->flags(column);
}

QVariant SignalTreeModel::data(const QModelIndex& index, int role) const
{
  QAbstractTreeItem* tree_item = item(index);
  if (tree_item)
    return (index.column() >= 0 ? tree_item->data(index.column(), (Qt::ItemDataRole)role) : QVariant());
  else
    return QVariant();
}

void SignalTreeModel::setCheckedState(SignalTreeItem* item, int index_column)
{
  item->setItemCheckedState((item->getItemCheckedState(index_column) == Qt::CheckState::Checked ? Qt::CheckState::Unchecked : Qt::CheckState::Checked), index_column);
}

QString SignalTreeModel::getFullSignalName(QAbstractTreeItem* item) const
{
  QString full_signal_name = item->data((int)(SignalTreeItem::Columns::FIELD_NAME)).toString();
  auto iter_item = item;
  while (iter_item != nullptr)
  {
    auto parent = iter_item->parentItem();
    if (parent->data((int)(SignalTreeItem::Columns::FIELD_NAME)).toString().isEmpty())
      break;
    full_signal_name = full_signal_name + "." + parent->data((int)(SignalTreeItem::Columns::FIELD_NAME)).toString();
    iter_item = iter_item->parentItem();
  }
  return full_signal_name;
}

QList<QString>& SignalTreeModel::getListOfCheckedItems(int model_column)
{
  if (model_column == (int)Columns::MULTI)
  {
    return checked_multi_items_;
  }
  return checked_single_items_;
}

int SignalTreeModel::getCountCheckedItems(int model_column, const QString& tab_name)
{
  if (model_column == (int)Columns::MULTI)
  {
    return checked_multi_items_.count();
  }
  else
  {
    if (tab_name.isEmpty())
    {
      return checked_single_items_.count();
    }
    return checked_single_items_.count(tab_name);
  }
}

void SignalTreeModel::appendItemChecked(const QString& full_signal_name, int model_column)
{
  // add to checked items and remove from previously checked if it is the case
  if (model_column == (int)Columns::MULTI)
  {
    if (!checked_multi_items_.contains(full_signal_name))
      checked_multi_items_.append(full_signal_name);

    if (previously_checked_items_.contains({ (int)SignalTreeItem::Columns::MULTI ,full_signal_name }))
    {
      previously_checked_items_.removeAll({ (int)SignalTreeItem::Columns::MULTI ,full_signal_name });
    }
  }
  else
  {
    if (!checked_single_items_.contains(full_signal_name))
      checked_single_items_.append(full_signal_name);
    if (previously_checked_items_.contains({ (int)SignalTreeItem::Columns::SINGLE ,full_signal_name }))
    {
      previously_checked_items_.removeAll({ (int)SignalTreeItem::Columns::SINGLE ,full_signal_name });
    }
  }
}

void SignalTreeModel::removeItemChecked(const QString& full_signal_name, int model_column)
{
  if (model_column == (int)Columns::MULTI)
  {
    checked_multi_items_.removeAll(full_signal_name);
    previously_checked_items_.append(QPair<int, QString>((int)SignalTreeItem::Columns::MULTI, full_signal_name));
  }
  else
  {
    checked_single_items_.removeAll(full_signal_name);
    previously_checked_items_.append(QPair<int, QString>((int)SignalTreeItem::Columns::SINGLE, full_signal_name));
  }
}

void SignalTreeModel::slt_itemValueChanged(SignalTreeItem* item)
{
  QString item_value;
  if (item->getValue().userType() == QMetaType::type("StringEnum"))
  {
    StringEnum string_enum = item->getValue().value<StringEnum>();
    item_value = QString::number(string_enum.value);
  }
  else
  {
    item_value = item->getValue().toString();
  }

  QString full_signal_name = getFullSignalName(item);

  // if an item is checked in the tree or was previously checked, send value
  // if it was checked but not curently on the tree, restore state
  if (item->getItemCheckedState((int)SignalTreeItem::Columns::MULTI) == Qt::Checked ||
    previously_checked_items_.contains(QPair<int, QString>((int)SignalTreeItem::Columns::MULTI, full_signal_name)))
  {
    emit plottedSignalValueChanged(item_value, full_signal_name, SignalTreeItem::Columns::MULTI);
  }
  else
  {
    if (checked_multi_items_.contains(full_signal_name))
    {
      emit restoreState(this->index(item->row(), (int)Columns::MULTI, this->index(item->parentItem())));
    }
  }
  if (item->getItemCheckedState((int)SignalTreeItem::Columns::SINGLE) == Qt::Checked ||
    previously_checked_items_.contains(QPair<int, QString>((int)SignalTreeItem::Columns::SINGLE, full_signal_name)))
  {
    emit plottedSignalValueChanged(item_value, full_signal_name, SignalTreeItem::Columns::SINGLE);
  }
  else
  {
    if (checked_single_items_.contains(full_signal_name))
    {
      emit restoreState(this->index(item->row(), (int)Columns::SINGLE, this->index(item->parentItem())));
    }
  }
}
