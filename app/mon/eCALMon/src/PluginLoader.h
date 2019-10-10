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

#include <QMap>

class PluginLoader;

class PluginWrapper
{
public:
  PluginWrapper() = default;

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
  };

  MetaData getMetaData() const;
  QString getPath() const;
  QString getIID() const;

  bool load();
  bool isLoaded() const;
  bool unload();

  PluginWrapper(const QString& path);
  
  eCAL::mon::PluginWidgetInterface* create(const QString& topic_name, const QString& topic_type, QWidget* parent);

private:
  QString iid_;
  QString path_;
  MetaData meta_data_;

  std::shared_ptr<eCAL::mon::PluginInterface> instance_;
};

class PluginLoader
{
public:
  static PluginLoader* getInstance();
  void discover();

  QList<QString> getAvailableIIDs() const;
  PluginWrapper& getPluginByIID(const QString& iid);

  QList<PluginWrapper> getSupportedPlugins(const QString& topic_name, const QString& topic_type) const;

private:
  PluginLoader();

  static std::unique_ptr<PluginLoader> instance_;
  QMap<QString, PluginWrapper> plugins_;
};
