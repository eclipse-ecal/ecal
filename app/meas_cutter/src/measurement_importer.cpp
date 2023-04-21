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

#include "measurement_importer.h"

MeasurementImporter::MeasurementImporter() :
  _reader(new eCAL::eh5::Reader),
  _current_opened_channel_data()
{
}

void MeasurementImporter::setPath(const std::string& path)
{
  _loaded_path = path;
  if (isEcalMeasFile(path))
  {
    _loaded_path = EcalUtils::Filesystem::CleanPath(path + "/..", EcalUtils::Filesystem::OsStyle::Current);
  }

  if (!_reader->Open(_loaded_path))
  {
    throw ImporterException("Unable to open HDF5 path " + path + ".");
  }

  if (!_reader->IsOk())
  {
    throw ImporterException("One or more HDF5 files are damaged.");
  }

  _channel_names = eCALMeasCutterUtils::ChannelNameSet(_reader->GetChannelNames());
}

MeasurementImporter::~MeasurementImporter()
{
  _reader->Close();
}

eCALMeasCutterUtils::ChannelNameSet MeasurementImporter::getChannelNames() const
{
  return _channel_names;
}

bool MeasurementImporter::hasChannel(const std::string& channel_name) const
{
  return _reader->HasChannel(channel_name);
}

void MeasurementImporter::openChannel(const std::string& channel_name)
{
  _current_opened_channel_data._timestamps.clear();
  _current_opened_channel_data._timestamp_entry_info_map.clear();

  if (isProtoChannel(_reader->GetChannelType(channel_name)))
  {
    _current_opened_channel_data._channel_info.format = eCALMeasCutterUtils::SerializationFormat::PROTOBUF;
    _current_opened_channel_data._channel_info.type   = _reader->GetChannelType(channel_name).substr(6); // remove "proto:" from type string
  }
  else
  {
    _current_opened_channel_data._channel_info.format = eCALMeasCutterUtils::SerializationFormat::UNKNOWN;
    _current_opened_channel_data._channel_info.type   = _reader->GetChannelType(channel_name);
  }
  _current_opened_channel_data._channel_info.description = _reader->GetChannelDescription(channel_name);
  _current_opened_channel_data._channel_info.name = channel_name;

  eCAL::measurement::base::EntryInfoSet entry_info_set;
  _reader->GetEntriesInfo(channel_name, entry_info_set);

  for (const auto& entry_info : entry_info_set)
  {
    _current_opened_channel_data._timestamps.insert(entry_info.RcvTimestamp);
    _current_opened_channel_data._timestamp_entry_info_map.insert(std::make_pair(entry_info.RcvTimestamp, entry_info));
  }
}

eCALMeasCutterUtils::ChannelInfo MeasurementImporter::getChannelInfoforCurrentChannel() const
{
  return _current_opened_channel_data._channel_info;
}

eCALMeasCutterUtils::TimestampSet MeasurementImporter::getTimestamps() const
{
  return _current_opened_channel_data._timestamps;
}

void MeasurementImporter::getData(eCALMeasCutterUtils::Timestamp timestamp, eCALMeasCutterUtils::MetaData& meta_data, std::string& data)
{
  const auto& entry_info = _current_opened_channel_data._timestamp_entry_info_map.at(timestamp);

  auto data_id = entry_info.ID;

  size_t size = 0;
  _reader->GetEntryDataSize(data_id, size);

   std::vector<char> buffer(size);
   _reader->GetEntryData(data_id, buffer.data());

  data.assign(buffer.data(), size);

  meta_data.clear();
  meta_data[eCALMeasCutterUtils::MetaDatumKey::RECEIVER_TIMESTAMP].receiver_timestamp = entry_info.RcvTimestamp;
  meta_data[eCALMeasCutterUtils::MetaDatumKey::SENDER_TIMESTAMP].sender_timestamp = entry_info.SndTimestamp;
  meta_data[eCALMeasCutterUtils::MetaDatumKey::SENDER_CLOCK].sender_clock = entry_info.SndClock;
  meta_data[eCALMeasCutterUtils::MetaDatumKey::SENDER_ID].sender_id = entry_info.SndID;
}

std::pair<eCALMeasCutterUtils::Timestamp, eCALMeasCutterUtils::Timestamp> MeasurementImporter::getOriginalStartFinishTimestamps()
{
  std::pair<eCALMeasCutterUtils::Timestamp, eCALMeasCutterUtils::Timestamp> timespan(std::numeric_limits<eCALMeasCutterUtils::Timestamp>::max(), std::numeric_limits<eCALMeasCutterUtils::Timestamp>::min());

  if (_channel_names.size() == 0) return std::make_pair<eCALMeasCutterUtils::Timestamp, eCALMeasCutterUtils::Timestamp>(0, 0);

  for (const auto& meta_mapping : _channel_names)
  {
    auto timestamp_start = _reader->GetMinTimestamp(meta_mapping);
    auto timestamp_end = _reader->GetMaxTimestamp(meta_mapping);

    timespan.first = (timestamp_start < timespan.first) ? timestamp_start : timespan.first;
    timespan.second = (timestamp_end > timespan.second) ? timestamp_end : timespan.second;
  }

  return timespan;
}

std::list<std::string> MeasurementImporter::getChannelNamesForRegex(const std::regex& regex)
{
  std::list<std::string> channel_names_matches;
  for (const auto& channel_name : _channel_names)
  {
    if (std::regex_match(channel_name, regex))
    {
      channel_names_matches.emplace_back(channel_name);
    }
  }
  return channel_names_matches;
}

bool MeasurementImporter::isEcalMeasFile(const std::string& path)
{
  // Check if the path received is a .ecalmeas file
  auto file_status = EcalUtils::Filesystem::FileStatus(path, EcalUtils::Filesystem::OsStyle::Current);
  if (file_status.GetType() == EcalUtils::Filesystem::Type::RegularFile)
  {
    std::string filename = EcalUtils::Filesystem::FileName(path, EcalUtils::Filesystem::OsStyle::Current);
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos != std::string::npos)
    {
      std::string file_extension = filename.substr(dot_pos);
      std::transform(file_extension.begin(), file_extension.end(), file_extension.begin(),
        [](char c) -> char
        {
          return static_cast<char>(std::tolower(static_cast<int>(c)));
        });
      if (file_extension == ".ecalmeas")
      {
        return true;
      }
    }
  }
  return false;
}

bool MeasurementImporter::isProtoChannel(const std::string& channel_type)
{
  std::string space = channel_type.substr(0, channel_type.find_first_of(':'));
  return (space.compare("proto") == 0);
}

std::string MeasurementImporter::getLoadedPath()
{
  return _loaded_path;
}

bool MeasurementImporter::areChannelsLeftToInclude(const std::list<std::string>& channels_to_exclude)
{
  for (const auto& channel_name : _channel_names)
  {
    // if we have at least a channel that is not excluded
    if (std::find(channels_to_exclude.begin(), channels_to_exclude.end(), channel_name) == channels_to_exclude.end())
      return true;
  }
  return false;
}
