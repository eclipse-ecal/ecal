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

#include <QCheckBox>

/**
 * @brief The QAdvancedCheckBox is quite similar to the QCheckBox, but provides the option to change the order of states in a tristate case (@see{setStateAfterPartiallyChecked()})
 */
class QAdvancedCheckBox : public QCheckBox
{
  Q_OBJECT

public:
  QAdvancedCheckBox(QWidget * parent = Q_NULLPTR)
    : QCheckBox(parent)
    , state_after_partially(Qt::CheckState::Checked)
  {}

  QAdvancedCheckBox(const QString& text, QWidget * parent = Q_NULLPTR)
    : QCheckBox(text, parent)
    , state_after_partially(Qt::CheckState::Checked)
  {}

  /**
   * @brief Sets the state after Qt::CheckState::PartiallChecked.
   *
   * The following state is only relevant, if the CheckBox operates in tristate
   * mode.
   *
   * If next_state is Checked, the order will be:
   *    PartiallyChecked -> Checked -> Unchecked
   *
   * If next_state is Unchecked, the order wil be:
   *    PartiallyChecked -> Unchecked -> Checked
   *
   * Note that this setting also influenced the following state of the Checked
   * and Unchecked states!
   *
   * @param next_state The state that shall follow the PartiallyChecked state
   */
  void setStateAfterPartiallyChecked(Qt::CheckState next_state)
  {
    state_after_partially = next_state;
  }

  /**
   * @brief Gets the state that follows the Qt::CheckState::PartiallChecked state
   *
   * The setting state is only relevant, if the CheckBox operates in tristate
   * mode. Also see @see{setStateAfterPartiallyChecked()}
   *
   * @return The state that follows the Qt::CheckState::PartiallChecked state
   */
  Qt::CheckState stateAfterPartiallyChecked()
  {
    return state_after_partially;
  }

protected:
  virtual void nextCheckState()
  {
    if (isTristate())
    {
      if (checkState() == Qt::CheckState::PartiallyChecked)
      {
        setCheckState(state_after_partially);
      }
      else if (checkState() == Qt::CheckState::Checked)
      {
        setCheckState(state_after_partially == Qt::CheckState::Unchecked ? Qt::CheckState::PartiallyChecked : Qt::CheckState::Unchecked);
      }
      else if (checkState() == Qt::CheckState::Unchecked)
      {
        setCheckState(state_after_partially == Qt::CheckState::Checked ? Qt::CheckState::PartiallyChecked : Qt::CheckState::Checked);
      }
    }
    else
    {
      setChecked(!isChecked());
    }

    emit stateChanged(checkState());
  }

private:
  Qt::CheckState state_after_partially;
};
