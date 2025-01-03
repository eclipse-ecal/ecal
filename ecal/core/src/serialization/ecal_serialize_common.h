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
 * @file   ecal_serialize_common.h
 * @brief  eCAL common (de)serialization
**/

#pragma once

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"

#include "ecal_struct_sample_payload.h"
#include "ecal_struct_sample_registration.h"
#include "ecal_struct_service.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace eCAL
{
  namespace nanopb
  {
    struct SNanoBytes
    {
      pb_byte_t* content = nullptr;
      size_t     length  = 0;
    };

    void encode_string(pb_callback_t& pb_callback, const std::string& str);
    void decode_string(pb_callback_t& pb_callback, std::string& str);

    void encode_int_to_string(pb_callback_t& pb_callback, const uint64_t& int_argument);
    void decode_int_from_string(pb_callback_t& pb_callback, uint64_t& int_argument);

    void encode_bytes(pb_callback_t& pb_callback, const SNanoBytes& nano_bytes);
    void encode_bytes(pb_callback_t& pb_callback, const std::vector<char>& vec);
    void decode_bytes(pb_callback_t& pb_callback, std::vector<char>& vec);

    void encode_map(pb_callback_t& pb_callback, const std::map<std::string, std::string>& str_map);
    void decode_map(pb_callback_t& pb_callback, std::map<std::string, std::string>& str_map);

    void encode_registration_layer(pb_callback_t& pb_callback, const Util::CExpandingVector<eCAL::Registration::TLayer>& layer_vec);
    void decode_registration_layer(pb_callback_t& pb_callback, Util::CExpandingVector<eCAL::Registration::TLayer>& layer_vec);

    void encode_service_methods(pb_callback_t& pb_callback, const Util::CExpandingVector<eCAL::Service::Method>& method_vec);
    void decode_service_methods(pb_callback_t& pb_callback, Util::CExpandingVector<eCAL::Service::Method>& method_vec);
  }
}
