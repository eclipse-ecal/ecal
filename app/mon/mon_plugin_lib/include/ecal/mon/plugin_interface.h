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

#include <ecal/mon/plugin_widget_interface.h>

#include <QString>
#include <QWidget>

namespace eCAL
{
  namespace mon
  {
    class PluginInterface
    {
    public:
      virtual PluginWidgetInterface* create(const QString& topic_name, const QString& topic_type, QWidget* parent = Q_NULLPTR) = 0;
    };
  }
}

#define PluginInterface_iid "de.conti.ecal.mon.plugin.interface"

Q_DECLARE_INTERFACE(eCAL::mon::PluginInterface, PluginInterface_iid)