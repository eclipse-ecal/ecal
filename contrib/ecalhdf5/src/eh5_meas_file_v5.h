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
 * eCALHDF5 file reader multiple channels
**/

#pragma once

#include <string>

#include "eh5_meas_file_v4.h"

namespace eCAL
{
    namespace eh5
    {
        class HDF5MeasFileV5 : virtual public HDF5MeasFileV4
        {
        public:
            /**
            * @brief Constructor
            **/
            HDF5MeasFileV5();

            /**
            * @brief Constructor
            *
            * @param path    Input file path
            **/
            explicit HDF5MeasFileV5(const std::string& path, eAccessType access = eAccessType::RDONLY);

            /**
            * @brief Destructor
            **/
            ~HDF5MeasFileV5() override;

            /**
            * @brief Gets the header info for all data entries for the given channel
            *        Header = timestamp + entry id
            *
            * @param [in]  channel_name  channel name
            * @param [out] entries       header info for all data entries
            *
            * @return                    true if succeeds, false if it fails
            **/
            bool GetEntriesInfo(const std::string& channel_name, EntryInfoSet& entries) const override;

            /**
            * @brief Enable GNU gzip compression filter and defines compression level
            *
            * @param level   compression level ranges from 0 which means no compression and 9 which highest compression
            **/
            bool SetGZipCompressionFilter(unsigned level) override;

            /**
            * @brief Check if GNU Gzip filter is enabled or not
            *
            * @return               true if GNU Gzip filter is enabled with level between 1 - 9
            **/
            bool IsGZipCompressionFilterEnabled() override;

            /**
            * @brief Enable szip compression filter
            *
            * @param options_mask     A bit-mask conveying the desired SZIP options; Valid values are H5_SZIP_EC_OPTION_MASK and H5_SZIP_NN_OPTION_MASK
            *
            * @param pixels_per_block The number of pixels or data elements in each data block
            **/
            virtual bool SetSZipCompressionFilter(unsigned options_mask, unsigned pixels_per_block) override;

            /**
            * @brief Check if szip filter is enabled or not
            *
            * @return               true if szip filter is enabled
            **/
            virtual bool IsSZipCompressionFilterEnabled() override;

            /**
            * @brief Sets the size of the chunks used to store a chunked layout dataset
            *
            * @param ndims     The number of dimensions of each chunk
            *
            * @param dim       An array defining the dividing factor to message length, in dataset elements, of each chunk
            **/
            virtual bool SetChunkDimensions(int ndims, unsigned long long dim[/*ndims*/]) override;

            /**
            * @brief Check if chunking is enabled or not
            *
            * @return               true if chunking is enabled
            **/
            virtual bool IsChunkingEnabled() override;

        };
    }  //  namespace eh5
}  //  namespace eCAL