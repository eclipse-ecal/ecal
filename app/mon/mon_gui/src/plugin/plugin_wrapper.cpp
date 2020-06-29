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

#include <plugin/plugin_wrapper.h>

#include <QJsonArray>
#include <QPluginLoader>
#define HAS_Q_VERSION_NUMBER (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
#if HAS_Q_VERSION_NUMBER
#include <QVersionNumber>
#endif

#include <stdexcept>

#include <ecal/ecal_log.h>

QPair<QString, QString> splitTopicType(const QString& topic_type)
{
  auto std_topic_type = topic_type.toStdString();
  auto position = std_topic_type.find(":");
  if (position == std::string::npos)
    return qMakePair(QString(), topic_type);

  auto format = std_topic_type.substr(0, position);
  std_topic_type.erase(0, position + 1);

  return qMakePair(QString(format.c_str()), QString(std_topic_type.c_str()));
}

PluginWrapper::PluginData::MetaData jsonToMetaStruct(const QJsonObject& json)
{
  PluginWrapper::PluginData::MetaData meta_data{};

  auto json_meta_data = json["MetaData"].toObject();
  meta_data.name = json_meta_data.contains("Name") ? json_meta_data["Name"].toString() : QString();
  meta_data.version = json_meta_data.contains("Version") ? json_meta_data["Version"].toString() : QString();
  meta_data.author = json_meta_data.contains("Author") ? json_meta_data["Author"].toString() : QString();
  meta_data.priority = json_meta_data.contains("Priority") ? json_meta_data["Priority"].toInt() : 0;

  auto json_topics = json_meta_data.contains("SupportedTopics") ? json_meta_data["SupportedTopics"].toArray() : QJsonArray();
  for (const auto& json_topic : json_topics)
  {
    PluginWrapper::PluginData::MetaData::Topic topic;
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
    plugin_data_ = PluginData
    {
      json_meta_data["IID"].toString(),
      path,
      jsonToMetaStruct(json_meta_data)
    };
  }
  else
  {
    throw std::runtime_error("Invalid meta data.");
  }
}

const PluginWrapper::PluginData& PluginWrapper::getPluginData() const
{
  return plugin_data_;
}

bool PluginWrapper::load()
{
  if (!isLoaded())
  {
    QPluginLoader loader(plugin_data_.path);
    auto instance = loader.instance();
    if (instance)
    {
      eCAL::mon::PluginInterface* plugin = qobject_cast<eCAL::mon::PluginInterface*>(instance);
      if (plugin)
      {
        eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_info, "Plugin " + plugin_data_.meta_data.name.toStdString() + " " + plugin_data_.meta_data.version.toStdString() + " has been loaded.");
        instance_ = std::shared_ptr<eCAL::mon::PluginInterface>(plugin,
          [this](eCAL::mon::PluginInterface* p)
        {
          eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_info, "Plugin " + plugin_data_.meta_data.name.toStdString() + " " + plugin_data_.meta_data.version.toStdString() + " has been unloaded.");
          delete dynamic_cast<QObject*>(p);
        });
        return true;
      }
      else
      {
        eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_warning, "Plugin " + plugin_data_.path.toStdString() + " is not suitable for eCAL Monitor");
        loader.unload();
      }
    }
    else
      eCAL::Logging::Log(eCAL_Logging_eLogLevel::log_level_warning, "Unable to load plugin " + plugin_data_.path.toStdString() + ".");
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

eCAL::mon::PluginWidgetInterface* PluginWrapper::create(const QString& topic_name, const QString& topic_type, QWidget* parent)
{
  if (!isLoaded())
  {
    if (!load())
      return nullptr;
  }

  return instance_->create(topic_name, topic_type, parent);
}

bool AcceptsTopic(const PluginWrapper & wrapper, QString topic_name, QString topic_type)
{
  for (const auto& topic : wrapper.getPluginData().meta_data.topics)
  {
    if (
      (topic.name.isEmpty() || (topic.name == topic_name)) &&
      (topic.type.isEmpty() || (topic.type == splitTopicType(topic_type).second)) &&
      (topic.format.isEmpty() || (topic.format == splitTopicType(topic_type).first))
      )
    {
      return true;
    }
  }
  return false;
}

bool PluginWrapper::PluginData::MetaData::operator<(const MetaData & rhs) const
{
  {
#if HAS_Q_VERSION_NUMBER
    auto qversion{ QVersionNumber::fromString(version) };
    auto rhs_qversion{ QVersionNumber::fromString(rhs.version) };
    return std::tie(priority, name, qversion) < std::tie(rhs.priority, rhs.name, rhs_qversion);
#else
    return std::tie(priority, name, version) < std::tie(rhs.priority, rhs.name, rhs.version);
#endif
  }
}

bool PluginWrapper::PluginData::operator<(const PluginWrapper::PluginData& rhs) const
{
  return meta_data < rhs.meta_data;
}

bool PluginWrapper::operator<(const PluginWrapper& rhs) const
{
  return plugin_data_ < rhs.plugin_data_;
}