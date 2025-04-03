/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#include <plugin/plugin_manager.h>

#include <memory>
#include <stdexcept>

#include <QApplication>
#include <QDir>
#include <QLibrary>
#include <QProcessEnvironment>

#include <ecal/log.h>
#include <ecal/defs.h>

std::unique_ptr<PluginManager> PluginManager::instance_;


PluginManager::PluginManager()
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

// Function returns a list of directories which are considered valid locations for plugins.
QVector<QDir> GetPluginDirectories()
{
  QVector<QDir> plugin_directories;


  

  // Retrieve additional plugin directories through the environment varialbe "ECAL_MON_PLUGIN_PATH"
  auto current_environment = QProcessEnvironment::systemEnvironment();
  auto additional_plugin_paths   = current_environment.value("ECAL_MON_PLUGIN_PATH");

  if (!additional_plugin_paths.isEmpty())
  {
    auto paths = additional_plugin_paths.split(pathSeparator());
    for (const auto& path : paths)
    {
      plugin_directories.push_back(QDir(path));
    }
  }

#ifndef Q_OS_WIN
  // Add the actually installed plugin directory (which may be a multiarch lib dir!)

  QString installed_plugin_dir;

  if (QString(ECAL_INSTALL_PREFIX).isEmpty()
     || QString(ECAL_INSTALL_LIB_DIR).startsWith('/'))
  {
    // The Path is absolute or the prefix is empty anyways
    installed_plugin_dir = QString(ECAL_INSTALL_LIB_DIR);
  }
  else
  {
    installed_plugin_dir = QString(ECAL_INSTALL_PREFIX) + QString("/") + QString(ECAL_INSTALL_LIB_DIR);
  }
  installed_plugin_dir += QString("/ecal/plugins/mon");

  plugin_directories.push_back(QDir(installed_plugin_dir));
#endif

  // Specify the default plugin directory
  // On windows: cwd/ecalmon_plugins
  // On Linux: ../lib/ecal/plugins/mon
  QString default_plugin_dir = qApp->applicationDirPath();
#ifdef Q_OS_WIN
  default_plugin_dir += QString("/ecalmon_plugins");
#else
  default_plugin_dir += QString("/../lib/ecal/plugins/mon");
#endif
  plugin_directories.push_back(QDir(default_plugin_dir));

  return plugin_directories;
}

// Function iterates over all specified directories and returns a list of library files contained in these directories
QVector<QString> GetPluginLibraryPaths(const QVector<QDir>& plugin_directories)
{
  QVector<QString> plugin_library_paths;
  for (const auto& dir : plugin_directories)
  {
    for (const auto& file_name : dir.entryList(QDir::Files))
    {
      auto file_path = dir.absoluteFilePath(file_name);
      if (QLibrary::isLibrary(file_path))
      {
        plugin_library_paths.push_back(file_path);
      }
    }
  }
  return plugin_library_paths;
}

// Add Found plugins and deactivate them by default
void PluginManager::AddFoundPlugins(const QVector<QString>& library_paths)
{
  for (const auto& file_path : library_paths)
  {
    try {
      PluginWrapper plugin(file_path);
      auto iid = plugin.getPluginData().iid;
      auto meta_data = plugin.getPluginData().meta_data;

      auto plugin_iter = plugins_.find(iid);

      if (plugin_iter == plugins_.end())
        plugins_.insert(iid, ActivePlugin{ plugin, false });
      else {
        if (plugin_iter->wrapper.getPluginData().path != file_path)
          eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_warning,
            "Ambiguous plugin iid " + iid.toStdString() + " of " + meta_data.name.toStdString() + " " +
            meta_data.version.toStdString() + ".  Plugin " + file_path.toStdString() +
            " was not loaded.");
      }
    }
    catch (const std::runtime_error &e) {
      eCAL::Logging::Log(eCAL::Logging::eLogLevel::log_level_warning,
        std::string(e.what()) + " Plugin " + file_path.toStdString() + " was not loaded.");
    }
  }
}

