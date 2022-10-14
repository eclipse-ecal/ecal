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
 * @brief  eCALHDF5 measurement class <TODO>
**/

#include <ecalhdf5/eh5_meas.h>

#include <ecal_utils/filesystem.h>

#include <utility>

#include "eh5_meas_dir.h"
#include "eh5_meas_file_v1.h"
#include "eh5_meas_file_v2.h"
#include "eh5_meas_file_v3.h"
#include "eh5_meas_file_v4.h"
#include "eh5_meas_file_v5.h"

#include "escape.h"

namespace
{
  const double file_version_max(5.0);
}

eCAL::eh5::HDF5Meas::HDF5Meas()
= default;

eCAL::eh5::HDF5Meas::HDF5Meas(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
{
  Open(path, access);
}

eCAL::eh5::HDF5Meas::~HDF5Meas()
= default;

bool eCAL::eh5::HDF5Meas::Open(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
{
  if (hdf_meas_impl_)
  {
    Close();
  }

  if (access == eAccessType::CREATE)
  {
    EcalUtils::Filesystem::MkPath(path, EcalUtils::Filesystem::OsStyle::Current);
  }

  auto type = EcalUtils::Filesystem::GetType(path, EcalUtils::Filesystem::OsStyle::Current);

  switch (type)
  {
  case EcalUtils::Filesystem::Dir:
  {
    hdf_meas_impl_ = std::make_unique<HDF5MeasDir>(path, access);
    break;
  }
  case EcalUtils::Filesystem::RegularFile:
  {
    hdf_meas_impl_ = std::make_unique<HDF5MeasFileV5>(path, access);

    if (!hdf_meas_impl_->IsOk())
    {
      hdf_meas_impl_.reset();
      return false;
    }

    // no file version at all ?
    auto file_version = GetFileVersion();
    if (file_version.empty())
    {
      hdf_meas_impl_.reset();
      return false;
    }

    // future file version ?
    auto file_version_numeric = stod(file_version);
    if (file_version_numeric > file_version_max)
    {
      hdf_meas_impl_.reset();
      return false;
    }

    // check if we need to create an older file reader
    if (file_version_numeric < 1.0 || file_version_numeric > file_version_max)
    {
      hdf_meas_impl_.reset();
    }
    else if (file_version_numeric >= 1.0 && file_version_numeric < 2.0)
    {
      hdf_meas_impl_ = std::make_unique<HDF5MeasFileV1>(path, access);
    }
    else if (file_version_numeric >= 2.0 && file_version_numeric < 3.0)
    {
      hdf_meas_impl_ = std::make_unique<HDF5MeasFileV2>(path, access);
    }
    else if (file_version_numeric >= 3.0 && file_version_numeric < 4.0)
    {
      hdf_meas_impl_ = std::make_unique<HDF5MeasFileV3>(path, access);
    }
    else if (file_version_numeric >= 4.0 && file_version_numeric < 5.0)
    {
      hdf_meas_impl_ = std::make_unique<HDF5MeasFileV4>(path, access);
    }
  }
  break;
  case EcalUtils::Filesystem::Unknown:
  default:
    return false;
    break;
  }

  if (access == eAccessType::CREATE)
  {
    return hdf_meas_impl_ ? EcalUtils::Filesystem::IsDir(path, EcalUtils::Filesystem::OsStyle::Current) : false;
  }
  else
  {
    return hdf_meas_impl_ ? hdf_meas_impl_->IsOk() : false;
  }
}

bool eCAL::eh5::HDF5Meas::Close()
{
  bool ret_val = false;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->Close();
  }

  return ret_val;
}

bool eCAL::eh5::HDF5Meas::IsOk() const
{
  bool ret_val = false;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->IsOk();
  }

  return ret_val;
}

std::string eCAL::eh5::HDF5Meas::GetFileVersion() const
{
  std::string ret_val;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->GetFileVersion();
  }

  return ret_val;
}

size_t eCAL::eh5::HDF5Meas::GetMaxSizePerFile() const
{
  size_t ret_val = 0;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->GetMaxSizePerFile();
  }

  return ret_val;
}

void eCAL::eh5::HDF5Meas::SetMaxSizePerFile(size_t size)
{
  if (hdf_meas_impl_)
  {
    hdf_meas_impl_->SetMaxSizePerFile(size);
  }
}

bool eCAL::eh5::HDF5Meas::IsOneFilePerChannelEnabled() const
{
  if (hdf_meas_impl_ != nullptr)
  {
    return hdf_meas_impl_->IsOneFilePerChannelEnabled();
  }
  return false;
}

