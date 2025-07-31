
.. _program_listing_file_contrib_ecalhdf5_include_ecalhdf5_eh5_meas_api_v3.h:

Program Listing for File eh5_meas_api_v3.h
==========================================

|exhale_lsh| :ref:`Return to documentation for file <file_contrib_ecalhdf5_include_ecalhdf5_eh5_meas_api_v3.h>` (``contrib/ecalhdf5/include/ecalhdf5/eh5_meas_api_v3.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   
   #include <cstdint>
   #include <functional>
   #include <set>
   #include <string>
   #include <memory>
   
   #include "eh5_types.h"
   
   
   namespace eCAL
   {
     namespace eh5
     {
       class HDF5MeasImpl;
   
       inline namespace v3{
         
       class HDF5Meas
       {
       public:
         HDF5Meas();
   
         explicit HDF5Meas(const std::string& path, v3::eAccessType access = v3::eAccessType::RDONLY);
   
         ~HDF5Meas();
   
         HDF5Meas(const HDF5Meas& other) = delete;
         HDF5Meas& operator=(const HDF5Meas& other) = delete;
   
         HDF5Meas(HDF5Meas&&) = default;
         HDF5Meas& operator=(HDF5Meas&&) = default;
   
         bool Open(const std::string& path, v3::eAccessType access = v3::eAccessType::RDONLY);
   
         bool Close();
   
         bool IsOk() const;
   
         std::string GetFileVersion() const;
   
         size_t GetMaxSizePerFile() const;
   
         void SetMaxSizePerFile(size_t size);
   
         bool IsOneFilePerChannelEnabled() const;
   
         void SetOneFilePerChannelEnabled(bool enabled);
   
         std::set<SChannel> GetChannels() const;
   
         bool HasChannel(const eCAL::eh5::SChannel& channel) const;
   
         DataTypeInformation GetChannelDataTypeInformation(const SChannel& channel) const;
   
         void SetChannelDataTypeInformation(const SChannel& channel, const DataTypeInformation& info);
   
         long long GetMinTimestamp(const SChannel& channel) const;
   
         long long GetMaxTimestamp(const SChannel& channel) const;
        
         bool GetEntriesInfo(const SChannel& channel, EntryInfoSet& entries) const;
   
         bool GetEntriesInfoRange(const SChannel& channel, long long begin, long long end, EntryInfoSet& entries) const;
   
         bool GetEntryDataSize(long long entry_id, size_t& size) const;
   
         bool GetEntryData(long long entry_id, void* data) const;
   
         void SetFileBaseName(const std::string& base_name);
   
         bool AddEntryToFile(const SWriteEntry& entry);
   
         typedef std::function<void(void)> CallbackFunction;
   
         void ConnectPreSplitCallback(CallbackFunction cb);
   
         void DisconnectPreSplitCallback();
   
        private:
         std::unique_ptr<HDF5MeasImpl> hdf_meas_impl_;
       };
       }
     }  // namespace eh5
   }  // namespace eCAL
