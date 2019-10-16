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

#include "PluginLoader.h"

#include <ecal/ecal_log.h>

#include <QDir>
#include <QApplication>
#include <QLibrary>
#include <QPluginLoader>
#include <QJsonArray>
#include <QDebug>
#include <QProcessEnvironment>

using namespace eCAL::mon;

PluginWrapper::MetaData PluginWrapper::getMetaData() const
{
  return meta_data_;
}

QString PluginWrapper::getPath() const
{
  return path_;
}

QString PluginWrapper::getIID() const
{
  return iid_;
}

bool PluginWrapper::load()
{
  if (!isLoaded())
  {
    QPluginLoader loader(path_);
    auto instance = loader.instance();
    if (instance)
    {
      eCAL::mon::PluginInterface* plugin = qobject_cast<eCAL::mon::PluginInterface*>(instance);
      if (plugin)
      {
        eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_info, "Plugin " + meta_data_.name.toStdString() + " " + meta_data_.version.toStdString() + " has been loaded.");
        instance_ = std::shared_ptr<eCAL::mon::PluginInterface>(plugin, 
          [this](eCAL::mon::PluginInterface* p)
        {
          eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_info, "Plugin " + meta_data_.name.toStdString() + " " + meta_data_.version.toStdString() + " has been unloaded.");
          delete dynamic_cast<QObject*>(p);
        });
        return true;
      }
      else
      {
        eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_warning, "Plugin " + path_.toStdString() + " is not suitable for eCAL Monitor");
        loader.unload();
      }
    }
    else
      eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_warning, "Unable to load plugin " + path_.toStdString() + ".");
  }
  else
    return true;

  return false;
}

bool PluginWrapper::isLoaded() const
{
  if (instance_.get() == nullptr)
    return false;
  return true;
}

bool PluginWrapper::unload()
{
  if (instance_.use_count() <= 1)
  {
    instance_.reset();
    return true;
  }

  return false;
}

static PluginWrapper::MetaData jsonToMetaStruct(const QJsonObject& json)
{
  PluginWrapper::MetaData meta_data{};

  auto json_meta_data = json["MetaData"].toObject();
  meta_data.name = json_meta_data.contains("Name") ? json_meta_data["Name"].toString() : QString();
  meta_data.version = json_meta_data.contains("Version") ? json_meta_data["Version"].toString() : QString();
  meta_data.author = json_meta_data.contains("Author") ? json_meta_data["Author"].toString() : QString();
  meta_data.priority = json_meta_data.contains("Priority") ? json_meta_data["Priority"].toInt() : 0;

  auto json_topics = json_meta_data.contains("SupportedTopics") ? json_meta_data["SupportedTopics"].toArray() : QJsonArray();
  for (const auto& json_topic : json_topics)
  {
    PluginWrapper::MetaData::Topic topic;
    auto json_topic_toObject = json_topic.toObject();
    topic.name = json_topic_toObject.contains("Name") ? json_topic_toObject["Name"].toString() : QString();
    topic.type = json_topic_toObject.contains("Type") ? json_topic_toObject["Type"].toString() : QString();
    topic.format = json_topic_toObject.contains("Format") ? json_topic_toObject["Format"].toString() : QString();

    meta_data.topics.push_back(topic);
  }

  return meta_data;
}


PluginWrapper::PluginWrapper(const QString& path)
{
  QPluginLoader loader(path);

  auto json_meta_data = loader.metaData();
  if (!json_meta_data.empty())
  {
    meta_data_ = jsonToMetaStruct(json_meta_data);
    iid_ = json_meta_data["IID"].toString();
    path_ = path;
  }
  else
  {
    throw std::runtime_error("Invalid meta data.");
  }
}

PluginWidgetInterface* PluginWrapper::create(const QString& topic_name, const QString& topic_type, QWidget* parent)
{
  if (!isLoaded())
  {
    if (!load())
      return nullptr;
  }

  return instance_->create(topic_name, topic_type, parent);
}

std::unique_ptr<PluginLoader> PluginLoader::instance_;

PluginLoader::PluginLoader()
{
}

QChar pathSeparator()
{
#ifdef Q_OS_WIN
  return ';';
#else
  return ':';
#endif
}

