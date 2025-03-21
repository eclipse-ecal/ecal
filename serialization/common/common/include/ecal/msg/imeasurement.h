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

#include <ecal/measurement/imeasurement.h>
#include <ecal/msg/exception.h>

namespace eCAL
{
  namespace measurement
  {
    template <typename T, typename Serializer>
    class IMessageChannel
    {
      //This was a try to make the constructor of IMessageChannel private, which, however did not work.
      //friend IMessageChannel GetChannel(IMeasurement& meas, const experimental::measurement::base::Channel& channel_);

    public:
      using SerializerT = Serializer;
      using MessageT = T;

      IMessageChannel(IChannel&& binary_channel_)
        : m_serializer{std::make_shared<Serializer>()}
        , binary_channel(std::move(binary_channel_))
      {
      }

      ~IMessageChannel() = default;

      IMessageChannel(const IMessageChannel&) = delete;
      IMessageChannel& operator=(const IMessageChannel&) = delete;

      IMessageChannel(IMessageChannel&& rhs) = default;
      IMessageChannel& operator=(IMessageChannel&& rhs) = default;

      bool operator==(const IMessageChannel& rhs) const { return  binary_channel == rhs.binary_channel; }
      bool operator!=(const IMessageChannel& rhs) const { return !(operator==(rhs)); }

      Frame<T> operator[](const experimental::measurement::base::EntryInfo& entry)
      {
        auto binary_entry = binary_channel[entry];
        
        message = m_serializer->Deserialize(binary_entry.message.c_str(), binary_entry.message.size(), GetDataypeInformation());
        return make_frame(message, binary_entry.send_timestamp, binary_entry.receive_timestamp);
      }

      const std::string& GetName() const
      {
        return binary_channel.GetName();
      }

      const eCAL::experimental::measurement::base::DataTypeInformation& GetDataypeInformation() const
      {
        return binary_channel.GetDatatypeInformation();
      }

      class iterator
      {
      public:
        iterator(const iterator& i)
          : it(i.it)
          , owner(i.owner)
        {};

        iterator(const IChannel::iterator& i, IMessageChannel& owner_)
          : it(i)
          , owner(owner_)
        {};

        ~iterator()
        {};

        iterator& operator=(const iterator& i)
        {
          it = i.it;
          owner = i.owner;
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
          BinaryFrame binary_frame = *it;
          message = owner.m_serializer->Deserialize(binary_frame.message.c_str(), binary_frame.message.size(), owner.GetDataypeInformation());
          return make_frame(message, binary_frame.send_timestamp, binary_frame.receive_timestamp);
        };
        //friend void swap(iterator& lhs, iterator& rhs); //C++11 I think
        bool operator==(const iterator& rhs) const { return it == rhs.it; };
        bool operator!=(const iterator& rhs) const { return it != rhs.it; };

      private:
        IChannel::iterator it;
        mutable T message;

        IMessageChannel& owner;
      };

      iterator begin()
      {
        return iterator(binary_channel.begin(), *this);
      }

      iterator end()
      {
        return iterator(binary_channel.end(), *this);
      }

    private:
      IChannel binary_channel;
      mutable T message;

      std::shared_ptr<Serializer> m_serializer;
    };

    template <typename Channel>
    Channel GetChannel(IMeasurement& meas_, const experimental::measurement::base::Channel& channel_)
    {
      auto binary_channel = meas_.Get(channel_);
      return Channel(std::move(binary_channel));
    }
  }
}
