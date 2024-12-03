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
 * @brief  eCALHDF5 measurement file version V2
**/

#include "eh5_meas_file_v2.h"
#include "hdf5_helper.h"
#include "datatype_helper.h"

#include "hdf5.h"
#include <ecal_utils/string.h>

#include <iostream>
#include <list>
#include <set>

eCAL::eh5::HDF5MeasFileV2::HDF5MeasFileV2()
  : file_id_(-1)
{
#ifndef _DEBUG
  H5Eset_auto(0, nullptr, nullptr);
#endif  //  _DEBUG
}

eCAL::eh5::HDF5MeasFileV2::HDF5MeasFileV2(const std::string& path, v3::eAccessType access /*= eAccessType::RDONLY*/)
  : file_id_(-1)
{
#ifndef _DEBUG
  H5Eset_auto(0, nullptr, nullptr);
#endif  //  _DEBUG

  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasFileV2::Open(path, access);
}

eCAL::eh5::HDF5MeasFileV2::~HDF5MeasFileV2()
{
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasFileV2::Close();
}


bool eCAL::eh5::HDF5MeasFileV2::Open(const std::string& path, v3::eAccessType access /*= eAccessType::RDONLY*/)
{
  if (file_id_ > 0) Close();
  if (path.empty()) return false;
  if (access != v3::eAccessType::RDONLY) return false;

  file_id_ = H5Fopen(path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

  // call the function via its class becase it's a virtual function that is called directly/indirectly in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  return HDF5MeasFileV2::IsOk();
}


bool eCAL::eh5::HDF5MeasFileV2::Close()
{
  if (HDF5MeasFileV2::IsOk() && H5Fclose(file_id_) >= 0)
  {
    file_id_ = -1;
    return true;
  }
  else
  {
    return false;
  }
}


bool eCAL::eh5::HDF5MeasFileV2::IsOk() const
{
  return (file_id_ >= 0);
}


std::string eCAL::eh5::HDF5MeasFileV2::GetFileVersion() const
{
  std::string file_version;
  GetAttribute(file_id_, kFileVerAttrTitle, file_version);
  return file_version;
}


size_t eCAL::eh5::HDF5MeasFileV2::GetMaxSizePerFile() const
{
  return 0;
}


void eCAL::eh5::HDF5MeasFileV2::SetMaxSizePerFile(size_t /*size*/)
{
}


bool eCAL::eh5::HDF5MeasFileV2::IsOneFilePerChannelEnabled() const
{
  return false;
}

void eCAL::eh5::HDF5MeasFileV2::SetOneFilePerChannelEnabled(bool /*enabled*/)
{
}

std::set<eCAL::eh5::SChannel> eCAL::eh5::HDF5MeasFileV2::GetChannels() const
{
  std::set<eCAL::eh5::SChannel> channels_set;

  std::string combined_channel_names;
  GetAttribute(file_id_, kChnAttrTitle, combined_channel_names);

  std::list<std::string> channel_name_list;
  EcalUtils::String::Split(combined_channel_names, ",", channel_name_list);

  for (const auto& channel_name : channel_name_list)
    channels_set.insert(eCAL::experimental::measurement::base::CreateChannel(channel_name));

  return channels_set;
}

bool eCAL::eh5::HDF5MeasFileV2::HasChannel(const eCAL::eh5::SChannel& channel) const
{
  auto channels = GetChannels();

  return std::find(channels.cbegin(), channels.cend(), channel) != channels.end();
}

eCAL::eh5::DataTypeInformation eCAL::eh5::HDF5MeasFileV2::GetChannelDataTypeInformation(const SChannel& channel) const
{
  std::string type;
  std::string description;

  if (this->IsOk())
  {
    auto dataset_id = H5Dopen(file_id_, channel.name.c_str(), H5P_DEFAULT);
    if (dataset_id >= 0)
    {
      GetAttribute(dataset_id, kChnTypeAttrTitle, type);
      GetAttribute(dataset_id, kChnDescAttrTitle, description);
      H5Dclose(dataset_id);
    }
  }

  return CreateInfo(type, description);
}

void eCAL::eh5::HDF5MeasFileV2::SetChannelDataTypeInformation(const SChannel& /*channel*/ , const eCAL::eh5::DataTypeInformation& /*info*/)
{
}


long long eCAL::eh5::HDF5MeasFileV2::GetMinTimestamp(const SChannel& channel) const
{
  long long ret_val = 0;
  EntryInfoSet entries;

  if (GetEntriesInfo(channel, entries) && !entries.empty())
  {
    ret_val = entries.begin()->RcvTimestamp;
  }

  return ret_val;
}

long long eCAL::eh5::HDF5MeasFileV2::GetMaxTimestamp(const SChannel& channel) const
{
  long long ret_val = 0;
  EntryInfoSet entries;

  if (GetEntriesInfo(channel, entries) && !entries.empty())
  {
    ret_val = entries.rbegin()->RcvTimestamp;
  }

  return ret_val;
}

bool eCAL::eh5::HDF5MeasFileV2::GetEntriesInfo(const SChannel& channel, EntryInfoSet& entries) const
{
  entries.clear();

  if (!this->IsOk()) return false;

  auto dataset_id = H5Dopen(file_id_, channel.name.c_str(), H5P_DEFAULT);

  if (dataset_id < 0) return false;

  const size_t sizeof_ll = sizeof(long long);
  hsize_t data_size = H5Dget_storage_size(dataset_id) / sizeof_ll;

  if (data_size <= 0) return false;

  long long* data = static_cast<long long*>(calloc(static_cast<size_t>(data_size), sizeof_ll));

  herr_t status = H5Dread(dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
  if (data == nullptr) return false;

  H5Dclose(dataset_id);

  for (unsigned int index = 0; index < data_size; index += 2)
  {
    //                        rec timestamp, channel id
    entries.emplace(SEntryInfo(data[index],   data[index + 1], 0));
  }

  free(data);
  data = nullptr;

  return (status >= 0);
}

bool eCAL::eh5::HDF5MeasFileV2::GetEntriesInfoRange(const SChannel& channel, long long begin, long long end, EntryInfoSet& entries) const
{
  bool ret_val = false;
  EntryInfoSet all_entries;
  entries.clear();

  if (GetEntriesInfo(channel, all_entries) && !all_entries.empty())
  {
    if (begin == 0) begin = entries.begin()->RcvTimestamp;
    if (end == 0) end = entries.rbegin()->RcvTimestamp;

    const auto& lower = all_entries.lower_bound(SEntryInfo(begin, 0, 0));
    const auto& upper = all_entries.upper_bound(SEntryInfo(end, 0, 0));

    entries.insert(lower, upper);
    ret_val = true;
  }

  return ret_val;
}

bool eCAL::eh5::HDF5MeasFileV2::GetEntryDataSize(long long entry_id, size_t& size) const
{
  if (!this->IsOk()) return false;

  auto dataset_id = H5Dopen(file_id_, std::to_string(entry_id).c_str(), H5P_DEFAULT);

  if (dataset_id < 0) return false;

  size = static_cast<size_t>(H5Dget_storage_size(dataset_id));

  H5Dclose(dataset_id);

  return true;
}

bool eCAL::eh5::HDF5MeasFileV2::GetEntryData(long long entry_id, void* data) const
{
  if (data == nullptr) return false;

  if (!this->IsOk()) return false;

  auto dataset_id = H5Dopen(file_id_, std::to_string(entry_id).c_str(), H5P_DEFAULT);

  if (dataset_id < 0) return false;

  auto size = H5Dget_storage_size(dataset_id);

  herr_t read_status = -1;
  if (size >= 0)
  {
    read_status = H5Dread(dataset_id, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
  }

  H5Dclose(dataset_id);

  return (read_status >= 0);
}


void eCAL::eh5::HDF5MeasFileV2::SetFileBaseName(const std::string& /*base_name*/)
{

}

bool eCAL::eh5::HDF5MeasFileV2::AddEntryToFile(const SWriteEntry& /*entry*/)
{
    return false;
}

void eCAL::eh5::HDF5MeasFileV2::ConnectPreSplitCallback(CallbackFunction /*cb*/)
{
}

void eCAL::eh5::HDF5MeasFileV2::DisconnectPreSplitCallback()
{
}