void PluginLoader::discover()
{
  // only (re)discover plugins that are not already loaded
  QVector<QString> unused_plugin_iids;
  for (const auto& plugin : plugins_)
    if (!plugin.isLoaded())
      unused_plugin_iids.push_back(plugin.getIID());
  for (const auto& iid : unused_plugin_iids)
    plugins_.erase(plugins_.find(iid));

  // Specify the default plugin directory
  // On windows: cwd/ecalmon_plugins
  // On Linux: ../lib/ecal/mon_plugins
  auto default_plugin_dir = QDir(qApp->applicationDirPath());
#ifdef Q_OS_WIN
  default_plugin_dir.cd("ecalmon_plugins");
#else
  default_plugin_dir.cdUp();
  default_plugin_dir.cd("lib");
  default_plugin_dir.cd("ecal");
  default_plugin_dir.cd("plugins");
  default_plugin_dir.cd("mon");
#endif

  // Vector to specify all pl
  QVector<QDir> plugin_directories;
  plugin_directories.push_back(default_plugin_dir);

  // Retrieve additional plugin directories through the environment varialbe "ECAL_MON_PLUGIN_PATH"
  auto current_environment =  QProcessEnvironment::systemEnvironment();
  auto additional_plugin_paths = current_environment.value("ECAL_MON_PLUGIN_PATH");

  if (!additional_plugin_paths.isEmpty())
  {
    auto paths = additional_plugin_paths.split(pathSeparator());
    for (const auto& path : paths)
    {
      plugin_directories.push_back(QDir(path));
    }
  }

  // Iterate through all specified plugin directories to locate plugins
  for (const auto& dir : plugin_directories)
  {
    for (const auto& file_name : dir.entryList(QDir::Files)) {
      auto file_path = dir.absoluteFilePath(file_name);
      if (!QLibrary::isLibrary(file_path))
        continue;
      try {
        PluginWrapper plugin(file_path);
        auto iid = plugin.getIID();
        auto meta_data = plugin.getMetaData();

        auto plugin_iter = plugins_.find(iid);

        if (plugin_iter == plugins_.end())
          plugins_.insert(iid, plugin);
        else {
          if (plugin_iter->getPath() != file_path)
            eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_warning,
                               "Ambiguous plugin iid " + iid.toStdString() + " of " + meta_data.name.toStdString() + " " +
                               meta_data.version.toStdString() + ".  Plugin " + file_name.toStdString() +
                               " was not loaded.");
        }
      }
      catch (const std::runtime_error &e) {
        eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_warning,
                           std::string(e.what()) + " Plugin " + file_path.toStdString() + " was not loaded.");
      }
    }
  }  
}

QList<QString> PluginLoader::getAvailableIIDs() const
{
  return plugins_.keys();
}

PluginWrapper& PluginLoader::getPluginByIID(const QString& iid)
{
  return plugins_[iid];
}

static QPair<QString, QString> splitTopicType(const QString& topic_type)
{
  auto std_topic_type = topic_type.toStdString();
  auto position = std_topic_type.find(":");
  if (position == std::string::npos)
    return qMakePair(QString(), topic_type);

  auto format = std_topic_type.substr(0, position);
  std_topic_type.erase(0, position + 1);

  return qMakePair(QString(format.c_str()), QString(std_topic_type.c_str()));
}

QList<PluginWrapper> PluginLoader::getSupportedPlugins(const QString& topic_name, const QString& topic_type) const
{
  QList<PluginWrapper> supported_plugins;
  for (const auto& plugin : plugins_)
  {
    if (!plugin.isLoaded())
      continue;

    bool accepts_topic{ false };
    for (const auto& topic : plugin.getMetaData().topics)
    {
      if (
        (topic.name.isEmpty() || (topic.name == topic_name)) &&
        (topic.type.isEmpty() || (topic.type == splitTopicType(topic_type).second)) &&
        (topic.format.isEmpty() || (topic.format == splitTopicType(topic_type).first))
        )
      {
        accepts_topic = true;
        break;
      }
    }
    if (accepts_topic)
      supported_plugins.push_back(plugin);
  }

  qSort(supported_plugins.begin(), supported_plugins.end(), [](const auto& lhs, const auto& rhs)
  {
    return lhs.getMetaData().priority < rhs.getMetaData().priority;
  });

  return supported_plugins;
}


//static QPair<QString, QString> splitTopicType(const QString& topic_type)
//{
//  auto std_topic_type = topic_type.toStdString();
//  auto position = std_topic_type.find(":");
//  if (position == std::string::npos)
//    return qMakePair(QString(), topic_type);
//
//  auto format = std_topic_type.substr(0, position);
//  std_topic_type.erase(0, position + 1);
//
//  return qMakePair(QString(format.c_str()), QString(std_topic_type.c_str()));
//}
//
//QVector<QPair<PluginMetaData, PluginWidgetInterface*>> PluginLoader::createSupportedPluginWidgets(const QString& topic_name, const QString& topic_type, QWidget* parent )
//{
//  QVector<QPair<PluginMetaData, PluginWidgetInterface*>> plugin_widgets;
//
//  for (const auto& plugin : plugins_)
//  {
//    bool accepts_topic{ false };
//    for (const auto& topic : plugin.first.topics)
//    {
//      if (
//        (topic.name.isEmpty() || (topic.name == topic_name)) &&
//        (topic.type.isEmpty() || (topic.type == splitTopicType(topic_type).second)) &&
//        (topic.format.isEmpty() || (topic.format == splitTopicType(topic_type).first))
//        )
//      {
//        accepts_topic = true;
//        break;
//      }
//    }
//    if (accepts_topic)
//      plugin_widgets.push_back(qMakePair(plugin.first, plugin.second->create(topic_name, topic_type, parent)));
//  }
//
//  qSort(plugin_widgets.begin(), plugin_widgets.end(), [](const auto& lhs, const auto& rhs)
//  {
//    return lhs.first.priority < rhs.first.priority;
//  });
//
//  return plugin_widgets;
//}
//
PluginLoader* PluginLoader::getInstance()
{
  if (instance_.get() == nullptr)
  {
    instance_.reset(new PluginLoader);
  }

  return instance_.get();
}
