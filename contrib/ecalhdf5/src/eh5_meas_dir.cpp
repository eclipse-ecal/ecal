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
#include "escape.h"
#include "eh5_meas_file_v5.h"

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
#include <ecal_utils/str_convert.h>

unsigned int kDefaultMaxFileSizeMB = 50;

eCAL::eh5::HDF5MeasDir::HDF5MeasDir()
  : cb_pre_split_(nullptr)
  , is_output_dir_created_(false)
  , entries_counter_(0)
  , max_size_per_file_(kDefaultMaxFileSizeMB * 1024 * 1024)
  , access_(RDONLY)
{

}

eCAL::eh5::HDF5MeasDir::HDF5MeasDir(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
  : cb_pre_split_(nullptr)
  , is_output_dir_created_(false)
  , entries_counter_(0)
  , max_size_per_file_(kDefaultMaxFileSizeMB * 1024 * 1024)
  , access_(access)
{
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasDir::Open(path, access);
}

eCAL::eh5::HDF5MeasDir::~HDF5MeasDir()
{
  // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasDir::Close();
}

bool eCAL::eh5::HDF5MeasDir::Open(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
{
  // call the function via its class becase it's a virtual function that is called directly/indirectly in constructor/destructor,-
  // where the vtable is not created yet or it's destructed.
  HDF5MeasDir::Close();

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
    // in case of writre access/recording
    for (auto& channel : channels_)
    {
      channel.second->Close();
      // deallocate the writer instance that is associated to this channel:
      delete channel.second;
      channel.second = nullptr;
    }
    channels_.clear();
    is_output_dir_created_ = false;
  }
  else
  {
    // in case of read access/replaying recordings
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
  }
  return true;
}

bool eCAL::eh5::HDF5MeasDir::IsOk() const
{
  switch (access_)
  {
  case eCAL::eh5::RDONLY:
  //case eCAL::eh5::RDWR:
    return files_.empty() == false && entries_by_id_.empty() == false;
  case eCAL::eh5::CREATE:
    return is_output_dir_created_;
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
  // check if this is a new channel:
  if (channels_.end() == channels_.find(channel_name))
  {
    // new channel, then create a writer instance for it:
    bool is_file_created = CreateFileInstanceForChannel(channel_name);
    if (!is_file_created)
      return ;
  }
  channels_[channel_name]->SetChannelDescription(channel_name, description);
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
  // check if this is a new channel:
  if (channels_.end() == channels_.find(channel_name))
  {
    // new channel, then create a writer instance for it:
    bool is_file_created = CreateFileInstanceForChannel(channel_name);
    if (!is_file_created)
      return;
  }
  channels_[channel_name]->SetChannelType(channel_name, type);
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

bool eCAL::eh5::HDF5MeasDir::AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock, unsigned long long /*entries_counter*/)
{
  if (!IsOk())
  {
    if (!Create())
    {
      return false;
    }
  }
  if (!IsOk())
  {
    return false;
  }
  // check if this is a new channel:
  if (channels_.end() == channels_.find(channel_name))
  {
    // new channel, then create a writer instance for it:
    bool is_file_created = CreateFileInstanceForChannel(channel_name);
    if (!is_file_created)
    {
      return false;
    }
  }

  // add cb function:
  if (cb_pre_split_ != nullptr)
    channels_[channel_name]->ConnectPreSplitCallback(cb_pre_split_);

  bool writeStatus = channels_[channel_name]->AddEntryToFile(data, size, snd_timestamp, rcv_timestamp, channel_name, id, clock, entries_counter_);

  entries_counter_++;

  return writeStatus;
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
  std::wstring dpath_w = EcalUtils::StrConvert::Utf8ToWide(dpath);

  WIN32_FIND_DATAW fd;
  HANDLE hFind = ::FindFirstFileW(dpath_w.c_str(), &fd);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do {
      std::wstring file_name_w(fd.cFileName);
      std::string file_name_utf8 = EcalUtils::StrConvert::WideToUtf8(file_name_w);

      if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
      {
        if (HasHdf5Extension(file_name_utf8) == true)
        {
          paths.push_back(path + "/" + std::string(file_name_utf8.begin(), file_name_utf8.end()));
        }
      }
      else
      {
        if (file_name_utf8 != "." && file_name_utf8 != "..")
        {
          paths.splice(paths.end(), GetHdfFiles(path + "/" + std::string(file_name_utf8.begin(), file_name_utf8.end())));
        }
      }
    } while (::FindNextFileW(hFind, &fd));
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

bool eCAL::eh5::HDF5MeasDir::Create()
{
  if (output_dir_.empty()) return false;

  if (!EcalUtils::Filesystem::IsDir(output_dir_, EcalUtils::Filesystem::OsStyle::Current)
      && !EcalUtils::Filesystem::MkPath(output_dir_, EcalUtils::Filesystem::OsStyle::Current))
    return false;

  if (IsOk() && !Close()) return false;

  is_output_dir_created_ = true;

  return true;
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
        auto escaped_name = GetEscapedString(channel);
        auto description = reader->GetChannelDescription(channel);

        if (channels_info_.find(escaped_name) == channels_info_.end())
        {
          channels_info_[escaped_name] = ChannelInfo(reader->GetChannelType(channel), description);
        }
        else
        {
          if (description.empty() == false)
          {
            channels_info_[escaped_name].description = description;
          }
        }

        channels_info_[escaped_name].files.push_back(reader);

        EntryInfoSet entries;
        if (reader->GetEntriesInfo(channel, entries) == true)
        {
          for (auto entry : entries)
          {
            entries_by_id_[id] = EntryInfo(entry.ID, reader);
            entry.ID = id;
            entries_by_chn_[escaped_name].insert(entry);
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

bool eCAL::eh5::HDF5MeasDir::EntryFitsTheFile(const hsize_t& /*size*/) const
{
  return false;
}

bool eCAL::eh5::HDF5MeasDir::GetFileSize(hsize_t& /*size*/) const
{
  return false;
}

bool eCAL::eh5::HDF5MeasDir::CreateEntriesTableOfContentsFor(const std::string& /*channelName*/, const std::string& /*channelType*/, const std::string& /*channelDescription*/, const EntryInfoVect& /*entries*/)
{
  return false;
}

bool eCAL::eh5::HDF5MeasDir::CreateFileInstanceForChannel(std::string channel_name)
{
  auto hdf_meas_file_ = new HDF5MeasFileV5(output_dir_, channel_name, file_name_, max_size_per_file_, eCAL::eh5::CREATE);

  channels_[channel_name] = hdf_meas_file_;
  return true;
}


