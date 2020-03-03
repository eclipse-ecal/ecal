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
#include <QWidget>

#include <ecal/ecal_time.h>

namespace eCAL
{
  namespace mon
  {
    class PluginWidgetInterface
    {
    public:
      virtual ~PluginWidgetInterface() {};

      virtual QWidget* getWidget() = 0;

    public slots:
      virtual void onUpdate() = 0;

      virtual void onResume() = 0;
      virtual void onPause() = 0;
    };
  }
}