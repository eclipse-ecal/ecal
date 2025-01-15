
.. _program_listing_file_contrib_ecalhdf5_include_ecalhdf5_eh5_meas_api_v2.h:

Program Listing for File eh5_meas_api_v2.h
==========================================

|exhale_lsh| :ref:`Return to documentation for file <file_contrib_ecalhdf5_include_ecalhdf5_eh5_meas_api_v2.h>` (``contrib/ecalhdf5/include/ecalhdf5/eh5_meas_api_v2.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   
   #include <functional>
   #include <map>
   #include <memory>
   #include <set>
   #include <string>
   
   #include "eh5_types.h"
   
   namespace eCAL
   {
     namespace eh5
     {
       inline namespace v3
       {
         class HDF5Meas;
       }
   
       namespace v2
       {
       class HDF5Meas
       {
       public:
         HDF5Meas();
   
         explicit HDF5Meas(const std::string& path, eAccessType access = RDONLY);
   
         ~HDF5Meas();
   
         HDF5Meas(const HDF5Meas& other) = delete;
         HDF5Meas& operator=(const HDF5Meas& other) = delete;
   
         HDF5Meas(HDF5Meas&&) = default;
         HDF5Meas& operator=(HDF5Meas&&) = default;
   
         bool Open(const std::string& path, eAccessType access = RDONLY);
   
         bool Close();
   
         bool IsOk() const;
   
         std::string GetFileVersion() const;
   
         size_t GetMaxSizePerFile() const;
   
         void SetMaxSizePerFile(size_t size);
   
         bool IsOneFilePerChannelEnabled() const;
   
         void SetOneFilePerChannelEnabled(bool enabled);
   
         std::set<std::string> GetChannelNames() const;
   
         bool HasChannel(const std::string& channel_name) const;
   
         std::string GetChannelDescription(const std::string& channel_name) const;
   
         void SetChannelDescription(const std::string& channel_name, const std::string& description);
   
         std::string GetChannelType(const std::string& channel_name) const;
   
         void SetChannelType(const std::string& channel_name, const std::string& type);
   
         DataTypeInformation GetChannelDataTypeInformation(const std::string& channel_name) const;
   
         void SetChannelDataTypeInformation(const std::string& channel_name, const DataTypeInformation& info);
   
         long long GetMinTimestamp(const std::string& channel_name) const;
   
         long long GetMaxTimestamp(const std::string& channel_name) const;
   
         bool GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const;
   
         bool GetEntriesInfoRange(const std::string& channel_name, long long begin, long long end, EntryInfoSet& entries) const;
   
         bool GetEntryDataSize(long long entry_id, size_t& size) const;
   
         bool GetEntryData(long long entry_id, void* data) const;
   
         void SetFileBaseName(const std::string& base_name);
   
         bool AddEntryToFile(const void* data, const unsigned long long& size, const long long& snd_timestamp, const long long& rcv_timestamp, const std::string& channel_name, long long id, long long clock);
   
         typedef std::function<void(void)> CallbackFunction;
   
         void ConnectPreSplitCallback(CallbackFunction cb);
   
         void DisconnectPreSplitCallback();
   
        private:
         std::unique_ptr<v3::HDF5Meas> hdf_meas_impl_;
         // this map saves all datatype infos that have been set, so that the api can still
         // support setting type and descriptor separately
         std::map<std::string, DataTypeInformation> data_type_info_map;
       };
       } // namespace v1
     }  // namespace eh5
   }  // namespace eCAL
