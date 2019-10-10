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

#pragma once

#include <memory>
#include <set>
#include <string>
#include <utility>

#include <ecalhdf5/eh5_meas.h>
#include <ecal/measurement/measurement.h>

namespace eCAL
{
  namespace measurement
  {
    class OBinaryChannel
    {
    public:
      OBinaryChannel(std::shared_ptr<eh5::HDF5Meas> meas_, const std::string& name_)
        : channel_name(name_)
        , meas(meas_)
        , SenderID(0)
        , clock(0)
      {
      }

      OBinaryChannel& operator<<(const BinaryFrame& entry_)
      {
        meas->AddEntryToFile((void*)entry_.message.data(), entry_.message.size(), entry_.send_timestamp, entry_.receive_timestamp, channel_name, SenderID, clock);
        ++clock;
        return *this;
      }

      OBinaryChannel& operator<<(const SenderID& id_)
      {
        SenderID = id_.ID;
        return *this;
      }

      bool operator==(const OBinaryChannel& rhs) const { return channel_name == rhs.channel_name && meas == rhs.meas; /*return it == rhs.it; */ };
      bool operator!=(const OBinaryChannel& rhs) const { return !(operator==(rhs)); /*return it == rhs.it; */ };


    private:
      const std::string channel_name;
      std::shared_ptr<eh5::HDF5Meas> meas;

      long long SenderID;
      long long clock;
    };


    template <typename T>
    class OChannel
    {
    public:
      OChannel(std::shared_ptr<eh5::HDF5Meas> meas_, std::string name_)
        : binary_channel(meas_, name_)
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
      std::shared_ptr<eh5::HDF5Meas> meas;
    };



    OMeasurement::OMeasurement(const std::string& base_path_, const std::string& measurement_name_)
      : meas{ std::make_shared<eh5::HDF5Meas>(base_path_, eh5::CREATE) }
    {
      meas->SetFileBaseName(measurement_name_);
    }

    // This will return a nullptr if channel name and 
    // This will throw an exception if 
    // a) channel does not exist in the OMeasurement
    // b) the registered type does not match with the descriptor in the chanenel
    template<typename T>
    OChannel<T> OMeasurement::Create(const std::string& channel) const
    {
      static T msg;
      meas->SetChannelType(channel, eCAL::message::GetTypeName(msg));
      meas->SetChannelDescription(channel, eCAL::message::GetDescription(msg));
        // Construct a channel based 
      return OChannel<T>{meas, channel};
    }

    
  }
}