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
 * @brief  eCALHDF5 reader multiple channels implement
**/

#include "eh5_meas_file_v6.h"

#include "hdf5.h"
#include "hdf5_helper.h"

namespace eCAL
{
  namespace eh5
  {

    HDF5MeasFileV6::HDF5MeasFileV6(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
      : HDF5MeasFileV2(path, access)
    {
    }

    HDF5MeasFileV6::HDF5MeasFileV6()
      = default;

    HDF5MeasFileV6::~HDF5MeasFileV6()
      = default;

    std::set<eCAL::eh5::SChannel> HDF5MeasFileV6::GetChannels() const
    {
      std::set<eCAL::eh5::SChannel> channels;

      const auto channel_names = GetChannelNames();

      for (const auto& channel_name : channel_names)
      {
        auto group_id = H5Gopen(file_id_, channel_name.c_str(), H5P_DEFAULT);
        auto groups = ListSubgroups(group_id);
        H5Gclose(group_id);

        for (const auto& group : groups)
        {
          const auto id = parseHexID(group);
          channels.insert({ channel_name, id });
        }

      }

      return channels;
    }

    eCAL::eh5::DataTypeInformation eCAL::eh5::HDF5MeasFileV6::GetChannelDataTypeInformation(const SChannel& channel) const
    {
      std::string type_name;
      std::string type_encoding;
      std::string type_descriptor;

      auto group_id = H5Gopen(file_id_, channel.name.c_str(), H5P_DEFAULT);
      auto groups = ListSubgroups(group_id);
      H5Gclose(group_id);

      if (this->IsOk())
      {
        std::string channel_id_key = printHex(channel.id);

        // Read Typename
        auto type_name_url = v6::GetUrl(channel.name, channel_id_key, eCAL::eh5::kChnIdTypename);
        ReadStringEntryAsString(file_id_, type_name_url, type_name);

        // Read Encoding
        auto type_encoding_url = v6::GetUrl(channel.name, channel_id_key, eCAL::eh5::kChnIdEncoding);
        ReadStringEntryAsString(file_id_, type_encoding_url, type_encoding);

        // Read Descriptor
        auto type_descriptor_url = v6::GetUrl(channel.name, channel_id_key, eCAL::eh5::kChnIdDescriptor);
        ReadStringEntryAsString(file_id_, type_descriptor_url, type_descriptor);
      }

      return eCAL::eh5::DataTypeInformation{ type_name, type_encoding, type_descriptor };
    }

    bool eCAL::eh5::HDF5MeasFileV6::GetEntriesInfo(const SChannel& channel, EntryInfoSet& entries) const
     {
      if (!this->IsOk()) return false;

      auto hex_id = printHex(channel.id);
      EntryInfoSet channel_id_entries;
      auto url = v6::GetUrl(channel.name, hex_id, kChnIdData);
      GetEntryInfoVector(file_id_, url, entries);

      return true;
    }
  }  //  namespace eh5
}  //  namespace eCAL
