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

#include "CustomQt/QAbstractTreeModel.h"
#include "CustomQt/QAbstractTreeItem.h"

#include <QObject>
#include <QModelIndex>

#include <map>

#include "signal_tree_item.h"

class SignalTreeModel : public QAbstractTreeModel
{
  Q_OBJECT

public:

  enum class Columns : int
  {
    FIELD_NAME,
    NUMBER,
    FIELD_RULE,
    TYPE_NAME,
    VALUE,
    MULTI,
    SINGLE,

    COLUMN_COUNT
  };

  SignalTreeModel(QObject* parent = nullptr);
  ~SignalTreeModel();

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  int mapColumnToItem(int model_column, int tree_item_type) const override;

  void setRoot(QAbstractTreeItem* root) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  QVariant data(const QModelIndex& index, int role) const override;

  void setCheckedState(SignalTreeItem* item, int index_column);

  QString getFullSignalName(QAbstractTreeItem* item) const;

  QList<QString>& getListOfCheckedItems(int model_column);

  int getCountCheckedItems(int model_column, const QString& tab_name = QString());

public slots:
  void slt_itemValueChanged(SignalTreeItem* item);

  void appendItemChecked(const QString& full_signal_name, int model_column);
  void removeItemChecked(const QString& full_signal_name, int model_column);

signals:
  void plottedSignalValueChanged(QString& value, QString& full_signal_name, SignalTreeItem::Columns column) const;
  void restoreState(const QModelIndex& index);

private:
  std::map<Columns, QString> column_labels_ =
  {
    { Columns::FIELD_NAME, "Field" },
    { Columns::NUMBER,     "Number" },
    { Columns::FIELD_RULE, "Rule" },
    { Columns::TYPE_NAME,  "Type" },
    { Columns::VALUE,      "Value" },
    { Columns::MULTI,      "Multi" },
    { Columns::SINGLE,     "Single" },
  };

  std::map<Columns, int> item_column_mapping_ =
  {
    { Columns::FIELD_NAME, (int)SignalTreeItem::Columns::FIELD_NAME },
    { Columns::NUMBER,     (int)SignalTreeItem::Columns::NUMBER },
    { Columns::FIELD_RULE, (int)SignalTreeItem::Columns::FIELD_RULE },
    { Columns::TYPE_NAME,  (int)SignalTreeItem::Columns::TYPE},
    { Columns::VALUE,      (int)SignalTreeItem::Columns::VALUE },
    { Columns::MULTI,      (int)SignalTreeItem::Columns::MULTI },
    { Columns::SINGLE,     (int)SignalTreeItem::Columns::SINGLE },
  };

  QList<QString> checked_multi_items_;
  QList<QString> checked_single_items_;
  QList<QPair<int, QString>> previously_checked_items_;
};