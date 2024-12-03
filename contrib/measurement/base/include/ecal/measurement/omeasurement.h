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

#include <ecal/measurement/hdf5/writer.h>
#include <ecal/measurement/measurement.h>

namespace eCAL
{
  namespace measurement
  {
    class OBinaryChannel
    {
    public:
      OBinaryChannel(std::shared_ptr<experimental::measurement::base::Writer> meas_, const std::string& name_, const eCAL::experimental::measurement::base::DataTypeInformation& datatype_info)
        : channel(name_, 0)
        , meas(meas_)
        , id(0)
        , clock(0)
      {
        meas->SetChannelDataTypeInformation(channel, datatype_info);
      }

      OBinaryChannel& operator<<(const BinaryFrame& entry_)
      {
        eCAL::experimental::measurement::base::WriteEntry entry;
        entry.channel = channel;
        entry.data = entry_.message.data();
        entry.size = entry_.message.size();
        entry.snd_timestamp = entry_.send_timestamp;
        entry.rcv_timestamp = entry_.receive_timestamp;
        entry.sender_id = id;
        entry.clock = clock;

        meas->AddEntryToFile(entry);
        ++clock;
        return *this;
      }

      OBinaryChannel& operator<<(const SenderID& id_)
      {
        id = id_.ID;
        return *this;
      }

      bool operator==(const OBinaryChannel& rhs) const { return channel == rhs.channel && meas == rhs.meas; /*return it == rhs.it; */ };
      bool operator!=(const OBinaryChannel& rhs) const { return !(operator==(rhs)); /*return it == rhs.it; */ };


    private:
      const experimental::measurement::base::Channel channel;
      std::shared_ptr<experimental::measurement::base::Writer> meas;

      long long id;
      long long clock;
    };


    template <typename T>
    class OChannel
    {
    public:
      OChannel(std::shared_ptr<experimental::measurement::base::Writer> meas_, std::string name_, const eCAL::experimental::measurement::base::DataTypeInformation& datatype_info)
        : binary_channel(meas_, name_, datatype_info)
      {
      }

      bool operator==(const OChannel& rhs) const { return  binary_channel == rhs.binary_channel ;}
      bool operator!=(const OChannel& rhs) const { return !(operator==(rhs)); }

      OChannel<T>& operator<<(const Frame<T>& entry_)
      {
        eCAL::message::Serialize(entry_.message, buffer);
        BinaryFrame binary_frame{ buffer, entry_.send_timestamp, entry_.receive_timestamp };
        binary_channel << binary_frame;
        return *this;
      }
      

      // Streaming operator to change the sender ID
      OChannel<T>& operator<<(const SenderID& id_)
      {
        binary_channel << id_;
        return *this;
      }


    protected:
      OBinaryChannel binary_channel;
      mutable std::string buffer;
    };

    using OStringChannel = OChannel<std::string>;

    class OMeasurement
    {
    public:
      OMeasurement(const std::string& base_path_, const std::string& measurement_name_= "measurement");

      template<typename T>
      OChannel<T> Create(const std::string& channel) const;

    private:
      std::shared_ptr<experimental::measurement::base::Writer> meas;
    };



    inline OMeasurement::OMeasurement(const std::string& base_path_, const std::string& measurement_name_)
      : meas{ std::make_shared<eCAL::experimental::measurement::hdf5::Writer>(base_path_) }
    {
      meas->SetFileBaseName(measurement_name_);
    }

    // This will return a nullptr if channel name and 
    // This will throw an exception if 
    // a) channel does not exist in the OMeasurement
    // b) the registered type does not match with the descriptor in the chanenel
    template<typename T>
    inline OChannel<T> OMeasurement::Create(const std::string& channel) const
    {
      static T msg;
      const eCAL::experimental::measurement::base::DataTypeInformation datatype_info{
        eCAL::message::GetTypeName(msg),
        eCAL::message::GetEncoding(msg),
        eCAL::message::GetDescription(msg)
      };
      // Construct a channel based 
      return OChannel<T>{meas, channel, datatype_info};
    }

    
  }
}