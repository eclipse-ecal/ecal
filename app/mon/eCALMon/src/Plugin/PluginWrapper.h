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

#include <memory>

#include <ecal/mon/plugin.h>

class PluginWrapper
{
public:
  struct PluginData
  {
    struct MetaData
    {
      QString name;
      QString version;
      QString author;
      int priority;

      struct Topic
      {
        QString name;
        QString type;
        QString format;
      };

      QVector<Topic> topics;
      bool operator<(const MetaData& rhs) const;
    };

    QString iid;
    QString path;
    MetaData meta_data;

    bool operator<(const PluginData& rhs) const;
  };

  PluginWrapper() = default;
  PluginWrapper(const QString& path);

  bool operator<(const PluginWrapper& rhs) const;

  /* Returns the plugin data, that is associated with this plugin */
  const PluginData& getPluginData() const;

  /* Checks if the Plugin has been loaded (the underlying dll)*/
  bool isLoaded() const;
  /* Load the plugin into memory */
  bool load();
  /* Unload the plugin from memory*/
  bool unload();
  
  /* Create an instance of a PluginWidgetInterface for this plugin */
  eCAL::mon::PluginWidgetInterface* create(const QString& topic_name, const QString& topic_type, QWidget* parent);

private:
  PluginData plugin_data_;

  std::shared_ptr<eCAL::mon::PluginInterface> instance_;
};

/* This function checks if a given topic name and topic type are accepted by the Plugin given in wrapper */
bool AcceptsTopic(const PluginWrapper& wrapper, QString topic_name, QString topic_type);