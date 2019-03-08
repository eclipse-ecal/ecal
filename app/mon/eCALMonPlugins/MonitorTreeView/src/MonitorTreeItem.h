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

#include <CustomQt/QAbstractTreeItem.h>
#include <CustomQt/QAbstractTreeModel.h>

class MonitorTreeModel;

struct StringEnum
{
  int value;
  QString name;
};

Q_DECLARE_METATYPE(StringEnum)

class MonitorTreeItem :
  public QAbstractTreeItem
{
public:

  enum class Columns : int
  {
    FIELD_NAME,
    FIELD_RULE,
    NUMBER,
    TYPE,
    VALUE,
  };

  MonitorTreeItem(int number, MonitorTreeModel* model);

  ~MonitorTreeItem();

  struct MonitorTreeMetaData
  {
    // Meta Information
    QString field_name_;
    QString type_;
    QString field_rule_;
  };

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const override;
  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  int type() const override;

  void setValue(const QVariant& data);
  void clearValue();

  // Call this when before accessing child elements
  void              markChildrenUnused();

  // call this function to modify elements
  MonitorTreeItem*  getChild(int index);

  // call this after setting everything
  void              deleteChildrenUnused();


  void UpdateModel();

  void setMetaData(const MonitorTreeMetaData& data_);

  // Getter for the visitor pattern to check whether this child has been processed
  inline bool isAccessed() const;
  // Setter for the visitor pattern to save whether this child has been processed
  inline void setAccessed(bool accessed);

private:
  QVariant getDisplayValue() const;
  QLocale locale;

  MonitorTreeModel* model_;

  int number_;

  QVariant data_;

  MonitorTreeMetaData meta_data_;

  bool accessed_; // Member variable used for the Visitor pattern

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
  static QString bytesToHex(const QByteArray& byte_array, char separator = '\0');
#endif //QT_VERSION

};
