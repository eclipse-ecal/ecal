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

#include <QCompleter>
#include <CustomQt/QCustomFileSystemModel.h>

/**
 * @brief The QPathLineEdit is a specialized QLineEdit with a completer.
 *
 * This LineEdit will automatically complete a path entered into it. By default,
 * directories and files of all type are displayed in the completer-popup. This
 * can be changed by the @see{setFilter} function.
 *
 * The QPathLineEdit will use the backslash '\' as separator on Windows and the
 * forward slash '/' as separator on all other operating systems.
 *
 * The user *must not* set any custom completer to this LineEdit.
 */
class QPathLineEdit : public QLineEdit
{
  Q_OBJECT

public:
  QPathLineEdit(QWidget *parent = Q_NULLPTR);
  QPathLineEdit(const QString &contents, QWidget *parent = Q_NULLPTR);

  ~QPathLineEdit();

  /**
   * @brief Sets the filter that defines which file system entries are shown in the popup
   *
   * By default, all directories and files are shown.
   *
   * @param filters The new Filter
   */
  void setFilter(QDir::Filters filters);

  /**
   * @brief Returns the filter that defines which file system entries are shown in the popup
   *
   * By default, all directories and files are shown.
   *
   * @return The current filter
   */
  QDir::Filters filter() const;

private:
  QCompleter*              completer_;                                          /**< The completer that autocompletes what the user types and shows the popup */
  QCustomFileSystemModel*  completer_model_;                                    /**< The model for the completer that queries the file-system and filters the results */
};
