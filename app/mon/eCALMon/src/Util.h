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

#include <QString>
#include <QVariant>
#include <QUuid>
#include <QUrl>

namespace QtUtil
{
  inline QString variantToString(const QVariant& variant)
  {
    switch ((QMetaType::Type)variant.type())
    {
    case QMetaType::Bool:
      return variant.toBool() ? "True" : "False";
    case QMetaType::QByteArray:
      return variant.toByteArray().toHex();
    case QMetaType::Char:
      return variant.toChar();
    case QMetaType::Double:
      return QString::number(variant.toDouble());
    case QMetaType::Float:
      return QString::number(variant.toFloat());
    case QMetaType::QUuid:
      return variant.toUuid().toString();
    case QMetaType::Int:
      return QString::number(variant.toInt());
    case QMetaType::LongLong:
      return QString::number (variant.toLongLong());
    case QMetaType::QString:
      return variant.toString();
    case QMetaType::UInt:
      return QString::number(variant.toUInt());
    case QMetaType::ULongLong:
      return QString::number(variant.toULongLong());
    case QMetaType::QUrl:
      return variant.toUrl().toString();
    default:
      return "";
    }
  }
}