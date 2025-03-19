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

#include <atomic>
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
      friend class OMeasurement;
      // The constructor is private
      // Only measurements can create Channel objects, not the user.
      OBinaryChannel(std::shared_ptr<experimental::measurement::base::Writer> meas_, const experimental::measurement::base::Channel& channel_, const eCAL::experimental::measurement::base::DataTypeInformation& datatype_info)
        : channel(channel_)
        , meas(meas_)
        , clock(0)
      {
        meas->SetChannelDataTypeInformation(channel, datatype_info);
      }

    public:
      OBinaryChannel& operator<<(const BinaryFrame& entry_)
      {
        eCAL::experimental::measurement::base::WriteEntry entry;
        entry.channel = channel;
        entry.data = entry_.message.data();
        entry.size = entry_.message.size();
        entry.snd_timestamp = entry_.send_timestamp;
        entry.rcv_timestamp = entry_.receive_timestamp;
        // We no longer set an entry_id, deprecated with hdf5 v5.
        entry.clock = clock;

        meas->AddEntryToFile(entry);
        ++clock;
        return *this;
      }

      ~OBinaryChannel() = default;

      OBinaryChannel(const OBinaryChannel&) = delete;
      OBinaryChannel& operator=(const OBinaryChannel&) = delete;

      OBinaryChannel(OBinaryChannel&& rhs) = default;
      OBinaryChannel& operator=(OBinaryChannel&& rhs) = default;

      bool operator==(const OBinaryChannel& rhs) const { return channel == rhs.channel && meas == rhs.meas; /*return it == rhs.it; */ };
      bool operator!=(const OBinaryChannel& rhs) const { return !(operator==(rhs)); /*return it == rhs.it; */ };


    private:
      const experimental::measurement::base::Channel channel;
      std::shared_ptr<experimental::measurement::base::Writer> meas;

      long long clock;
    };

    class OMeasurement
    {
    public:
      OMeasurement(const std::string& base_path_, const std::string& measurement_name_= "measurement");

      OBinaryChannel Create(const std::string& channel_, const eCAL::experimental::measurement::base::DataTypeInformation& datatype_info_) const;

    private:
      std::shared_ptr<experimental::measurement::base::Writer> meas;
    };



    inline OMeasurement::OMeasurement(const std::string& base_path_, const std::string& measurement_name_)
      : meas{ std::make_shared<eCAL::experimental::measurement::hdf5::Writer>(base_path_) }
    {
      meas->SetFileBaseName(measurement_name_);
    }

    inline OBinaryChannel OMeasurement::Create(const std::string& channel_, const eCAL::experimental::measurement::base::DataTypeInformation& datatype_info_) const
    {
      // Construct a channel based
      static std::atomic<eCAL::experimental::measurement::base::Channel::id_t> i = 0;
      return OBinaryChannel{ meas, {channel_, ++i}, datatype_info_ };
    }

    
  }
}