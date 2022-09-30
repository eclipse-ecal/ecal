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

/**
 * @brief  eCALHDF5 directory reader
**/

#include "eh5_meas_file_writer_v5.h"
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

constexpr unsigned int kDefaultMaxFileSizeMB = 1000;

eCAL::eh5::HDF5MeasFileWriterV5::HDF5MeasFileWriterV5()
  : cb_pre_split_      (nullptr)
  , file_id_           (-1)
  , file_split_counter_(-1)
  , entries_counter_   (0)
  , max_size_per_file_ (kDefaultMaxFileSizeMB * 1024 * 1024)
{}

eCAL::eh5::HDF5MeasFileWriterV5::~HDF5MeasFileWriterV5()
{
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasFileWriterV5::Close();
}

bool eCAL::eh5::HDF5MeasFileWriterV5::Open(const std::string& output_dir, eAccessType /*access = eAccessType::RDONLY*/)
{
  Close();

  // Check if the given path points to a directory
  if (!EcalUtils::Filesystem::IsDir(output_dir, EcalUtils::Filesystem::Current))
    return false;

  output_dir_ = output_dir;

  return true;
}

bool eCAL::eh5::HDF5MeasFileWriterV5::Close()
{
  if (!this->IsOk())  return false;

  std::string channels_with_entries;

  for (const auto& channel : channels_)
    if (CreateEntriesTableOfContentsFor(channel.first, channel.second.Type, channel.second.Description, channel.second.Entries))
      channels_with_entries += channel.first + ",";

  if ((!channels_with_entries.empty())  && (channels_with_entries.back() == ','))
    channels_with_entries.pop_back();

  SetAttribute(file_id_, kChnAttrTitle, channels_with_entries);

  for (auto& channel : channels_)
    channel.second.Entries.clear();

  if (H5Fclose(file_id_) >= 0)
  {
    file_id_ = -1;
    return true;
  }
  else
  {
    return false;
  }
}

bool eCAL::eh5::HDF5MeasFileWriterV5::IsOk() const
{
  return (file_id_ >= 0);
}

std::string eCAL::eh5::HDF5MeasFileWriterV5::GetFileVersion() const
{
  // UNSUPPORTED FUNCTION
  return "";
}

size_t eCAL::eh5::HDF5MeasFileWriterV5::GetMaxSizePerFile() const
{
  return max_size_per_file_ / 1024 / 1024;
}

void eCAL::eh5::HDF5MeasFileWriterV5::SetMaxSizePerFile(size_t max_file_size_mib)
{
  max_size_per_file_ = max_file_size_mib * 1024 * 1024;
}

bool eCAL::eh5::HDF5MeasFileWriterV5::IsOneFilePerChannelEnabled() const
{
  return false;
}

void eCAL::eh5::HDF5MeasFileWriterV5::SetOneFilePerChannelEnabled(bool /*enabled*/)
{
}

std::set<std::string> eCAL::eh5::HDF5MeasFileWriterV5::GetChannelNames() const
{
  // UNSUPPORTED FUNCTION
  return {};
}

bool eCAL::eh5::HDF5MeasFileWriterV5::HasChannel(const std::string& /*channel_name*/) const
{
  // UNSUPPORTED FUNCTION
  return false;
}

std::string eCAL::eh5::HDF5MeasFileWriterV5::GetChannelDescription(const std::string& /*channel_name*/) const
{
  // UNSUPPORTED FUNCTION
  return "";
}

void eCAL::eh5::HDF5MeasFileWriterV5::SetChannelDescription(const std::string& channel_name, const std::string& description)
{
  channels_[channel_name].Description = description;
}

std::string eCAL::eh5::HDF5MeasFileWriterV5::GetChannelType(const std::string& /*channel_name*/) const
{
  // UNSUPPORTED FUNCTION
  return "";
}

void eCAL::eh5::HDF5MeasFileWriterV5::SetChannelType(const std::string& channel_name, const std::string& type)
{
  channels_[channel_name].Type = type;
}

long long eCAL::eh5::HDF5MeasFileWriterV5::GetMinTimestamp(const std::string& /*channel_name*/) const
{
  // UNSUPPORTED FUNCTION
  return -1;
}

long long eCAL::eh5::HDF5MeasFileWriterV5::GetMaxTimestamp(const std::string& /*channel_name*/) const
{
  // UNSUPPORTED FUNCTION
  return -1;
}

bool eCAL::eh5::HDF5MeasFileWriterV5::GetEntriesInfo(const std::string& /*channel_name*/, EntryInfoSet& /*entries*/) const
{
  // UNSUPPORTED FUNCTION
  return false;
}

bool eCAL::eh5::HDF5MeasFileWriterV5::GetEntriesInfoRange(const std::string& /*channel_name*/, long long /*begin*/, long long /*end*/, EntryInfoSet& /*entries*/) const
{
  // UNSUPPORTED FUNCTION
  return false;
}

