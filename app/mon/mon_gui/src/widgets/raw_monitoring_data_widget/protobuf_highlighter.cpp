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

#include "protobuf_highlighter.h"

ProtobufHighlighter::ProtobufHighlighter(QTextDocument* parent)
    : ProtobufHighlighter(false, parent)
{}

ProtobufHighlighter::ProtobufHighlighter(bool darkmode_optimized, QTextDocument* parent)
  : QSyntaxHighlighter(parent)
{
  // Curly braces
  {
    HighlightingRule rule;
    rule.pattern_ = QRegularExpression ("\\{|\\}");
    rule.format_.setForeground(darkmode_optimized ? QColor(86, 156, 214) : Qt::darkBlue);
    rule.format_.setFontWeight(QFont::Bold);
    highlighting_rules_.push_back(std::move(rule));
  }

  // HighlightingRule that highlights everything after the first colon in a line
  // This is used to highlight enums (or basically everything that is neither a string nor a number, as those two will be overridden by the next rules)
  {
    HighlightingRule rule;
    rule.pattern_ = QRegularExpression ("\\:.*");
    rule.format_.setForeground(darkmode_optimized ? QColor(218, 99, 161) : Qt::darkMagenta);
    rule.format_.setFontWeight(QFont::Bold);
    highlighting_rules_.push_back(std::move(rule));
  }

  // Format the colons themselves in bold dark blue, just like the curly braces
  {
    HighlightingRule rule;
    rule.pattern_ = QRegularExpression ("\\:");
    rule.format_.setForeground(darkmode_optimized ? QColor(86, 156, 214) : Qt::darkBlue);
    rule.format_.setFontWeight(QFont::Bold);
    highlighting_rules_.push_back(std::move(rule));
  }

  // Numbers
  // regular expression that matches numbers with optional sign, decimal point and exponent
  {
    HighlightingRule rule;
    rule.pattern_ = QRegularExpression("([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)");
    rule.format_.setForeground(darkmode_optimized ? QColor(206, 145, 120) : Qt::darkRed);
    rule.format_.setFontWeight(QFont::Bold);
    highlighting_rules_.push_back(rule);
  }

  // Strings
  // Strings are started and ended by double quotes.
  // Double quotes inside strings are escaped by a backslash.
  // The backslash itself can be escaped by another backslash.
  // Binary characters can be escaped by a backslash followed by a number.
  // Special characters like line endings can be escabed by a backslash followed by a letter.
  //    --> The backslash just escapes everything that follows.
  {
    HighlightingRule rule;
    rule.pattern_ = QRegularExpression("\"(?:[^\"\\\\]|\\\\.)*\"");
    rule.format_.setForeground(darkmode_optimized ? QColor(87, 166, 74) : Qt::darkGreen);
    rule.format_.setFontWeight(QFont::Bold);
    highlighting_rules_.push_back(std::move(rule));
  }
}

void ProtobufHighlighter::highlightBlock(const QString &text)
{
  for (const HighlightingRule& rule : highlighting_rules_)
  {
    QRegularExpressionMatchIterator matchIterator = rule.pattern_.globalMatch(text);
    while (matchIterator.hasNext())
    {
      const QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format_);
    }
  }
}
