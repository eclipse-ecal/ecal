/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
 * @brief  eCALHDF5 measurement class <TODO>
**/

#include <ecalhdf5/eh5_meas_api_v2.h>

#include <algorithm>
#include <limits>

#include <ecalhdf5/eh5_meas_api_v3.h>
#include "datatype_helper.h"


namespace {
  using namespace eCAL::eh5;

  v3::eAccessType convert(v2::eAccessType v2_access)
  {
    switch (v2_access)
    {
    case v2::RDONLY:
      return v3::eAccessType::RDONLY;
    case v2::CREATE:
      return v3::eAccessType::CREATE_V5;
    default:
      return v3::eAccessType::RDONLY;
    }
  }

  SChannel createChannel(const std::string& channel_name)
  {
    return SChannel(channel_name, 0);
  }


  std::set<SChannel> GetChannelsWithName(const std::unique_ptr<v3::HDF5Meas>& hdf_meas_impl_, const std::string& channel_name)
  {
    const auto all_channels = hdf_meas_impl_->GetChannels();
    std::set<SChannel> named_channels;
    for (const auto& channel : all_channels)
    {
      if (channel.name == channel_name)
      {
        named_channels.insert(channel);
      }
    }
    return named_channels;
  }
}

using namespace eCAL::eh5::v2;

eCAL::eh5::v2::HDF5Meas::HDF5Meas()
: hdf_meas_impl_(std::make_unique<eCAL::eh5::v3::HDF5Meas>())
{}

