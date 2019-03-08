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

#include <QLineEdit>

/**
 * @brief The QFilterLineEdit class is a LineEdit with a default configuration as expected from a filter input
 *
 * This LineEdit distinguishes itself from a QLineEdit by the following additions:
 *   - A Clear button is displayed a the end
 *   - Pressing ESC removes all Text from the LineEdit
 */
class QFilterLineEdit : public QLineEdit
{
public:
  QFilterLineEdit(QWidget *parent = Q_NULLPTR);
  QFilterLineEdit(const QString &contents, QWidget *parent = Q_NULLPTR);

  ~QFilterLineEdit();

  /**
   * @brief Sets the icon of the button that clears the LineEdit
   * @param icon the new icon
   */
  void setClearIcon(const QIcon& icon);

protected:
  void keyPressEvent(QKeyEvent* key_event);

private:
  QAction* clear_lineedit_action_;                                              /**< The action that displays the button at the end of the LineEdit */
};
