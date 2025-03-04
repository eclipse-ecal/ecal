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
    class IChannel
    {
    public:
      IChannel(std::shared_ptr<experimental::measurement::base::Reader> meas_, const experimental::measurement::base::Channel& channel_)
        : m_serializer{}
        , binary_channel(meas_, channel_)
      {
      }

      bool operator==(const IChannel& rhs) const { return  binary_channel == rhs.binary_channel; }
      bool operator!=(const IChannel& rhs) const { return !(operator==(rhs)); }

      //virtual Entry<T> operator[](unsigned long long timestamp);
      virtual Frame<T> operator[](const experimental::measurement::base::EntryInfo& entry)
      {
        auto binary_entry = binary_channel[entry];
        
        try
        {
          message = m_deserializer.Deserialize(binary_entry.message.c_str(), binary_entry.message.size(), GetDataypeInformation());
          return make_frame(message, binary_entry.send_timestamp, binary_entry.receive_timestamp);
        }

      }

      const std::string& GetName() const
      {
        return binary_channel.GetName();
      }

      const eCAL::experimental::measurement::base::DataTypeInformation& GetDataypeInformation() const
      {
        return binary_channel.GetDatatypeInformation();
      }

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
          BinaryFrame binary_frame = *it;
          message = m_deserializer.Deserialize(binary_frame.message.c_str(), binary_frame.message.size(), GetDataypeInformation());
          return make_frame(message, binary_frame.send_timestamp, binary_frame.receive_timestamp);
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

    private:
      Serializer m_serializer;
      IBinaryChannel binary_channel;
      mutable T message;
    };
  }
}