bool eCAL::eh5::HDF5MeasFileWriterV5::GetEntryDataSize(long long /*entry_id*/, size_t& /*size*/) const
{
  // UNSUPPORTED FUNCTION
  return false;
}

bool eCAL::eh5::HDF5MeasFileWriterV5::GetEntryData(long long /*entry_id*/, void* /*data*/) const
{
  // UNSUPPORTED FUNCTION
  return false;
}

void eCAL::eh5::HDF5MeasFileWriterV5::SetFileBaseName(const std::string& base_name)
{
  base_name_ = base_name;
}

bool eCAL::eh5::HDF5MeasFileWriterV5::AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock)
{
  if (!IsOk()) file_id_ = Create();
  if (!IsOk())
    return false;

  hsize_t hsSize = static_cast<hsize_t>(size);

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
  herr_t writeStatus = H5Dwrite(dataSet, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

  //  Close dataset, data space, and data set property
  H5Dclose(dataSet);
  H5Pclose(dsProperty);
  H5Sclose(dataSpace);

  channels_[channel_name].Entries.emplace_back(SEntryInfo(rcv_timestamp, static_cast<long long>(entries_counter_), clock, snd_timestamp, id));

  entries_counter_++;

  return (writeStatus >= 0);
}

void eCAL::eh5::HDF5MeasFileWriterV5::ConnectPreSplitCallback(CallbackFunction cb)
{
  cb_pre_split_ = cb;
}

void eCAL::eh5::HDF5MeasFileWriterV5::DisconnectPreSplitCallback()
{
  cb_pre_split_ = nullptr;
}

hid_t eCAL::eh5::HDF5MeasFileWriterV5::Create()
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
    SetAttribute(file_id_, kFileVerAttrTitle, "5.0");
  else
    file_split_counter_--;

  return file_id_;
}

bool eCAL::eh5::HDF5MeasFileWriterV5::SetAttribute(const hid_t& id, const std::string& name, const std::string& value)
{
  if (id < 0) return false;

  if (H5Aexists(id, name.c_str()) > 0)
    H5Adelete(id, name.c_str());
  //  create scalar dataset
  hid_t scalarDataset = H5Screate(H5S_SCALAR);

  //  create new string data type
  hid_t stringDataType = H5Tcopy(H5T_C_S1);

  //  if attribute's value length exists, allocate space for it
  if (value.length() > 0)
    H5Tset_size(stringDataType, value.length());

  //  create attribute
  hid_t attribute = H5Acreate(id, name.c_str(), stringDataType, scalarDataset, H5P_DEFAULT, H5P_DEFAULT);

  if (attribute < 0) return false;

  //  write attribute value to attribute
  herr_t writeStatus = H5Awrite(attribute, stringDataType, value.c_str());
  if (writeStatus < 0) return false;

  //  close attribute
  H5Aclose(attribute);
  //  close scalar dataset
  H5Sclose(scalarDataset);
  //  close string data type
  H5Tclose(stringDataType);

  return true;
}

bool eCAL::eh5::HDF5MeasFileWriterV5::EntryFitsTheFile(const hsize_t& size) const
{
  hsize_t fileSize = 0;
  bool status = GetFileSize(fileSize);

  //  check if buffer fits the current file
  return (status && ((fileSize + size) <= max_size_per_file_));
}

bool eCAL::eh5::HDF5MeasFileWriterV5::GetFileSize(hsize_t& size) const
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

bool eCAL::eh5::HDF5MeasFileWriterV5::CreateEntriesTableOfContentsFor(const std::string& channelName, const std::string& channelType, const std::string& channelDescription, const EntryInfoVect& entries) const
{
  if (!IsOk()) return false;

  const size_t dataSetsSize = entries.size();

  if (dataSetsSize == 0)  return false;

  hsize_t dims[2] = { dataSetsSize, 5 };

  //  Create DataSpace with rank 2 and size dimension
  auto dataSpace = H5Screate_simple(2, dims, nullptr);

  //  Create creation property for dataSpace
  auto dsProperty = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_obj_track_times(dsProperty, false);

  auto dataSet = H5Dcreate(file_id_, channelName.c_str(), H5T_NATIVE_LLONG, dataSpace, H5P_DEFAULT, dsProperty, H5P_DEFAULT);

  if (dataSet < 0) return false;

  SetAttribute(dataSet, kChnTypeAttrTitle, channelType);
  SetAttribute(dataSet, kChnDescAttrTitle, channelDescription);

  //  Write buffer to dataset
  herr_t writeStatus = H5Dwrite(dataSet, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, entries.data());
  if (writeStatus < 0) return false;

  //  Close dataset, data space, and data set property
  H5Dclose(dataSet);
  H5Pclose(dsProperty);
  H5Sclose(dataSpace);

  return true;
}

