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
 * @brief  eCALHDF5 reader multiple channels implement
**/

#include "eh5_meas_file_v4.h"

#include "hdf5.h"

namespace eCAL
{
  namespace eh5
  {

    HDF5MeasFileV4::HDF5MeasFileV4(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
      : HDF5MeasFileV2(path, access)
    {
    }

    HDF5MeasFileV4::HDF5MeasFileV4()
    {
    }

    HDF5MeasFileV4::~HDF5MeasFileV4()
    {
    }

    bool HDF5MeasFileV4::GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const
    {
      entries.clear();

      if (!this->IsOk()) return false;

      auto dataset_id = H5Dopen(file_id_, channel_name.c_str(), H5P_DEFAULT);

      if (dataset_id < 0) return false;

      const size_t sizeof_ll = sizeof(long long);
      hsize_t data_size = H5Dget_storage_size(dataset_id) / sizeof_ll;

      if (data_size <= 0) return false;

      long long* data = static_cast<long long*>(calloc(static_cast<size_t>(data_size), sizeof_ll));
      if (data == nullptr) return false;

      herr_t status = H5Dread(dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

      H5Dclose(dataset_id);

      for (unsigned int index = 0; index < data_size; index += 4)
      {
        //                        rec timestamp, channel id,      send clock,      send time stamp
        entries.emplace(SEntryInfo(data[index],   data[index + 1], data[index + 2], data[index + 3]));
      }

      free(data);
      data = nullptr;

      return (status >= 0);
    }
  }  //  namespace eh5
}  //  namespace eCAL
