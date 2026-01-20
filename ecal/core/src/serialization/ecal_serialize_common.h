/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
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
 * @file   ecal_serialize_common.h
 * @brief  eCAL common (de)serialization
**/

#pragma once

#include "ecal_struct_sample_payload.h"
#include "ecal_struct_sample_registration.h"
#include "ecal_struct_service.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include <ecal/core/pb/datatype.pbftags.h>
#include <protozero/pbf_reader.hpp>
#include <protozero/ecal_helper.h>

namespace eCAL
{
  inline namespace protozero
  {
    template <typename Writer>
    void SerializeDataTypeInformation(Writer& writer, const eCAL::SDataTypeInformation& data_type_info)
    {
      writer.add_string(+eCAL::pb::DataTypeInformation::optional_string_name, data_type_info.name);
      writer.add_string(+eCAL::pb::DataTypeInformation::optional_string_encoding, data_type_info.encoding);
      writer.add_bytes(+eCAL::pb::DataTypeInformation::optional_bytes_descriptor_information, data_type_info.descriptor);
    }

    inline bool DeserializeDataTypeInformation(::protozero::pbf_reader& reader, eCAL::SDataTypeInformation& data_type_info)
    {
      while (reader.next())
      {
        switch (reader.tag())
        {
        case +eCAL::pb::DataTypeInformation::optional_string_name:
          AssignString(reader, data_type_info.name);
          break;
        case +eCAL::pb::DataTypeInformation::optional_string_encoding:
          AssignString(reader, data_type_info.encoding);
          break;
        case +eCAL::pb::DataTypeInformation::optional_bytes_descriptor_information:
          AssignBytes(reader, data_type_info.descriptor);
          break;
        default:
          reader.skip();
        }
      }
      return true;
    }

    void LogDeserializationException(const std::exception& exception, const std::string& context);
  }
}


