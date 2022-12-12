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

#include "eh5_meas_file_v5.h"

#include "hdf5.h"

namespace eCAL
{
    namespace eh5
    {

        HDF5MeasFileV5::HDF5MeasFileV5(const std::string& path, eAccessType access /*= eAccessType::RDONLY*/)
                : HDF5MeasFileV2(path, access)
        {
        }

        HDF5MeasFileV5::HDF5MeasFileV5()
        = default;

        HDF5MeasFileV5::~HDF5MeasFileV5()
        = default;

        bool HDF5MeasFileV5::GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const
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

            for (unsigned int index = 0; index < data_size; index += 5)
            {
                //                        rec timestamp,  channel id,       send clock,       send time stamp,  send ID
                entries.emplace(SEntryInfo(data[index],    data[index + 1],  data[index + 2],  data[index + 3],  data[index + 4]));
            }

            free(data);
            data = nullptr;

            return (status >= 0);
        }


    }  //  namespace eh5
}  //  namespace eCAL


bool eCAL::eh5::HDF5MeasFileV5::SetGZipCompressionFilter(unsigned level) {
    bool correct_level = false;
    if(level <= 9) {
        gzip_compression_level_ = level;
        correct_level = true;
        if(ndims_ == 0) {
            ndims_ = 1;
            // TODO: use smart pointers instead, because current code leaks memory
            dim_ = new unsigned long long[1] ;
            dim_[0] = 1;
        }
    }

    return correct_level;
}

bool eCAL::eh5::HDF5MeasFileV5::IsGZipCompressionFilterEnabled() {
    return gzip_compression_level_ > 0 ? true : false;
}

bool eCAL::eh5::HDF5MeasFileV5::SetSZipCompressionFilter(unsigned options_mask, unsigned pixels_per_block) {
    bool correct_params = false;
    if((options_mask == H5_SZIP_EC_OPTION_MASK || options_mask == H5_SZIP_NN_OPTION_MASK) &&
       ((pixels_per_block % 2 == 0) && pixels_per_block <= 32 && pixels_per_block > 0)
            ) {
        options_mask_ = options_mask;
        pixels_per_block_ = pixels_per_block;
        szip_compression_enabled_ = true;
        correct_params = true;
        if(ndims_ == 0) {
            ndims_ = 1;
            // TODO: use smart pointers instead, because current code leaks memory
            dim_ = new unsigned long long[1] ;
            dim_[0] = 1;
        }
    } else {
        szip_compression_enabled_ = false;
    }

    return correct_params;
}

bool eCAL::eh5::HDF5MeasFileV5::IsSZipCompressionFilterEnabled() {
    return szip_compression_enabled_;
}

bool eCAL::eh5::HDF5MeasFileV5::SetChunkDimensions(int ndims, unsigned long long dim[/*ndims*/]) {
    bool valid_dim = false;
    if(ndims >= 0) {
        ndims_ = ndims;
        dim_ = dim;
        valid_dim = true;
    }
    return valid_dim;
}

bool eCAL::eh5::HDF5MeasFileV5::IsChunkingEnabled() {
    return (ndims_ > 0);
}