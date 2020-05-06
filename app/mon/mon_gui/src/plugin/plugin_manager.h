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

#include <QMap>

#include <plugin/plugin_wrapper.h>

class PluginManager
{
public:
  struct ActivePlugin
  {
    PluginWrapper wrapper;
    bool active;

    bool operator<(const ActivePlugin& rhs) const
    {
      return wrapper < rhs.wrapper;
    }
  };

  static PluginManager* getInstance();
  void discover();

  QList<QString> getAvailableIIDs() const;
  PluginWrapper::PluginData getPluginData(const QString& iid) const;
  bool isActive(const QString& iid) const;
  void setActive(const QString& iid, bool loaded);

  QList<QPair<PluginWrapper::PluginData, eCAL::mon::PluginWidgetInterface*>> CreatePlugins(QString topic_name, QString topic_type, QWidget* parent);

private:
  PluginManager();

  void AddFoundPlugins(const QVector<QString>& library_paths);
  void ActivateDistinctPlugins();

  static std::unique_ptr<PluginManager> instance_;
  QMap<QString, ActivePlugin> plugins_;
};