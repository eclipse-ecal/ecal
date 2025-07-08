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

/**
 * eCALHDF5 file reader multiple channels
**/

#pragma once

#include <string>

#include "eh5_meas_file_v3.h"
#include "escape.h"

namespace eCAL
{
  namespace eh5
  {
    class HDF5MeasFileV4 : virtual public HDF5MeasFileV3
    {
    public:
      /**
      * @brief Constructor
      **/
      HDF5MeasFileV4();

      /**
      * @brief Constructor
      *
      * @param path    Input file path
      **/
      explicit HDF5MeasFileV4(const std::string& path, v3::eAccessType access = v3::eAccessType::RDONLY);

      /**
      * @brief Destructor
      **/
      ~HDF5MeasFileV4() override;

      /**
      * @brief Gets the header info for all data entries for the given channel
      *        Header = timestamp + entry id
      *
      * @param [in]  channel_name  channel name
      * @param [out] entries       header info for all data entries
      *
      * @return                    true if succeeds, false if it fails
      **/
      bool GetEntriesInfo(const SEscapedChannel& channel, EntryInfoSet& entries) const override;
    };
  }  //  namespace eh5
}  //  namespace eCAL
