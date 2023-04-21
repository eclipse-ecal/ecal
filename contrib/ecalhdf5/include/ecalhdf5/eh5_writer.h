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

/**
 * @file   measurement.h
 * @brief  Base class for low level measurement oprations
**/

#pragma once

#include <functional>
#include <set>
#include <string>
#include <memory>

#include <ecal/measurement/base/writer.h>
#include <ecalhdf5/eh5_meas.h>

namespace eCAL
{
  namespace eh5
  {
      /**
       * @brief eCAL measurement API
      **/
      class Writer : public measurement::base::Writer
      {
      public:
        /**
         * @brief Constructor
        **/
        Writer() = default;

        /**
         * @brief Constructor
        **/
        Writer(const std::string& path) : measurement(path, eAccessType::CREATE) {}

        /**
         * @brief Destructor
        **/
        virtual ~Writer() = default;

        /**
         * @brief Copy operator
        **/
        Writer(const Writer& other) = delete;
        Writer& operator=(const Writer& other) = delete;

        /**
        * @brief Move operator
        **/
        Writer(Writer&&) = default;
        Writer& operator=(Writer&&) = default;

        /**
         * @brief Open file
         *
         * @param path     Input file path / measurement directory path.
         *
         *                 Default measurement directory structure:
         *                  - root directory e.g.: M:\measurement_directory\measurement01
         *                  - documents directory:                                |_doc
         *                  - hosts directories:                                  |_Host1 (e.g.: CARPC01)
         *                                                                        |_Host2 (e.g.: CARPC02)
         *
         *                 File path as input (AccessType::RDONLY):
         *                  - root directory (e.g.: M:\measurement_directory\measurement01) in this case all hosts subdirectories will be iterated,
         *                  - host directory (e.g.: M:\measurement_directory\measurement01\CARPC01),
         *                  - file path, path to file from measurement (e.g.: M:\measurement_directory\measurement01\CARPC01\meas01_05.hdf5).
         *
         *                 File path as output (AccessType::CREATE):
         *                  - full path to  measurement directory (recommended with host name) (e.g.: M:\measurement_directory\measurement01\CARPC01),
         *                  - to set the name of the actual hdf5 file use SetFileBaseName method.
         *
         * @param access   Access type
         *
         * @return         true if output (AccessType::CREATE) measurement directory structure can be accessed/created, false otherwise.
         *                 true if input (AccessType::RDONLY) measurement/file path was opened, false otherwise.
        **/
        bool Open(const std::string& path) override { return measurement.Open(path, eAccessType::CREATE); }

        /**
         * @brief Close file
         *
         * @return         true if succeeds, false if it fails
        **/
        bool Close() override { return measurement.Close(); }

        /**
         * @brief Checks if file/measurement is ok
         *
         * @return  true if meas can be opened(read) or location is accessible(write), false otherwise
        **/
        bool IsOk() const override { return measurement.IsOk(); }

        /**
         * @brief Gets maximum allowed size for an individual file
         *
         * @return       maximum size in MB
        **/
        size_t GetMaxSizePerFile() const override { return measurement.GetMaxSizePerFile(); }

        /**
         * @brief Sets maximum allowed size for an individual file
         *
         * @param size   maximum size in MB
        **/
        void SetMaxSizePerFile(size_t size) override { return measurement.SetMaxSizePerFile(size); }

        /**
        * @brief Whether each Channel shall be writte in its own file
        *
        * When enabled, data is clustered by channel and each channel is written
        * to its own file. The filenames will consist of the basename and the
        * channel name.
        *
        * @return true, if one file per channel is enabled
        */
        bool IsOneFilePerChannelEnabled() const override { return measurement.IsOneFilePerChannelEnabled(); }

        /**
        * @brief Enable / disable the creation of one individual file per channel
        *
        * When enabled, data is clustered by channel and each channel is written
        * to its own file. The filenames will consist of the basename and the
        * channel name.
        *
        * @param enabled   Whether one file shall be created per channel
        */
        void SetOneFilePerChannelEnabled(bool enabled) override { return measurement.SetOneFilePerChannelEnabled(enabled); }

        /**
         * @brief Set description of the given channel
         *
         * @param channel_name    channel name
         * @param description     description of the channel
        **/
        void SetChannelDescription(const std::string& channel_name, const std::string& description) override { return measurement.SetChannelDescription(channel_name, description); }

        /**
         * @brief Set type of the given channel
         *
         * @param channel_name  channel name
         * @param type          type of the channel
        **/
        void SetChannelType(const std::string& channel_name, const std::string& type) override { return measurement.SetChannelType(channel_name, type); }

        /**
         * @brief Set measurement file base name (desired name for the actual hdf5 files that will be created)
         *
         * @param base_name        Name of the hdf5 files that will be created.
        **/
        void SetFileBaseName(const std::string& base_name) { return measurement.SetFileBaseName(base_name); }

        /**
         * @brief Add entry to file
         *
         * @param data           data to be added
         * @param size           size of the data
         * @param snd_timestamp  send time stamp
         * @param rcv_timestamp  receive time stamp
         * @param channel_name   channel name
         * @param id             message id
         * @param clock          message clock
         *
         * @return              true if succeeds, false if it fails
        **/
        bool AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock) override
        {
          return measurement.AddEntryToFile(data, size, snd_timestamp, rcv_timestamp, channel_name, id, clock);
        }

        private:
          HDF5Meas measurement;

      };

  }  // namespace eh5
}  // namespace eCAL
