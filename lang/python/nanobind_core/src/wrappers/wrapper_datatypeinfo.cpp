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
 * @brief  common datatype info based on eCAL, adapted for Nanobind
**/

#include <ecal/ecal.h>
#include <stdint.h>
#include <string>
#include <cstddef>
#include <ecal/ecal_types.h>

#include <wrappers/wrapper_datatypeinfo.h>

namespace eCAL
{
    SDataTypeInformation convert(const CNBDataTypeInformation& nb_info)
    {
        SDataTypeInformation info;
        info.name = nb_info.name;
        info.encoding = nb_info.encoding;
        info.descriptor = std::string(nb_info.descriptor.c_str(), nb_info.descriptor.size());

        return info;
    }
}