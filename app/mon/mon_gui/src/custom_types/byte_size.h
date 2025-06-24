/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <QtGlobal>
#include <QString>
#include <QMetaType>
#include <cstdint>
#include <QStyledItemDelegate>

class ByteSize
{
public:
    explicit ByteSize(uint64_t bytes = 0) : m_bytes(bytes) {}

    QString toString(int precision = 2) const;

    bool operator<(const ByteSize &other) const;


private:
    uint64_t m_bytes;
};

Q_DECLARE_METATYPE(ByteSize)

class ByteSizeDelegate : public QStyledItemDelegate {
public:
  using QStyledItemDelegate::QStyledItemDelegate;
  QString displayText(const QVariant& v, const QLocale& /*loc*/) const override {
    if (v.canConvert<ByteSize>())
      return v.value<ByteSize>().toString();
    return QStyledItemDelegate::displayText(v, {});
  }
};