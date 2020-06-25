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

#include "eh5_meas_dir.h"

#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif //WIN32

#include <string.h>
#include <string>
#include <list>
#include <iostream>

#include <ecal_utils/filesystem.h>

unsigned int kDefaultMaxFileSizeMB = 50;

eCAL::eh5::HDF5MeasDir::HDF5MeasDir()
  : cb_pre_split_(nullptr)
  , file_id_(-1)
  , file_split_counter_(-1)
  , entries_counter_(0)
  , max_size_per_file_(kDefaultMaxFileSizeMB * 1024 * 1024)
  , access_(RDONLY)
{

}

eCAL::eh5::HDF5MeasDir::HDF5MeasDir(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
  : cb_pre_split_(nullptr)
  , file_id_(-1)
  , file_split_counter_(-1)
  , entries_counter_(0)
  , max_size_per_file_(kDefaultMaxFileSizeMB * 1024 * 1024)
{
  Open(path, access);
}

eCAL::eh5::HDF5MeasDir::~HDF5MeasDir()
{
  Close();
}

bool eCAL::eh5::HDF5MeasDir::Open(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
{
  Close();

  access_ = access;

  switch (access)
  {
  case eCAL::eh5::RDONLY:
  //case eCAL::eh5::RDWR:
    return OpenRX(path, access);
  case eCAL::eh5::CREATE:
    output_dir_ = path;
    return true;
  default:
    break;
  }

  return false;
}

bool eCAL::eh5::HDF5MeasDir::Close()
{
  if (access_ == eAccessType::CREATE)
  {
    if (!this->IsOk())  return false;

    std::string channels_with_entries;

    for (const auto& channel : channels_)
      if (CreateEntriesTableOfContentsFor(channel.first, channel.second.Type, channel.second.Description, channel.second.Entries))
        channels_with_entries += channel.first + ",";

    if ((channels_with_entries.size() > 0)  && (channels_with_entries.back() == ','))
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
  else
  {
    for (auto file : files_)
    {
      if (file != nullptr)
      {
        file->Close();
        delete file;
        file = nullptr;
      }
    }

    files_.clear();
    channels_info_.clear();
    entries_by_id_.clear();
    entries_by_chn_.clear();

    return true;
  }
}

bool eCAL::eh5::HDF5MeasDir::IsOk() const
{
  switch (access_)
  {
  case eCAL::eh5::RDONLY:
  //case eCAL::eh5::RDWR:
    return files_.empty() == false && entries_by_id_.empty() == false;
  case eCAL::eh5::CREATE:
    return (file_id_ >= 0);
  default:
    return false;
  }
}

std::string eCAL::eh5::HDF5MeasDir::GetFileVersion() const
{
  std::string version;
  if (files_.empty() == false)
  {
    version = files_.front()->GetFileVersion();
  }
  return version;
}

size_t eCAL::eh5::HDF5MeasDir::GetMaxSizePerFile() const
{
  return max_size_per_file_ / 1024 / 1024;
}

void eCAL::eh5::HDF5MeasDir::SetMaxSizePerFile(size_t size)
{
  max_size_per_file_ = size * 1024 * 1024;
}

std::set<std::string> eCAL::eh5::HDF5MeasDir::GetChannelNames() const
{
  std::set<std::string> channels;
  for (const auto& chn : channels_info_)
    channels.insert(chn.first);

  return channels;
}

bool eCAL::eh5::HDF5MeasDir::HasChannel(const std::string& channel_name) const
{
  return channels_info_.count(channel_name) != 0;
}

std::string eCAL::eh5::HDF5MeasDir::GetChannelDescription(const std::string& channel_name) const
{
  std::string ret_val;

  const auto& found = channels_info_.find(channel_name);

  if (found != channels_info_.end())
  {
    ret_val = found->second.description;
  }
  return ret_val;
}

void eCAL::eh5::HDF5MeasDir::SetChannelDescription(const std::string& channel_name, const std::string& description)
{
  channels_[channel_name].Description = description;
}

std::string eCAL::eh5::HDF5MeasDir::GetChannelType(const std::string& channel_name) const
{
  std::string ret_val;

  const auto& found = channels_info_.find(channel_name);

  if (found != channels_info_.end())
  {
    ret_val = found->second.type;
  }
  return ret_val;
}

void eCAL::eh5::HDF5MeasDir::SetChannelType(const std::string& channel_name, const std::string& type)
{
  channels_[channel_name].Type = type;
}

long long eCAL::eh5::HDF5MeasDir::GetMinTimestamp(const std::string& channel_name) const
{
  long long ret_val = 0;

  const auto& found = entries_by_chn_.find(channel_name);

  if (found != entries_by_chn_.end())
  {
    if (found->second.empty() == false)
    {
      ret_val = found->second.begin()->RcvTimestamp;
    }
  }

  return ret_val;
}

long long eCAL::eh5::HDF5MeasDir::GetMaxTimestamp(const std::string& channel_name) const
{
  long long ret_val = 0;

  const auto& found = entries_by_chn_.find(channel_name);

  if (found != entries_by_chn_.end())
  {
    if (found->second.empty() == false)
    {
      ret_val = found->second.rbegin()->RcvTimestamp;
    }
  }

  return ret_val;
}

bool eCAL::eh5::HDF5MeasDir::GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const
{
  entries.clear();

  const auto& found = entries_by_chn_.find(channel_name);

  if (found != entries_by_chn_.end())
  {
    entries = found->second;
  }

  return entries.empty() == false;
}

bool eCAL::eh5::HDF5MeasDir::GetEntriesInfoRange(const std::string& channel_name, long long begin, long long end, EntryInfoSet& entries) const
{
  bool ret_val = false;

  entries.clear();

  const auto& found = entries_by_chn_.find(channel_name);

  if (found != entries_by_chn_.end())
  {
    if (begin == 0) begin = entries.begin()->RcvTimestamp;
    if (end == 0) end = entries.rbegin()->RcvTimestamp;

    const auto& lower = found->second.lower_bound(SEntryInfo(begin, 0, 0));
    const auto& upper = found->second.upper_bound(SEntryInfo(end, 0, 0));

    entries.insert(lower, upper);
    ret_val = true;
  }

  return ret_val;
}

bool eCAL::eh5::HDF5MeasDir::GetEntryDataSize(long long entry_id, size_t& size) const
{
  auto ret_val = false;
  const auto& found = entries_by_id_.find(entry_id);
  if (found != entries_by_id_.end())
  {
    ret_val = found->second.reader->GetEntryDataSize(found->second.file_id, size);
  }
  return ret_val;
}

bool eCAL::eh5::HDF5MeasDir::GetEntryData(long long entry_id, void* data) const
{
  auto ret_val = false;
  const auto& found = entries_by_id_.find(entry_id);
  if (found != entries_by_id_.end())
  {
    ret_val = found->second.reader->GetEntryData(found->second.file_id, data);
  }
  return ret_val;
}

void eCAL::eh5::HDF5MeasDir::SetFileBaseName(const std::string& base_name)
{
  file_name_ = base_name;
}

bool eCAL::eh5::HDF5MeasDir::AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock)
{
  if (!IsOk()) file_id_ = Create();
  if (!IsOk())
    return false;

  hsize_t hsSize = static_cast<hsize_t>(size);

  if (EntryFitsTheFile(hsSize) == false)
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

  //  Create dataset in dataSpace
  auto dataSet = H5Dcreate(file_id_, std::to_string(entries_counter_).c_str(), H5T_NATIVE_UCHAR, dataSpace, H5P_DEFAULT, dsProperty, H5P_DEFAULT);

  //  Write buffer to dataset
  herr_t writeStatus = H5Dwrite(dataSet, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

  //  Close dataset, data space, and data set property
  H5Dclose(dataSet);
  H5Pclose(dsProperty);
  H5Sclose(dataSpace);

  channels_[channel_name].Entries.emplace_back(SEntryInfo(rcv_timestamp, entries_counter_, clock, snd_timestamp, id));

  entries_counter_++;

  return (writeStatus >= 0);
}

void eCAL::eh5::HDF5MeasDir::ConnectPreSplitCallback(CallbackFunction cb)
{
  cb_pre_split_ = cb;
}

void eCAL::eh5::HDF5MeasDir::DisconnectPreSplitCallback()
{
  cb_pre_split_ = nullptr;
}

std::list<std::string> eCAL::eh5::HDF5MeasDir::GetHdfFiles(const std::string& path) const
{
  std::list<std::string> paths;
#ifdef WIN32
  std::string dpath = path + "/*.*";

  WIN32_FIND_DATAA fd;
  HANDLE hFind = ::FindFirstFileA(dpath.c_str(), &fd);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do {
      std::string file_name_w(fd.cFileName);

      if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
      {
        if (HasHdf5Extension(file_name_w) == true)
        {
          paths.push_back(path + "/" + std::string(file_name_w.begin(), file_name_w.end()));
        }
      }
      else
      {
        if (file_name_w != "." && file_name_w != "..")
        {
          paths.splice(paths.end(), GetHdfFiles(path + "/" + std::string(file_name_w.begin(), file_name_w.end())));
        }
      }
    } while (::FindNextFileA(hFind, &fd));
    ::FindClose(hFind);
  }
#else
  struct dirent* de = nullptr;
  DIR* dir = nullptr;

  dir = opendir(path.c_str());

  if (dir != nullptr)
  {
    de = readdir(dir);
    while (de != nullptr)
    {
      std::string d_name = de->d_name;
      if (d_name != "." && d_name != "..")
      {
        if (de->d_type == DT_DIR)
        {
          paths.splice(paths.end(), GetHdfFiles(path + "/" + d_name));
        }
        else
        {
          if (HasHdf5Extension(d_name) == true)
            paths.push_back(path + "/" + d_name);
        }
      }
      de = readdir(dir);
    }
    closedir(dir);
}
#endif  //  WIN32
  return paths;
}

hid_t eCAL::eh5::HDF5MeasDir::Create()
{
  if (output_dir_.empty()) return -1;

  if (!EcalUtils::Filesystem::IsDir(output_dir_, EcalUtils::Filesystem::OsStyle::Current)
      && !EcalUtils::Filesystem::MkPath(output_dir_, EcalUtils::Filesystem::OsStyle::Current))
    return -1;

  if (file_name_.empty()) return -1;

  if (IsOk() && !Close()) return -1;

  file_split_counter_++;

  std::string filePath = output_dir_ + "/" + file_name_;

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

bool eCAL::eh5::HDF5MeasDir::OpenRX(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
{
  if (access != eAccessType::RDONLY /*&& access != eAccessType::RDWR*/) return false;

  auto files = GetHdfFiles(path);

  long long id = 0;

  for (const auto& file_path : files)
  {
    auto reader = new eCAL::eh5::HDF5Meas(file_path);

    if (reader->IsOk() == true)
    {
      auto channels = reader->GetChannelNames();
      for (const auto& channel : channels)
      {
        auto description = reader->GetChannelDescription(channel);

        if (channels_info_.find(channel) == channels_info_.end())
        {
          channels_info_[channel] = ChannelInfo(reader->GetChannelType(channel), description);
        }
        else
        {
          if (description.empty() == false)
          {
            channels_info_[channel].description = description;
          }
        }

        channels_info_[channel].files.push_back(reader);

        EntryInfoSet entries;
        if (reader->GetEntriesInfo(channel, entries) == true)
        {
          for (auto entry : entries)
          {
            entries_by_id_[id] = EntryInfo(entry.ID, reader);
            entry.ID = id;
            entries_by_chn_[channel].insert(entry);
            id++;
          }
        }
      }
      files_.push_back(reader);
    }
    else
    {
      reader->Close();
      delete reader;
      reader = nullptr;
    }
  }
  return files_.empty() == false;
}

bool eCAL::eh5::HDF5MeasDir::SetAttribute(const hid_t& id, const std::string& name, const std::string& value)
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

bool eCAL::eh5::HDF5MeasDir::EntryFitsTheFile(const hsize_t& size) const
{
  hsize_t fileSize = 0;
  herr_t status = GetFileSize(fileSize);

  //  check if buffer fits the current file
  return (status && ((fileSize + size) <= max_size_per_file_));
}

bool eCAL::eh5::HDF5MeasDir::GetFileSize(hsize_t& size) const
{
  size = 0;

  if (!IsOk()) return false;

  return H5Fget_filesize(file_id_, &size) >= 0;
}

bool eCAL::eh5::HDF5MeasDir::CreateEntriesTableOfContentsFor(const std::string& channelName, const std::string& channelType, const std::string& channelDescription, const EntryInfoVect& entries)
{
  if (!IsOk()) return false;

  const size_t dataSetsSize = entries.size();

  if (dataSetsSize == 0)  return false;

  hsize_t dims[2] = { dataSetsSize, 5 };

  //  Create DataSpace with rank 2 and size dimension
  auto dataSpace = H5Screate_simple(2, dims, nullptr);

  //  Create creation property for dataSpace
  auto dsProperty = H5Pcreate(H5P_DATASET_CREATE);

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

