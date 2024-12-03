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
 * @brief  eCALHDF5 directory reader
**/

#include "eh5_meas_file_writer_v6.h"
#include "escape.h"

#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif //WIN32

#include <string>
#include <list>
#include <iostream>

#include <ecal_utils/filesystem.h>
#include <ecal_utils/str_convert.h>

#include "hdf5_helper.h"

constexpr unsigned int kDefaultMaxFileSizeMB = 1000;

eCAL::eh5::HDF5MeasFileWriterV6::HDF5MeasFileWriterV6()
  : cb_pre_split_      (nullptr)
  , file_id_           (-1)
  , file_split_counter_(-1)
  , entries_counter_   (0)
  , max_size_per_file_ (kDefaultMaxFileSizeMB * 1024 * 1024)
{}

eCAL::eh5::HDF5MeasFileWriterV6::~HDF5MeasFileWriterV6()
{
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasFileWriterV6::Close();
}

bool eCAL::eh5::HDF5MeasFileWriterV6::Open(const std::string& output_dir, v3::eAccessType /*access = eAccessType::RDONLY*/)
{
  Close();

  // Check if the given path points to a directory
  if (!EcalUtils::Filesystem::IsDir(output_dir, EcalUtils::Filesystem::Current))
    return false;

  output_dir_ = output_dir;

  return true;
}

bool eCAL::eh5::HDF5MeasFileWriterV6::Close()
{
  if (!this->IsOk())  return false;

  std::string channels_with_entries;

  for (const auto& channel_per_name : channels_)
  {
    for (const auto& channel_per_id : channel_per_name.second)
    {
      std::ignore = CreateEntriesTableOfContentsFor(channel_per_name.first, channel_per_id.first, channel_per_id.second.Info, channel_per_id.second.Entries);
    }
    channels_with_entries += channel_per_name.first + ",";
  }

  if ((!channels_with_entries.empty())  && (channels_with_entries.back() == ','))
    channels_with_entries.pop_back();

  SetAttribute(file_id_, kChnAttrTitle, channels_with_entries);

  for (auto& channel_per_name : channels_)
    for (auto& channel_per_id : channel_per_name.second)
      channel_per_id.second.Entries.clear();

  if (H5Fclose(file_id_) >= 0)
  {
    file_id_ = -1;
    return true ;
  }
  else
  {
    return false;
  }
}

bool eCAL::eh5::HDF5MeasFileWriterV6::IsOk() const
{
  return (file_id_ >= 0);
}

std::string eCAL::eh5::HDF5MeasFileWriterV6::GetFileVersion() const
{
  // UNSUPPORTED FUNCTION
  return "";
}

size_t eCAL::eh5::HDF5MeasFileWriterV6::GetMaxSizePerFile() const
{
  return max_size_per_file_ / 1024 / 1024;
}

void eCAL::eh5::HDF5MeasFileWriterV6::SetMaxSizePerFile(size_t max_file_size_mib)
{
  max_size_per_file_ = max_file_size_mib * 1024 * 1024;
}

bool eCAL::eh5::HDF5MeasFileWriterV6::IsOneFilePerChannelEnabled() const
{
  return false;
}

void eCAL::eh5::HDF5MeasFileWriterV6::SetOneFilePerChannelEnabled(bool /*enabled*/)
{
}

std::set<eCAL::eh5::SChannel> eCAL::eh5::HDF5MeasFileWriterV6::GetChannels() const
{
  // UNSUPPORTED FUNCTION
  return std::set<eCAL::eh5::SChannel>();
}

bool eCAL::eh5::HDF5MeasFileWriterV6::HasChannel(const eCAL::eh5::SChannel& /*channel*/) const
{
  // UNSUPPORTED FUNCTION
  return false;
}

eCAL::eh5::DataTypeInformation eCAL::eh5::HDF5MeasFileWriterV6::GetChannelDataTypeInformation(const SChannel& /*channel*/) const
{
  // UNSUPPORTED FUNCTION
  return eCAL::eh5::DataTypeInformation{};
}

void eCAL::eh5::HDF5MeasFileWriterV6::SetChannelDataTypeInformation(const SChannel& channel , const eCAL::eh5::DataTypeInformation& info)
{
  channels_[channel.name][channel.id].Info = info;
}


long long eCAL::eh5::HDF5MeasFileWriterV6::GetMinTimestamp(const SChannel& /*channel_name*/) const
{
  // UNSUPPORTED FUNCTION
  return -1;
}

long long eCAL::eh5::HDF5MeasFileWriterV6::GetMaxTimestamp(const SChannel&  /*channel_name*/) const
{
  // UNSUPPORTED FUNCTION
  return -1;
}

bool eCAL::eh5::HDF5MeasFileWriterV6::GetEntriesInfo(const SChannel&  /*channel_name*/, EntryInfoSet& /*entries*/) const
{
  // UNSUPPORTED FUNCTION
  return false;
}

bool eCAL::eh5::HDF5MeasFileWriterV6::GetEntriesInfoRange(const SChannel&  /*channel_name*/, long long /*begin*/, long long /*end*/, EntryInfoSet& /*entries*/) const
{
  // UNSUPPORTED FUNCTION
  return false;
}

bool eCAL::eh5::HDF5MeasFileWriterV6::GetEntryDataSize(long long /*entry_id*/, size_t& /*size*/) const
{
  // UNSUPPORTED FUNCTION
  return false;
}