void PluginManager::ActivateDistinctPlugins()
{
  // Map that contains a vector of references to all plugins with the same name
  QMap<QString, std::vector<std::reference_wrapper<ActivePlugin>>> plugins_by_name;
  // 1. Get distinct plugin names
  for (auto& plugin : plugins_)
  {
    QString name = plugin.wrapper.getPluginData().meta_data.name;
    plugins_by_name[name].emplace_back(plugin);
  }
  // 2. For each distinct name
  for (auto& vector_of_plugins_with_same_name : plugins_by_name)
  {
    // 3. Activate plugin with highest version number
    auto max_plugin = std::max_element(vector_of_plugins_with_same_name.begin(), vector_of_plugins_with_same_name.end(), std::less<ActivePlugin>{});
    max_plugin->get().active = true;
  }
}

// This function creates a list with plugins
void PluginManager::discover()
{
  auto plugin_directories = GetPluginDirectories();
  auto library_paths = GetPluginLibraryPaths(plugin_directories);

  AddFoundPlugins(library_paths);
  ActivateDistinctPlugins();
}

QList<QString> PluginManager::getAvailableIIDs() const
{
  return plugins_.keys();
}

PluginWrapper::PluginData PluginManager::getPluginData(const QString & iid) const
{
  return plugins_[iid].wrapper.getPluginData();
}

bool PluginManager::isActive(const QString & iid) const
{
  return plugins_[iid].active;
}

void PluginManager::setActive(const QString & iid, bool active)
{
  plugins_[iid].active = active;
}



QList<QPair<PluginWrapper::PluginData, eCAL::mon::PluginWidgetInterface*>> PluginManager::CreatePlugins(QString topic_name, QString topic_type, QWidget* parent)
{
  QList<QPair<PluginWrapper::PluginData, eCAL::mon::PluginWidgetInterface*>> supported_plugins;
  for (auto& plugin : plugins_)
  {
    if (!plugin.active)
      continue;

    bool accepts_topic{ AcceptsTopic(plugin.wrapper, topic_name, topic_type) };
    if (accepts_topic)
    {
      auto interface { plugin.wrapper.create(topic_name, topic_type, parent) };
      if (interface)
      {
        supported_plugins.push_back(QPair<PluginWrapper::PluginData, eCAL::mon::PluginWidgetInterface*>{plugin.wrapper.getPluginData(), interface});
      }
    }
  }

  // Sort return vector by priority
  std::sort(supported_plugins.begin(), supported_plugins.end(), [](const auto& lhs, const auto& rhs)
  {
    return lhs.first.meta_data.priority < rhs.first.meta_data.priority;
  });

  return supported_plugins;
}

eCAL::mon::PluginWidgetInterface* PluginManager::CreatePlugin(const QString& topic_name, const QString& topic_type, const QString& plugin_iid, QWidget* parent)
{
  auto plugin_iter = plugins_.find(plugin_iid);
  if (plugin_iter == plugins_.end())
    return nullptr;
  
  if (!plugin_iter->active)
    return nullptr;

  return plugin_iter->wrapper.create(topic_name, topic_type, parent);
}

QList<PluginWrapper::PluginData> PluginManager::getMatchingPluginData(const QString& topic_name, const QString& topic_type) const
{
  QList<PluginWrapper::PluginData> matching_plugin_data;
  for (const auto& plugin : plugins_)
  {
    if (!plugin.active)
      continue;

    if (AcceptsTopic(plugin.wrapper, topic_name, topic_type))
    {
      matching_plugin_data.push_back(plugin.wrapper.getPluginData());
    }
  }
  return matching_plugin_data;
}

PluginManager* PluginManager::getInstance()
{
  if (!instance_)
  {
    instance_.reset(new PluginManager);
  }

  return instance_.get();
}
