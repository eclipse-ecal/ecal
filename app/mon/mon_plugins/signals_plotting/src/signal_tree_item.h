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

#include <QLocale>
#include <QMap>
#include <QSet>
#include <QObject>
#include <QColor>
#include <QSet>
#include <QByteArray>

#include <CustomQt/QAbstractTreeItem.h>
#include <CustomQt/QAbstractTreeModel.h>

#include "util.h"

constexpr int kMonitoringContent = 6;

class SignalTreeModel;

struct StringEnum
{
  int value;
  QString name;
};

Q_DECLARE_METATYPE(StringEnum)

class SignalTreeItem :
  public QObject, public QAbstractTreeItem
{
  Q_OBJECT

public:

  enum class Columns : int
  {
    FIELD_NAME,
    FIELD_RULE,
    NUMBER,
    TYPE,
    VALUE,
    MULTI,
    SINGLE
  };

  SignalTreeItem(int number, SignalTreeModel* model);

  ~SignalTreeItem() override;

  struct SignalTreeMetaData
  {
    // Meta Information
    QString field_name_;
    QString type_;
    QString field_rule_;
  };

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const override;
  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;
  Qt::ItemFlags flags(int column) const override;
  Qt::CheckState getItemCheckedState(int model_column) const;
  void setItemCheckedState(Qt::CheckState state, int model_column);

  void setItemBackgroundColor(const QColor& color);

  int type() const override;

  void setValue(const QVariant& data);
  QVariant& getValue();
  void clearValue();

  // Call this when before accessing child elements
  void              markChildrenUnused();

  // call this function to modify elements
  SignalTreeItem* getChild(int index);

  // call this after setting everything
  void              deleteChildrenUnused();


  void UpdateModel();

  void setMetaData(const SignalTreeMetaData& data_);

  // Getter for the visitor pattern to check whether this child has been processed
  bool isAccessed() const;
  // Setter for the visitor pattern to save whether this child has been processed
  void setAccessed(bool accessed);

signals:

  void sgn_itemValueChanged(SignalTreeItem* item);

private:
  QVariant getDisplayValue() const;
  QLocale locale;

  SignalTreeModel* model_;

  int number_;

  Qt::CheckState multi_checked_;
  Qt::CheckState single_checked_;

  QVariant data_;

  SignalTreeMetaData meta_data_;

  bool accessed_; // Member variable used for the Visitor pattern

  SignalPlotting::Color background_color_;

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
  static QString bytesToHex(const QByteArray& byte_array, char separator = '\0');
#endif //QT_VERSION

};