void eCAL::eh5::HDF5Meas::SetOneFilePerChannelEnabled(bool enabled)
{
  if (hdf_meas_impl_ != nullptr)
  {
    hdf_meas_impl_->SetOneFilePerChannelEnabled(enabled);
  }
}

std::set<std::string> eCAL::eh5::HDF5Meas::GetChannelNames() const
{
  std::set<std::string> ret_val;
  if (hdf_meas_impl_)
  {
    std::set<std::string> escaped_channel_names = hdf_meas_impl_->GetChannelNames();
    for (const std::string& escaped_name : escaped_channel_names)
    {
      ret_val.emplace(GetUnescapedString(escaped_name));
    }
  }

  return ret_val;
}

bool eCAL::eh5::HDF5Meas::HasChannel(const std::string& channel_name) const
{
  bool ret_val = false;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->HasChannel(GetEscapedTopicname(channel_name));
  }

  return ret_val;
}

std::string eCAL::eh5::HDF5Meas::GetChannelDescription(const std::string& channel_name) const
{
  std::string ret_val;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->GetChannelDescription(GetEscapedTopicname(channel_name));
  }

  return ret_val;
}

void eCAL::eh5::HDF5Meas::SetChannelDescription(const std::string& channel_name, const std::string& description)
{
  if (hdf_meas_impl_)
  {
    hdf_meas_impl_->SetChannelDescription(GetEscapedTopicname(channel_name), description);
  }
}

std::string eCAL::eh5::HDF5Meas::GetChannelType(const std::string& channel_name) const
{
  std::string ret_val;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->GetChannelType(GetEscapedTopicname(channel_name));
  }

  return ret_val;
}

void eCAL::eh5::HDF5Meas::SetChannelType(const std::string& channel_name, const std::string& type)
{
  if (hdf_meas_impl_)
  {
    hdf_meas_impl_->SetChannelType(GetEscapedTopicname(channel_name), type);
  }
}

long long eCAL::eh5::HDF5Meas::GetMinTimestamp(const std::string& channel_name) const
{
  long long ret_val = 0;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->GetMinTimestamp(GetEscapedTopicname(channel_name));
  }

  return ret_val;
}

long long eCAL::eh5::HDF5Meas::GetMaxTimestamp(const std::string& channel_name) const
{
  long long ret_val = 0;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->GetMaxTimestamp(GetEscapedTopicname(channel_name));
  }

  return ret_val;
}

bool eCAL::eh5::HDF5Meas::GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const
{
  bool ret_val = false;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->GetEntriesInfo(GetEscapedTopicname(channel_name), entries);
  }

  return ret_val;
}

bool eCAL::eh5::HDF5Meas::GetEntriesInfoRange(const std::string& channel_name, long long begin, long long end, EntryInfoSet& entries) const
{
  bool ret_val = false;
  if (hdf_meas_impl_ && begin < end)
  {
    ret_val = hdf_meas_impl_->GetEntriesInfoRange(GetEscapedTopicname(channel_name), begin, end, entries);
  }

  return ret_val;
}

bool eCAL::eh5::HDF5Meas::GetEntryDataSize(long long entry_id, size_t& size) const
{
  bool ret_val = false;
  if (hdf_meas_impl_)
  {
    ret_val = hdf_meas_impl_->GetEntryDataSize(entry_id, size);
  }

  return ret_val;
}

bool eCAL::eh5::HDF5Meas::GetEntryData(long long entry_id, void* data) const
{
  bool ret_val = false;
  if (hdf_meas_impl_)
  {
    return hdf_meas_impl_->GetEntryData(entry_id, data);
  }

  return ret_val;
}

void eCAL::eh5::HDF5Meas::SetFileBaseName(const std::string& base_name)
{
  if (hdf_meas_impl_)
  {
    return hdf_meas_impl_->SetFileBaseName(base_name);
  }
}

bool eCAL::eh5::HDF5Meas::AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock)
{
  bool ret_val = false;
  if (hdf_meas_impl_)
  {
    return hdf_meas_impl_->AddEntryToFile(data, size, snd_timestamp, rcv_timestamp, GetEscapedTopicname(channel_name), id, clock);
  }

  return ret_val;
}

void eCAL::eh5::HDF5Meas::ConnectPreSplitCallback(CallbackFunction cb)
{
  if (hdf_meas_impl_)
  {
    return hdf_meas_impl_->ConnectPreSplitCallback(std::move(cb));
  }
}

void eCAL::eh5::HDF5Meas::DisconnectPreSplitCallback()
{
  if (hdf_meas_impl_)
  {
    return hdf_meas_impl_->DisconnectPreSplitCallback();
  }
}
