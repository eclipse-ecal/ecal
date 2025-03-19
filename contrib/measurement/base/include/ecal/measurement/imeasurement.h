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

#pragma once

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <stdexcept>

#include <ecal/measurement/hdf5/reader.h>
#include <ecal/measurement/base/types.h>
#include <ecal/measurement/measurement.h>

namespace eCAL
{
  namespace measurement
  {
    class IBinaryChannel
    {
    public:
      IBinaryChannel(std::shared_ptr<experimental::measurement::base::Reader> meas_, experimental::measurement::base::Channel channel_)
        : meas(meas_)
        , channel(channel_)
        , datatype_info(meas->GetChannelDataTypeInformation(channel))
      {
        meas->GetEntriesInfo(channel, entry_infos);
      }

      ~IBinaryChannel() = default;

      IBinaryChannel(const IBinaryChannel&) = delete;
      IBinaryChannel& operator=(const IBinaryChannel&) = delete;

      IBinaryChannel(IBinaryChannel&& rhs) = default;
      IBinaryChannel& operator=(IBinaryChannel&& rhs) = default;

      virtual BinaryFrame operator[](const experimental::measurement::base::EntryInfo& entry)
      {
        size_t message_size;
        meas->GetEntryDataSize(entry.ID, message_size);
        data.resize(message_size);
        meas->GetEntryData(entry.ID, (void*)data.data());
        return make_frame( data, entry.SndTimestamp, entry.RcvTimestamp );
      }

      const std::string& GetName() const
      {
        return channel.name;
      }

      // We read the data upon request! (directly from the measurement)
      const eCAL::experimental::measurement::base::DataTypeInformation& GetDatatypeInformation() const
      {
        return datatype_info;
      }

      class iterator /*: public std::iterator<std::forward_iterator_tag, Entry<T>>*/
      {
      public:
        iterator(IBinaryChannel& owner_)
          : m_owner(owner_)
          , m_entry_iterator(owner_.entry_infos.begin())
        {};

        iterator(IBinaryChannel& owner_, const experimental::measurement::base::EntryInfoSet::iterator& it)
          : m_owner(owner_)
          , m_entry_iterator(it)
        {};

        iterator(const iterator& i)
          : m_owner(i.m_owner)
          , m_entry_iterator(i.m_entry_iterator)
          , m_msg(i.m_msg)
        {};
        ~iterator()
        {};
        iterator& operator++()
        {
          ++m_entry_iterator;
          return *this;
        }; //prefix increment
        iterator& operator--()
        {
          --m_entry_iterator;
          return *this;
        }; //prefix decrement
           //reference operator*() const
        BinaryFrame operator*() const
        {
          return m_owner[*m_entry_iterator];
        };
        //friend void swap(iterator& lhs, iterator& rhs); //C++11 I think
        bool operator==(const iterator& rhs) const { return m_owner == rhs.m_owner && m_entry_iterator == rhs.m_entry_iterator; /*return it == rhs.it; */ };
        bool operator!=(const iterator& rhs) const { return !(operator==(rhs)); };
      private:
        IBinaryChannel& m_owner;
        experimental::measurement::base::EntryInfoSet::iterator m_entry_iterator;
        mutable std::string m_msg;
      };

      bool operator==(const IBinaryChannel& rhs) const { return channel == rhs.channel && meas == rhs.meas; /*return it == rhs.it; */ };
      bool operator!=(const IBinaryChannel& rhs) const { return !(operator==(rhs)); /*return it == rhs.it; */ };

      iterator begin()
      {
        return iterator(*this, entry_infos.begin());
      }

      iterator end()
      {
        return iterator(*this, entry_infos.end());
      }

    private:
      std::shared_ptr<experimental::measurement::base::Reader> meas;
      
      const experimental::measurement::base::Channel channel;
      const experimental::measurement::base::DataTypeInformation datatype_info;

      mutable experimental::measurement::base::EntryInfoSet entry_infos;
      mutable std::string data;
    };

    class IMeasurement
    {
    public:
      IMeasurement(const std::string& path);

      ChannelSet Channels() const;
      ChannelSet Channels(const std::string& channel_name) const;

      IBinaryChannel Get(const experimental::measurement::base::Channel& channel) const;

    private:
      std::shared_ptr<experimental::measurement::base::Reader> meas;
    };

    inline IMeasurement::IMeasurement(const std::string& path)
      : meas{ std::make_shared<experimental::measurement::hdf5::Reader>(path) }
    {
    }

    inline ChannelSet IMeasurement::Channels() const
    {
      return meas->GetChannels();
    }

    // This is probably not very performant. We should check!
    inline ChannelSet IMeasurement::Channels(const std::string& channel_name) const
    {
      ChannelSet channels_filtered_by_name;
      auto all_channels = meas->GetChannels();
      for (const auto& channel : all_channels) {
        if (channel.name == channel_name)
        {
          channels_filtered_by_name.insert(channel);
        }
      }
      return channels_filtered_by_name;
    }

    inline IBinaryChannel IMeasurement::Get(const experimental::measurement::base::Channel& channel) const
    {
      // Assert that the channel is in the IMeasurement
      auto channels = Channels();
      if (channels.find(channel) == channels.end())
      {
        // Throw an exception, if channel is not available?
        throw std::out_of_range("The channel {" + channel.name + ", " + std::to_string(channel.id) + "} does not exist in this measurement");
      }

      // Construct a binary Channel
      return IBinaryChannel{meas, channel};
    }
  }
}
