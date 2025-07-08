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

/**
 * @brief  eCALHDF5 measurement file version V1
**/

#include "eh5_meas_file_v1.h"

#include "hdf5.h"
#include <ecal_utils/string.h>
#include "datatype_helper.h"

#include <iostream>

eCAL::eh5::HDF5MeasFileV1::HDF5MeasFileV1()
  : file_id_(-1)
{
#ifndef _DEBUG
  H5Eset_auto(0, nullptr, nullptr);
#endif  //  _DEBUG
}


eCAL::eh5::HDF5MeasFileV1::HDF5MeasFileV1(const std::string& path, v3::eAccessType access /*= eAccessType::RDONLY*/)
  : file_id_(-1)
{
#ifndef _DEBUG
  H5Eset_auto(0, nullptr, nullptr);
#endif  //  _DEBUG

  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasFileV1::Open(path, access);
}

eCAL::eh5::HDF5MeasFileV1::~HDF5MeasFileV1()
{
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasFileV1::Close();
  entries_.clear();
}


bool eCAL::eh5::HDF5MeasFileV1::Open(const std::string& path, v3::eAccessType access /*= eAccessType::RDONLY*/)
{
  entries_.clear();

  if (path.empty())
    return false;

  if (file_id_ > 0)
    HDF5MeasFileV1::Close();

  if (access != v3::eAccessType::RDONLY)
  {
    ReportUnsupportedAction();
    return false;
  }

  file_id_ = H5Fopen(path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

  if (HDF5MeasFileV1::IsOk())
  {
    auto channels = HDF5MeasFileV1::GetChannels();
    if (channels.size() == 1)
    {
      auto channel = *channels.begin();
      HDF5MeasFileV1::GetEntriesInfo(channel, entries_);
    }
  }

  // call the function via its class becase it's a virtual function that is called directly/indirectly in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  return HDF5MeasFileV1::IsOk();
}


bool eCAL::eh5::HDF5MeasFileV1::Close()
{
  if (HDF5MeasFileV1::IsOk() && H5Fclose(file_id_) >= 0)
  {
    file_id_ = -1;
    return true;
  }
  else
  {
    return false;
  }
}


bool eCAL::eh5::HDF5MeasFileV1::IsOk() const
{
  return (file_id_ >= 0);
}


std::string eCAL::eh5::HDF5MeasFileV1::GetFileVersion() const
{
  std::string file_version;
  GetAttributeValue(file_id_, kFileVerAttrTitle, file_version);
  return file_version;
}


size_t eCAL::eh5::HDF5MeasFileV1::GetMaxSizePerFile() const
{
  ReportUnsupportedAction();
  return 0;
}


void eCAL::eh5::HDF5MeasFileV1::SetMaxSizePerFile(size_t /*size*/)
{
  ReportUnsupportedAction();
}

bool eCAL::eh5::HDF5MeasFileV1::IsOneFilePerChannelEnabled() const
{
  ReportUnsupportedAction();
  return false;
}

void eCAL::eh5::HDF5MeasFileV1::SetOneFilePerChannelEnabled(bool /*enabled*/)
{
  ReportUnsupportedAction();
}

std::set<eCAL::eh5::SEscapedChannel> eCAL::eh5::HDF5MeasFileV1::GetChannels() const
{
  std::set<eCAL::eh5::SEscapedChannel> channels;

  std::string channel_name;
  GetAttributeValue(file_id_, kChnNameAttribTitle, channel_name);

  if (!channel_name.empty())
    channels.insert({ channel_name, 0 });

  return channels;
}

bool eCAL::eh5::HDF5MeasFileV1::HasChannel(const eCAL::eh5::SEscapedChannel& channel) const
{
  auto channels = GetChannels();

  return std::find(channels.cbegin(), channels.cend(), channel) != channels.end();
}

eCAL::eh5::DataTypeInformation eCAL::eh5::HDF5MeasFileV1::GetChannelDataTypeInformation(const SEscapedChannel& channel) const
{
  std::string type;

  if (EcalUtils::String::Icompare(channel.name, channel_name_))
    GetAttributeValue(file_id_, kChnTypeAttrTitle, type);

  std::string description;

  if (EcalUtils::String::Icompare(channel.name, channel_name_))
    GetAttributeValue(file_id_, kChnDescAttrTitle, description);

  return CreateInfo(type, description);
}

void eCAL::eh5::HDF5MeasFileV1::SetChannelDataTypeInformation(const SEscapedChannel& /*channel*/, const eCAL::eh5::DataTypeInformation& /*info*/)
{
  ReportUnsupportedAction();
}

long long eCAL::eh5::HDF5MeasFileV1::GetMinTimestamp(const SEscapedChannel& /*channel_name*/) const
{
  long long ret_val = 0;

  if (!entries_.empty())
  {
    ret_val = entries_.begin()->RcvTimestamp;
  }

  return ret_val;
}

long long eCAL::eh5::HDF5MeasFileV1::GetMaxTimestamp(const SEscapedChannel& /*channel_name*/) const
{
  long long ret_val = 0;

  if (!entries_.empty())
  {
    ret_val = entries_.rbegin()->RcvTimestamp;
  }

  return ret_val;
}

bool eCAL::eh5::HDF5MeasFileV1::GetEntriesInfo(const SEscapedChannel& channel, EntryInfoSet& entries) const
{
  entries.clear();

  if (!EcalUtils::String::Icompare(channel.name, channel_name_)) return false;

  if (!HDF5MeasFileV1::IsOk()) return false;

  auto dataset_id = H5Dopen(file_id_, kTimestampAttrTitle.c_str(), H5P_DEFAULT);

  if (dataset_id < 0) return false;

  const size_t size_of_ll = sizeof(long long);
  hsize_t data_size = H5Dget_storage_size(dataset_id) / size_of_ll;

  if (data_size <= 0) return false;

  long long* data = static_cast<long long*>(calloc(static_cast<size_t>(data_size), size_of_ll));

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

bool eCAL::eh5::HDF5MeasFileV1::GetEntriesInfoRange(const SEscapedChannel& /*channel_name*/, long long begin, long long end, EntryInfoSet& entries) const
{
  bool ret_val = false;

  if (!entries_.empty())
  {
    if (begin == 0) begin = entries.begin()->RcvTimestamp;
    if (end == 0) end = entries.rbegin()->RcvTimestamp;

    const auto& lower = entries_.lower_bound(SEntryInfo(begin, 0, 0));
    const auto& upper = entries_.upper_bound(SEntryInfo(end, 0, 0));

    entries.insert(lower, upper);
    ret_val = true;
  }

  return ret_val;
}

bool eCAL::eh5::HDF5MeasFileV1::GetEntryDataSize(long long entry_id, size_t& size) const
{
  if (!this->IsOk()) return false;

  auto dataset_id = H5Dopen(file_id_, std::to_string(entry_id).c_str(), H5P_DEFAULT);

  if (dataset_id < 0) return false;

  size = static_cast<size_t>(H5Dget_storage_size(dataset_id));

  H5Dclose(dataset_id);

  return true;
}

bool eCAL::eh5::HDF5MeasFileV1::GetEntryData(long long entry_id, void* data) const
{
  if (data == nullptr) return false;

  if (!this->IsOk()) return false;

  auto dataset_id = H5Dopen(file_id_, std::to_string(entry_id).c_str(), H5P_DEFAULT);

  if (dataset_id < 0) return false;

  auto size = H5Dget_storage_size(dataset_id);

  if (size <= 0) return false;

  auto readStatus = H5Dread(dataset_id, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

  H5Dclose(dataset_id);

  return (readStatus >= 0);
}


void eCAL::eh5::HDF5MeasFileV1::SetFileBaseName(const std::string& /*base_name*/)
{
  ReportUnsupportedAction();
}

bool eCAL::eh5::HDF5MeasFileV1::AddEntryToFile(const SEscapedWriteEntry& /*entry*/)
{
  ReportUnsupportedAction();
  return false;
}

void eCAL::eh5::HDF5MeasFileV1::ConnectPreSplitCallback(CallbackFunction /*cb*/)
{
  ReportUnsupportedAction();
}

void eCAL::eh5::HDF5MeasFileV1::DisconnectPreSplitCallback()
{
  ReportUnsupportedAction();
}

bool eCAL::eh5::HDF5MeasFileV1::GetAttributeValue(hid_t obj_id, const std::string& name, std::string& value) 
{
  bool ret_val = false;
  //  empty attribute value
  value.clear();
  if (obj_id < 0) return false;

  //  check if attribute exists
  if (H5Aexists(obj_id, name.c_str()) != 0)
  {
    //  open attribute by name, getting the attribute index
    hid_t attr_id = H5Aopen_name(obj_id, name.c_str());
    //  fail - attribute can not be opened
    if (attr_id <= 0) return false;

    //  get attribute type
    hid_t attr_type = H5Aget_type(attr_id);
    //  get type class based on attribute type
    H5T_class_t type_class = H5Tget_class(attr_type);
    //  get attribute content dataSize
    const size_t attr_size = H5Tget_size(attr_type);

    //  if attribute class is string
    if (type_class == H5T_STRING)
    {
      hid_t attr_type_mem = H5Tget_native_type(attr_type, H5T_DIR_ASCEND);
      //  create buffer to store the value of the attribute
      char* content_buffer = new char[attr_size];
      //  get attribute value
      ret_val = (H5Aread(attr_id, attr_type_mem, content_buffer) >= 0);

      //  convert value to std string
      value = std::string(content_buffer, attr_size);

      //  free buffer
      delete[]content_buffer;
    }
    else
    {
      //  fail - attribute is not string type
      ret_val = false;
    }
    //  close attribute
    H5Aclose(attr_id);
  }
  else
  {
    //  fail - attribute name does not exist
    ret_val = false;
  }
  //  return read status
  return ret_val;
}

void eCAL::eh5::HDF5MeasFileV1::ReportUnsupportedAction() 
{
  std::cout << "eCALHDF5 file version below 2.0 support only readonly access type. Desired action not supported.\n";
}
