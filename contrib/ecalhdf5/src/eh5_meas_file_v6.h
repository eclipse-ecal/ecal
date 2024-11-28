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

/**
 * eCALHDF5 file reader multiple channels
**/

#pragma once

#include <string>

#include "eh5_meas_file_v5.h"

namespace eCAL
{
  namespace eh5
  {
    class HDF5MeasFileV6 : virtual public HDF5MeasFileV5
    {
    public:
      /**
      * @brief Constructor
      **/
      HDF5MeasFileV6();

      /**
      * @brief Constructor
      *
      * @param path    Input file path
      **/
      explicit HDF5MeasFileV6(const std::string& path, v3::eAccessType access = v3::eAccessType::RDONLY);

      /**
      * @brief Destructor
      **/
      ~HDF5MeasFileV6() override;

      /**
       * @brief Get the available channel names of the current opened file / measurement
       *
       * @return       channel names & ids
      **/
      std::set<eCAL::eh5::SChannel> GetChannels() const override;

      /**
       * @brief Check if channel exists in measurement
       *
       * @param channel   channel name & id
       *
       * @return       true if exists, false otherwise
      **/
      bool HasChannel(const eCAL::eh5::SChannel& channel) const override;

      DataTypeInformation GetChannelDataTypeInformation(const SChannel& channel) const override;

      bool GetEntriesInfo(const SChannel& channel, EntryInfoSet& entries) const override;
    };
  }  //  namespace eh5
}  //  namespace eCAL
