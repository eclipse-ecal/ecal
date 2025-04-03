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

#include <ecal/measurement/omeasurement.h>

namespace eCAL
{
  namespace measurement
  {
    template <typename T, typename Serializer>
    class OMessageChannel
    {
      friend OMessageChannel CreateChannel(OMeasurement& meas_, const std::string& channel_name_);

    public:
      // Should those be private?
      using SerializerT = Serializer;
      using MessageT = T;

      OMessageChannel(OChannel&& binary_channel)
        : m_serializer(std::make_shared<Serializer>())
        , binary_channel(std::move(binary_channel))
      {
      }

      ~OMessageChannel() = default;

      OMessageChannel(const OMessageChannel&) = delete;
      OMessageChannel& operator=(const OMessageChannel&) = delete;

      OMessageChannel(OMessageChannel&& rhs) = default;
      OMessageChannel& operator=(OMessageChannel&& rhs) = default;

      bool operator==(const OMessageChannel& rhs) const { return  binary_channel == rhs.binary_channel ;}
      bool operator!=(const OMessageChannel& rhs) const { return !(operator==(rhs)); }

      OMessageChannel& operator<<(const Frame<T>& entry_)
      {
        // The way we handle Publishers requires us to do a two pass serialization;
        size_t message_size = m_serializer->MessageSize(entry_.message);
        buffer.resize(message_size);
        if (message_size == 0)
        {
          m_serializer->Serialize(entry_.message, nullptr, 0);
        }
        else
        {
          m_serializer->Serialize(entry_.message, static_cast<void*>(&buffer[0]), buffer.size());
        }
        BinaryFrame binary_frame{ buffer, entry_.send_timestamp, entry_.receive_timestamp };
        binary_channel << binary_frame;
        return *this;
      }
      
    private:
      std::shared_ptr<Serializer> m_serializer;
      OChannel binary_channel;
      mutable std::string buffer;
    };
    
    template <typename Channel>
    Channel CreateChannel(OMeasurement& meas_, const std::string& channel_name_)
    {
      typename Channel::SerializerT serializer;

      auto binary_channel = meas_.Create(channel_name_, serializer.GetDataTypeInformation());
      return Channel(std::move(binary_channel));
    }
  }
}