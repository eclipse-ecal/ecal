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

#include <QMap>

#include <CustomQt/QAbstractTreeItem.h>
#include <CustomQt/QAbstractTreeModel.h>

#include <ecal/protobuf/ecal_proto_hlp.h>

class ProtobufTreeModel;

class ProtobufTreeItem :
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

  ProtobufTreeItem(int number, ProtobufTreeModel* model);

  ~ProtobufTreeItem();

  QVariant data(int column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const override;

  QVariant data(Columns column, Qt::ItemDataRole role = Qt::ItemDataRole::DisplayRole) const;

  int type() const override;

  void setMessage(const google::protobuf::Message& message);
  void setValue(const QVariant& raw_value, const QVariant& display_value = QVariant());
  void setField(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field_descriptor);
  void setRepeatedField(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field_descriptor, int index);

private:
  ProtobufTreeModel* model_;

  int number_;

  QVariant raw_value_;
  QVariant display_value_;

  QMap<int, ProtobufTreeItem*> children_;

  // Meta Information
  QString field_name_;
  QString type_;
  QString field_rule_;

  void setFieldMetaInformation(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field_descriptor);
  void setRepeatedFieldMetaInformation(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* field_descriptor, int index);

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
  static QString bytesToHex(const QByteArray& byte_array, char separator = '\0');
#endif //QT_VERSION

};

