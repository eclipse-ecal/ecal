/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2023 Continental Corporation
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

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

#include <vector>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

/**
 * @brief A QSyntaxHighlighter that can highlight the DebugString Protobuf Syntax
 */
class ProtobufHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT

////////////////////////////////////////////
// Constructor & Destructor
////////////////////////////////////////////
public:
  ProtobufHighlighter(QTextDocument* parent = nullptr);
  ProtobufHighlighter(bool darkmode_optimized = false, QTextDocument *parent = nullptr);

////////////////////////////////////////////
// QSyntaxHighlighter overrides
////////////////////////////////////////////
protected:
  void highlightBlock(const QString &text) override;

////////////////////////////////////////////
// Member Variables
////////////////////////////////////////////
private:
  struct HighlightingRule
  {
    QRegularExpression pattern_;
    QTextCharFormat    format_;
  };

  std::vector<HighlightingRule> highlighting_rules_;                            //!< A list of highlighting rules. They are evaluated one after another and may ovewrite each other.
};