// TODO restrict to V5 due to API
eCAL::eh5::v2::HDF5Meas::HDF5Meas(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
: hdf_meas_impl_(std::make_unique<eCAL::eh5::v3::HDF5Meas>(path, convert(access)))
{}

eCAL::eh5::v2::HDF5Meas::~HDF5Meas()
= default;

bool eCAL::eh5::v2::HDF5Meas::Open(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
{
  return hdf_meas_impl_->Open(path, convert(access));
}

bool eCAL::eh5::v2::HDF5Meas::Close()
{
  return hdf_meas_impl_->Close();
}

bool eCAL::eh5::v2::HDF5Meas::IsOk() const
{
  return hdf_meas_impl_->IsOk();
}

std::string eCAL::eh5::v2::HDF5Meas::GetFileVersion() const
{
  return hdf_meas_impl_->GetFileVersion();
}

size_t eCAL::eh5::v2::HDF5Meas::GetMaxSizePerFile() const
{
  return hdf_meas_impl_->GetMaxSizePerFile();
}

void eCAL::eh5::v2::HDF5Meas::SetMaxSizePerFile(size_t size)
{
  return hdf_meas_impl_->SetMaxSizePerFile(size);
}

bool eCAL::eh5::v2::HDF5Meas::IsOneFilePerChannelEnabled() const
{
  return hdf_meas_impl_->IsOneFilePerChannelEnabled();
}

void eCAL::eh5::v2::HDF5Meas::SetOneFilePerChannelEnabled(bool enabled)
{
  return hdf_meas_impl_->SetOneFilePerChannelEnabled(enabled);
}

std::set<std::string> eCAL::eh5::v2::HDF5Meas::GetChannelNames() const
{
  auto channels = hdf_meas_impl_->GetChannels();
  std::set<std::string> channel_names;
  for (const auto& channel : channels) {
    channel_names.insert(channel.name);
  }
  return channel_names;
}

bool eCAL::eh5::v2::HDF5Meas::HasChannel(const std::string& channel_name) const
{
  auto named_channels = GetChannelsWithName(hdf_meas_impl_, channel_name);
  bool has_channel = false;
  for (const auto& channel : named_channels)
  {
    has_channel |= hdf_meas_impl_->HasChannel(channel);
  }
  return has_channel;
}

std::string eCAL::eh5::v2::HDF5Meas::GetChannelDescription(const std::string& channel_name) const
{
  auto datatype_info = GetChannelDataTypeInformation(channel_name);
  return datatype_info.descriptor;
}

void eCAL::eh5::v2::HDF5Meas::SetChannelDescription(const std::string& channel_name, const std::string& description)
{
  const auto& channel = createChannel(channel_name);
  auto& current_info = data_type_info_map[channel_name];
  current_info.descriptor = description;
  hdf_meas_impl_->SetChannelDataTypeInformation(channel, current_info);
}

std::string eCAL::eh5::v2::HDF5Meas::GetChannelType(const std::string& channel_name) const
{
  std::string ret_val;
  auto datatype_info = GetChannelDataTypeInformation(channel_name);
  std::tie(ret_val, std::ignore) = FromInfo(datatype_info);
  return ret_val;
}

void eCAL::eh5::v2::HDF5Meas::SetChannelType(const std::string& channel_name, const std::string& type)
{
  const auto& channel = createChannel(channel_name);
  auto& current_info = data_type_info_map[channel_name];
  current_info = CreateInfo(type, current_info.descriptor);
  hdf_meas_impl_->SetChannelDataTypeInformation(channel, current_info);
}

// This function "has" to loose information if there are multople channels in the measurement with the same name
eCAL::eh5::DataTypeInformation eCAL::eh5::v2::HDF5Meas::GetChannelDataTypeInformation(const std::string& channel_name) const
{
  auto named_channels = GetChannelsWithName(hdf_meas_impl_, channel_name);
  if (named_channels.size() > 0)
  {
    return hdf_meas_impl_->GetChannelDataTypeInformation(*named_channels.begin());
  }
  return DataTypeInformation{};
}

void eCAL::eh5::v2::HDF5Meas::SetChannelDataTypeInformation(const std::string& channel_name, const eCAL::eh5::DataTypeInformation& info)
{
  data_type_info_map[channel_name] = info;
  return hdf_meas_impl_->SetChannelDataTypeInformation(createChannel(channel_name), info);
}

long long eCAL::eh5::v2::HDF5Meas::GetMinTimestamp(const std::string& channel_name) const
{
  auto named_channels = GetChannelsWithName(hdf_meas_impl_, channel_name);
  long long min_timestamp = std::numeric_limits<long long>::max();
  for (const auto& channel : named_channels)
  {
    min_timestamp = std::min(min_timestamp, hdf_meas_impl_->GetMinTimestamp(channel));
  }
  if (min_timestamp == std::numeric_limits<long long>::max())
    return 0; // the old API would return 0 in this case
  return min_timestamp;
}

long long eCAL::eh5::v2::HDF5Meas::GetMaxTimestamp(const std::string& channel_name) const
{
  auto named_channels = GetChannelsWithName(hdf_meas_impl_, channel_name);
  long long max_timestamp = std::numeric_limits<long long>::min();
  for (const auto& channel : named_channels)
  {
    max_timestamp = std::max(max_timestamp, hdf_meas_impl_->GetMaxTimestamp(channel));
  }
  if (max_timestamp == std::numeric_limits<long long>::min())
    return 0; // the old API would return 0 in this case
  return max_timestamp;
}

bool eCAL::eh5::v2::HDF5Meas::GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const
{
  // we need to aggregate info from all channels within the measurement with a given name.
  auto named_channels = GetChannelsWithName(hdf_meas_impl_, channel_name);
  entries.clear();
  bool ret_value{ true };
  for (const auto& channel : named_channels)
  {
    EntryInfoSet channel_entries;
    ret_value &= hdf_meas_impl_->GetEntriesInfo(channel, channel_entries);
    entries.insert(channel_entries.begin(), channel_entries.end());
  }
  return ret_value;
}

bool eCAL::eh5::v2::HDF5Meas::GetEntriesInfoRange(const std::string& channel_name, long long begin, long long end, EntryInfoSet& entries) const
{
  // we need to aggregate info from all channels within the measurement with a given name.
  auto named_channels = GetChannelsWithName(hdf_meas_impl_, channel_name);
  entries.clear();
  bool ret_value{ true };
  for (const auto& channel : named_channels)
  {
    EntryInfoSet channel_entries;
    ret_value &= hdf_meas_impl_->GetEntriesInfoRange(channel, begin, end, channel_entries);
    entries.insert(channel_entries.begin(), channel_entries.end());
  }
  return ret_value;
}

bool eCAL::eh5::v2::HDF5Meas::GetEntryDataSize(long long entry_id, size_t& size) const
{
  return hdf_meas_impl_->GetEntryDataSize(entry_id, size);
}

bool eCAL::eh5::v2::HDF5Meas::GetEntryData(long long entry_id, void* data) const
{
  return hdf_meas_impl_->GetEntryData(entry_id, data);
}

void eCAL::eh5::v2::HDF5Meas::SetFileBaseName(const std::string& base_name)
{
  return hdf_meas_impl_->SetFileBaseName(base_name);
}

bool eCAL::eh5::v2::HDF5Meas::AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock)
{
  SWriteEntry entry;
  entry.channel = createChannel(channel_name);
  entry.data = data;
  entry.size = size;
  entry.snd_timestamp = snd_timestamp;
  entry.rcv_timestamp = rcv_timestamp;
  entry.sender_id = id;
  entry.clock = clock;
  return hdf_meas_impl_->AddEntryToFile(entry);
}

void eCAL::eh5::v2::HDF5Meas::ConnectPreSplitCallback(CallbackFunction cb)
{
  return hdf_meas_impl_->ConnectPreSplitCallback(std::move(cb));
}

void eCAL::eh5::v2::HDF5Meas::DisconnectPreSplitCallback()
{
  return hdf_meas_impl_->DisconnectPreSplitCallback();
}
