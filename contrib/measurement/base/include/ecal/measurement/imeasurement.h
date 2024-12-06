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

#pragma once

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <stdexcept>

#include <ecal/measurement/hdf5/reader.h>
#include <ecal/measurement/measurement.h>

namespace eCAL
{
  namespace measurement
  {
    class IBinaryChannel
    {
    public:
      IBinaryChannel(std::shared_ptr<experimental::measurement::base::Reader> meas_, experimental::measurement::base::Channel channel_)
        : channel(channel_)
        , meas(meas_)
      {
        meas->GetEntriesInfo(channel, entry_infos);
      }

      virtual BinaryFrame operator[](const experimental::measurement::base::EntryInfo& entry)
      {
        size_t message_size;
        meas->GetEntryDataSize(entry.ID, message_size);
        data.resize(message_size);
        meas->GetEntryData(entry.ID, (void*)data.data());
        return make_frame( data, entry.SndTimestamp, entry.RcvTimestamp );
      }

      std::string name()
      {
        return channel.name;
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
      const experimental::measurement::base::Channel channel;
      std::shared_ptr<experimental::measurement::base::Reader> meas;
      mutable experimental::measurement::base::EntryInfoSet entry_infos;
      mutable std::string data;
    };


    template <typename T>
    class IChannel
    {
    public:
      IChannel(std::shared_ptr<experimental::measurement::base::Reader> meas_, const experimental::measurement::base::Channel& channel_)
        : binary_channel(meas_, channel_)
      {
      }

      bool operator==(const IChannel& rhs) const { return  binary_channel == rhs.binary_channel; }
      bool operator!=(const IChannel& rhs) const { return !(operator==(rhs)); }

      //virtual Entry<T> operator[](unsigned long long timestamp);
      virtual Frame<T> operator[](const experimental::measurement::base::EntryInfo& entry)
      {
        auto binary_entry = binary_channel[entry];
        eCAL::message::Deserialize(binary_entry.message, message);
        return make_frame( message, binary_entry.send_timestamp, binary_entry.receive_timestamp );
      }

      std::string name()
      {
        return binary_channel.name();
      }

      //typedef typename Entry<T> value_type;
      //typedef typename Alloc::reference reference;
      //typedef typename Alloc::const_reference const_reference;
      //typedef typename Alloc::difference_type difference_type;
      //typedef typename Alloc::size_type size_type;

      class iterator /*: public std::iterator<std::forward_iterator_tag, Entry<T>>*/
      {
      public:
        iterator(const iterator& i)
          : it(i.it)
        {};

        iterator(const IBinaryChannel::iterator& i)
          : it(i)
        {};

        ~iterator()
        {};

        iterator& operator=(const iterator& i)
        {
          it = i.it;
          return *this;
        };
        iterator& operator++()
        {
          ++it;
          return *this;
        }; //prefix increment
        iterator& operator--()
        {
          --it;
          return *this;
        }; //prefix decrement
           //reference operator*() const

        virtual Frame<T> operator*() const
        {
          //  return m_owner[*m_entry_iterator];
          BinaryFrame e = *it;
          eCAL::message::Deserialize(e.message, message);
          return make_frame(message, e.send_timestamp, e.receive_timestamp);
        };
        //friend void swap(iterator& lhs, iterator& rhs); //C++11 I think
        bool operator==(const iterator& rhs) const { return it == rhs.it; };
        bool operator!=(const iterator& rhs) const { return it != rhs.it; };

      protected:
        IBinaryChannel::iterator it;
        mutable T message;
      };

      iterator begin()
      {
        return iterator(binary_channel.begin());
      }

      iterator end()
      {
        return iterator(binary_channel.end());
      }

    protected:
      IBinaryChannel binary_channel;
      mutable T message;
    };

    class IMeasurement
    {
    public:
      IMeasurement(const std::string& path);

      ChannelSet Channels() const;
      ChannelSet Channels(const std::string& channel_name) const;

      template<typename T>
      IChannel<T> Get(const experimental::measurement::base::Channel& channel) const;

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

    // This will return a nullptr if channel name and 
    // This will throw an exception if 
    // a) channel does not exist in the IMeasurement
    // b) the registered type does not match with the descriptor in the chanenel
    template<typename T>
    inline IChannel<T> IMeasurement::Get(const experimental::measurement::base::Channel& channel) const
    {
      // Assert that the channel is in the IMeasurement
      auto channels = Channels();
      if (channels.find(channel) == channels.end())
      {
        // Throw an exception, if channel is not available?
        throw std::out_of_range("The channel {" + channel.name + ", " + std::to_string(channel.id) + "} does not exist in this measurement");
      }

      // Assert that the channel type is compatible with the requested type

      // Construct a channel based 
      return IChannel<T>{meas, channel};
    }

  }
}