bool eCAL::eh5::HDF5MeasFileWriterV6::GetEntryData(long long /*entry_id*/, void* /*data*/) const
{
  // UNSUPPORTED FUNCTION
  return false;
}

void eCAL::eh5::HDF5MeasFileWriterV6::SetFileBaseName(const std::string& base_name)
{
  base_name_ = base_name;
}

bool eCAL::eh5::HDF5MeasFileWriterV6::AddEntryToFile(const SWriteEntry& entry)
{
  if (!IsOk()) file_id_ = Create();
  if (!IsOk())
    return false;

  hsize_t hsSize = static_cast<hsize_t>(entry.size);

  if (!EntryFitsTheFile(hsSize))
  {
    if (cb_pre_split_ != nullptr)
    {
      cb_pre_split_();
    }

    if (Create() < 0)
      return false;
  }

  //  Create DataSpace with rank 1 and size dimension
  auto dataSpace = H5Screate_simple(1, &hsSize, nullptr);

  //  Create creation property for dataSpace
  auto dsProperty = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_obj_track_times(dsProperty, false);

  //  Create dataset in dataSpace
  auto dataSet = H5Dcreate(file_id_, std::to_string(entries_counter_).c_str(), H5T_NATIVE_UCHAR, dataSpace, H5P_DEFAULT, dsProperty, H5P_DEFAULT);

  //  Write buffer to dataset
  herr_t writeStatus = H5Dwrite(dataSet, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, entry.data);

  //  Close dataset, data space, and data set property
  H5Dclose(dataSet);
  H5Pclose(dsProperty);
  H5Sclose(dataSpace);

  // TODO: check here about id vs channel.id
  channels_[entry.channel.name][entry.channel.id].Entries.emplace_back(SEntryInfo(entry.rcv_timestamp, static_cast<long long>(entries_counter_), entry.clock, entry.snd_timestamp, entry.sender_id));

  entries_counter_++;

  return (writeStatus >= 0);
}

void eCAL::eh5::HDF5MeasFileWriterV6::ConnectPreSplitCallback(CallbackFunction cb)
{
  cb_pre_split_ = cb;
}

void eCAL::eh5::HDF5MeasFileWriterV6::DisconnectPreSplitCallback()
{
  cb_pre_split_ = nullptr;
}

hid_t eCAL::eh5::HDF5MeasFileWriterV6::Create()
{
  if (output_dir_.empty()) return -1;

  if (!EcalUtils::Filesystem::IsDir(output_dir_, EcalUtils::Filesystem::OsStyle::Current)
      && !EcalUtils::Filesystem::MkPath(output_dir_, EcalUtils::Filesystem::OsStyle::Current))
    return -1;

  if (base_name_.empty()) return -1;

  if (IsOk() && !Close()) return -1;

  file_split_counter_++;

  std::string filePath = output_dir_ + "/" + base_name_;

  if (file_split_counter_ > 0)
    filePath += "_" + std::to_string(file_split_counter_);

  filePath += ".hdf5";

  //  create file access property
  hid_t fileAccessPropery = H5Pcreate(H5P_FILE_ACCESS);
  //  create file create property
  hid_t fileCreateProperty = H5Pcreate(H5P_FILE_CREATE);

  //  Create hdf file and get file id
  file_id_ = H5Fcreate(filePath.c_str(), H5F_ACC_TRUNC, fileCreateProperty, fileAccessPropery);

  if (file_id_ >= 0)
    SetAttribute(file_id_, kFileVerAttrTitle, "6.0");
  else
    file_split_counter_--;

  return file_id_;
}

bool eCAL::eh5::HDF5MeasFileWriterV6::EntryFitsTheFile(const hsize_t& size) const
{
  hsize_t fileSize = 0;
  bool status = GetFileSize(fileSize);

  //  check if buffer fits the current file
  return (status && ((fileSize + size) <= max_size_per_file_));
}

bool eCAL::eh5::HDF5MeasFileWriterV6::GetFileSize(hsize_t& size) const
{
  if (!IsOk())
  {
    size = 0;
    return false;
  }
  else
  {
    return H5Fget_filesize(file_id_, &size) >= 0;
  }
}

bool eCAL::eh5::HDF5MeasFileWriterV6::CreateEntriesTableOfContentsFor(const std::string& channelName, std::uint64_t channelId, const DataTypeInformation& channelInfo, const EntryInfoVect& entries) const
{
  if (!IsOk()) return false;

//  const size_t dataSetsSize = entries.size();
//  if (dataSetsSize == 0)  return false;

  std::string hex_id = printHex(channelId);

  // Create a group with the cannel name
  auto group_name_id = OpenOrCreateGroup(file_id_, channelName);
  auto group_id_id = OpenOrCreateGroup(group_name_id, hex_id);

  CreateStringEntryInRoot(file_id_, v6::GetUrl(channelName, hex_id, kChnIdTypename), channelInfo.name);
  CreateStringEntryInRoot(file_id_, v6::GetUrl(channelName, hex_id, kChnIdEncoding),   channelInfo.encoding);
  CreateStringEntryInRoot(file_id_, v6::GetUrl(channelName, hex_id, kChnIdDescriptor), channelInfo.descriptor);
  CreateInformationEntryInRoot(file_id_, v6::GetUrl(channelName, hex_id, kChnIdData), entries);

  H5Gclose(group_name_id);
  H5Gclose(group_id_id);

  return true;
}

