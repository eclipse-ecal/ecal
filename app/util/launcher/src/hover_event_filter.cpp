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

/**
 * @brief eCALLauncher Application
**/

#include "hovereventfilter.h"

HoverEventFilter::HoverEventFilter(QObject *parent) : QObject(parent)
{
}

bool HoverEventFilter::eventFilter(QObject *watched, QEvent *event)
{
  QEvent::Type t = event->type();

  switch (t)
  {
  case QEvent::Enter:
    emit HoverIn(watched);
    break;
  case QEvent::Leave:
    emit HoverOut(watched);
    break;
  default:
    return false;
  }
  return true;
}
