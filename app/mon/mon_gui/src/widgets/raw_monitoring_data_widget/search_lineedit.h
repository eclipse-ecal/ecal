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

#include <QLineEdit>

/**
 * @brief A search-bar with some special buttons / actions and find next / previous functionality
 * 
 * The Widget using this class has to connect the following singals:
 *    - newSearchTriggered
 *    - searchNextTriggered
 *    - searchPreviousTriggered
 */
class SearchLineedit : public QLineEdit
{
  Q_OBJECT

/////////////////////////////////////////
// Constructors & Destructor
/////////////////////////////////////////
public:
  SearchLineedit(QWidget *parent = Q_NULLPTR);
  SearchLineedit(const QString &contents, QWidget *parent = Q_NULLPTR);

/////////////////////////////////////////
// Icons
/////////////////////////////////////////
public:
  /**
   * @brief Set the "clear" icon to a custom icon
   * 
   * If not used, the default Qt icon will be used.
   * 
   * @param icon The new icon to set
   */
  void setClearIcon(const QIcon& icon);

private:
  /** @brief Choose icons based on dark / light mode */
  void chooseThemeIcons();

/////////////////////////////////////////
// Signals and slots for search functionality
/////////////////////////////////////////
public slots:
  /**
   * @brief request searching for the next occurrence.
   * 
   * This slot doesn't need to be triggered from the outside, as this widget
   * will do that itself. It may however be helpfull to trigger the search
   * functionality from the outside, e.g. when the user clicked on a menu item
   * or pressed a special key sequence.
   */
  void searchNext();

  /**
  * @brief request searching for the previous occurrence.
  * 
  * This slot doesn't need to be triggered from the outside, as this widget
  * will do that itself. It may however be helpfull to trigger the search
  * functionality from the outside, e.g. when the user clicked on a menu item
  * or pressed a special key sequence.
  */
  void searchPrevious();

signals:
  /**
   * @brief The user requested to find a new string.
   * 
   * This this signal is always emitted, when BOTH is true:
   *   - The user has changed the input in the search field
   *   - The user has requested to find the next / previous occurence
   * 
   * In this case, the searchNextTriggered / searchPreviousTriggered signals
   * are also emitted
   * 
   * It is ALSO triggered, when the input was cleared. In this case, the
   * searchNextTriggered / searchPreviousTriggered are not emitted, as the user
   * didn't actually search for anything.
   * 
   * Another widget doesn't have to use this signal. It is however usefull e.g.
   * to update the SpecialSelections / Result highlighting.
   */
  void newSearchTriggered     (const QString& search_text);

  /** @brief The user wants to find the next occurence. */
  void searchNextTriggered    (const QString& search_text);

  /** @brief The user wants to find the previous occurence. */
  void searchPreviousTriggered(const QString& search_text);

/////////////////////////////////////////
// Qt Events override
/////////////////////////////////////////
protected:
  void keyPressEvent(QKeyEvent* key_event) override;
  void changeEvent(QEvent* event) override;

/////////////////////////////////////////
// Member variables
/////////////////////////////////////////
private:
  QAction* clear_lineedit_action_;                                              /**< Action for clearing the search string */
  QAction* search_next_action;                                                  /**< Action for finding the next occurence */
  QAction* search_previous_action;                                              /**< Action for finding the previous occurence */

  QString last_search_text_;                                                    /**< The last search text. Used to determine if a new search was triggered. */
};
