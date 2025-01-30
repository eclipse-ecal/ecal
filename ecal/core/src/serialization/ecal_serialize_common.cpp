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
 * @file   ecal_serialize_common.cpp
 * @brief  eCAL common (de)serialization
**/

#include "nanopb/ecal/core/pb/ecal.npb.h"
#include "nanopb/pb_decode.h"
#include "nanopb/pb_encode.h"
#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "ecal_serialize_common.h"

namespace eCAL
{
  namespace nanopb
  {
    ///////////////////////////////////////////////
    // string
    ///////////////////////////////////////////////
    bool encode_string_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      if (!pb_encode_tag_for_field(stream, field))
        return false;

      auto* str = static_cast<std::string*>(*arg);
      return pb_encode_string(stream, (pb_byte_t*)(str->data()), str->size()); // NOLINT(*-pro-type-cstyle-cast)
    }

    void encode_string(pb_callback_t& pb_callback, const std::string& str)
    {
      if (str.empty()) return;

      pb_callback.funcs.encode = &encode_string_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = (void*)(&str);
    }

    bool decode_string_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      size_t len = stream->bytes_left;
      auto* tgt_string = static_cast<std::string*>(*arg);
      tgt_string->resize(len);

      return pb_read(stream, (pb_byte_t*)(tgt_string->data()), tgt_string->size()); // NOLINT(*-pro-type-cstyle-cast)
    }

    void decode_string(pb_callback_t& pb_callback, std::string& str)
    {
      pb_callback.funcs.decode = &decode_string_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = &str;
    }

    bool encode_int_to_string_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      if (!pb_encode_tag_for_field(stream, field))
        return false;

      auto* int_value = static_cast<uint64_t*>(*arg);
      auto integer_value_as_string = std::to_string(*int_value);
      return pb_encode_string(stream, (pb_byte_t*)(integer_value_as_string.data()), integer_value_as_string.size()); // NOLINT(*-pro-type-cstyle-cast)
    }

    void encode_int_to_string(pb_callback_t& pb_callback, const uint64_t& int_argument)
    {
      pb_callback.funcs.encode = &encode_int_to_string_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = (void*)(&int_argument);
    }


    bool decode_int_from_string_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      size_t len = stream->bytes_left;
      auto* tgt_integer = static_cast<uint64_t*>(*arg);

      std::string intermediate_string;
      intermediate_string.resize(len);

      auto ret = pb_read(stream, (pb_byte_t*)(intermediate_string.data()), intermediate_string.size()); // NOLINT(*-pro-type-cstyle-cast)
      *tgt_integer = std::stoull(intermediate_string);
      return ret;
    }

    void decode_int_from_string(pb_callback_t& pb_callback, uint64_t& int_argument)
    {
      pb_callback.funcs.decode = &decode_int_from_string_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = (void*)(&int_argument);
    }

    ///////////////////////////////////////////////
    // bytes
    ///////////////////////////////////////////////
    bool encode_bytes_field_nano_bytes(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      if (!pb_encode_tag_for_field(stream, field))
        return false;

      auto* bytes = static_cast<SNanoBytes*>(*arg);
      return pb_encode_string(stream, (pb_byte_t*)bytes->content, bytes->length);
    }

    void encode_bytes(pb_callback_t& pb_callback, const SNanoBytes& nano_bytes)
    {
      if (nano_bytes.length == 0) return;

      pb_callback.funcs.encode = &encode_bytes_field_nano_bytes; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = (void*)(&nano_bytes);
    }

    bool encode_bytes_field_vec(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      if (!pb_encode_tag_for_field(stream, field))
        return false;

      auto* vec = static_cast<std::vector<char>*>(*arg);
      return pb_encode_string(stream, (pb_byte_t*)vec->data(), vec->size()); // NOLINT(*-pro-type-cstyle-cast)
    }

    void encode_bytes(pb_callback_t& pb_callback, const std::vector<char>& vec)
    {
      pb_callback.funcs.encode = &encode_bytes_field_vec; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = (void*)(&vec);
    }

    bool decode_bytes_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      size_t len = stream->bytes_left;
      auto* tgt_vector = static_cast<std::vector<char>*>(*arg);
      tgt_vector->resize(len);

      return pb_read(stream, (pb_byte_t*)(tgt_vector->data()), tgt_vector->size()); // NOLINT(*-pro-type-cstyle-cast)
    }

    void decode_bytes(pb_callback_t& pb_callback, std::vector<char>& vec)
    {
      pb_callback.funcs.decode = &decode_bytes_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = &vec;
    }

    ///////////////////////////////////////////////
    // list<string>
    ///////////////////////////////////////////////
    bool encode_string_vector_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      auto* str_list = static_cast<Util::CExpandingVector<std::string>*>(*arg);

      for (const auto& str : *str_list)
      {
        if (!pb_encode_tag_for_field(stream, field))
        {
          return false;
        }

        if (!pb_encode_string(stream, (pb_byte_t*)str.c_str(), str.size())) // NOLINT(*-pro-type-cstyle-cast)
        {
          return false;
        }
      }

      return true;
    }

    bool decode_string_vector_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      auto* tgt_list = static_cast<Util::CExpandingVector<std::string>*>(*arg);
      auto& tgt_string = tgt_list->push_back();

      size_t len = stream->bytes_left;
      tgt_string.resize(len);

      if (!pb_read(stream, (pb_byte_t*)(tgt_string.data()), tgt_string.size())) // NOLINT(*-pro-type-cstyle-cast)
        return false;

      return true;
    }

    ///////////////////////////////////////////////
    // registration_layer
    ///////////////////////////////////////////////
    bool encode_registration_layer_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      auto* layer_vec = static_cast<Util::CExpandingVector<eCAL::Registration::TLayer>*>(*arg);

      for (auto layer : *layer_vec)
      {
        if (!pb_encode_tag_for_field(stream, field))
        {
          return false;
        }

        eCAL_pb_TransportLayer pb_layer = eCAL_pb_TransportLayer_init_default;
        pb_layer.type    = static_cast<eCAL_pb_eTransportLayerType>(layer.type);
        pb_layer.version = layer.version;
        pb_layer.enabled = layer.enabled;
        pb_layer.active  = layer.active;

        // layer
        pb_layer.has_par_layer = true;
        
        // udp layer parameter
        pb_layer.par_layer.has_layer_par_udpmc = false;

        // tcp layer parameter
        pb_layer.par_layer.has_layer_par_tcp = true;
        pb_layer.par_layer.layer_par_tcp.port = layer.par_layer.layer_par_tcp.port;

        // shm layer parameter
        pb_layer.par_layer.has_layer_par_shm = true;
        pb_layer.par_layer.layer_par_shm.memory_file_list.funcs.encode = &encode_string_vector_field; // NOLINT(*-pro-type-union-access)
        pb_layer.par_layer.layer_par_shm.memory_file_list.arg          = (void*)(&layer.par_layer.layer_par_shm.memory_file_list);

        if (!pb_encode_submessage(stream, eCAL_pb_TransportLayer_fields, &pb_layer))
        {
          return false;
        }
      }

      return true;
    }

    void encode_registration_layer(pb_callback_t& pb_callback, const Util::CExpandingVector<eCAL::Registration::TLayer>& layer_vec)
    {
      pb_callback.funcs.encode = &encode_registration_layer_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = (void*)(&layer_vec);
    }

    bool decode_registration_layer_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      eCAL_pb_TransportLayer             pb_layer = eCAL_pb_TransportLayer_init_default;
      auto* tgt_vector = static_cast<Util::CExpandingVector<eCAL::Registration::TLayer>*>(*arg);
      auto& layer = tgt_vector->push_back();

      // decode shm layer parameter
      pb_layer.par_layer.layer_par_shm.memory_file_list.funcs.decode = &decode_string_vector_field; // NOLINT(*-pro-type-union-access)
      pb_layer.par_layer.layer_par_shm.memory_file_list.arg          = (void*)(&layer.par_layer.layer_par_shm.memory_file_list);

      if (!pb_decode(stream, eCAL_pb_TransportLayer_fields, &pb_layer))
      {
        return false;
      }

      // apply layer values
      layer.type    = static_cast<eCAL::eTLayerType>(pb_layer.type);
      layer.version = pb_layer.version;
      layer.enabled = pb_layer.enabled;
      layer.active  = pb_layer.active;

      // apply tcp layer parameter
      layer.par_layer.layer_par_tcp.port = pb_layer.par_layer.layer_par_tcp.port;

      return true;
    }

    void decode_registration_layer(pb_callback_t& pb_callback, Util::CExpandingVector<eCAL::Registration::TLayer>& layer_vec)
    {
      pb_callback.funcs.decode = &decode_registration_layer_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = &layer_vec;
    }

    ///////////////////////////////////////////////
    // service_methods
    ///////////////////////////////////////////////
    bool encode_service_methods_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      auto* method_vec = static_cast<eCAL::Util::CExpandingVector<eCAL::Service::Method>*>(*arg);

      for (const auto& method : *method_vec)
      {
        if (!pb_encode_tag_for_field(stream, field))
        {
          return false;
        }

        eCAL_pb_Method pb_method = eCAL_pb_Method_init_default;
        encode_string(pb_method.method_name, method.method_name);
        encode_string(pb_method.req_type, method.req_type);
        encode_string(pb_method.resp_type, method.resp_type);
        encode_string(pb_method.req_desc, method.req_desc);
        encode_string(pb_method.resp_desc, method.resp_desc);

        pb_method.has_request_datatype_information = true;
        encode_string(pb_method.request_datatype_information.name, method.request_datatype_information.name);
        encode_string(pb_method.request_datatype_information.encoding, method.request_datatype_information.encoding);
        encode_string(pb_method.request_datatype_information.descriptor_information, method.request_datatype_information.descriptor);

        pb_method.has_response_datatype_information = true;
        encode_string(pb_method.response_datatype_information.name, method.response_datatype_information.name);
        encode_string(pb_method.response_datatype_information.encoding, method.response_datatype_information.encoding);
        encode_string(pb_method.response_datatype_information.descriptor_information, method.response_datatype_information.descriptor);

        pb_method.call_count = method.call_count;

        if (!pb_encode_submessage(stream, eCAL_pb_Method_fields, &pb_method))
        {
          return false;
        }
      }

      return true;
    }

    void encode_service_methods(pb_callback_t& pb_callback, const eCAL::Util::CExpandingVector<eCAL::Service::Method>& method_vec)
    {
      pb_callback.funcs.encode = &encode_service_methods_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = (void*)(&method_vec);
    }

    bool decode_service_methods_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      eCAL_pb_Method pb_method = eCAL_pb_Method_init_default;
      auto* method_vec = static_cast<eCAL::Util::CExpandingVector<eCAL::Service::Method>*>(*arg);
      auto& method = method_vec->push_back();

      // decode method parameter
      decode_string(pb_method.method_name, method.method_name);
      decode_string(pb_method.req_type, method.req_type);
      decode_string(pb_method.req_desc, method.req_desc);
      decode_string(pb_method.resp_type, method.resp_type);
      decode_string(pb_method.resp_desc, method.resp_desc);

      decode_string(pb_method.request_datatype_information.name, method.request_datatype_information.name);
      decode_string(pb_method.request_datatype_information.encoding, method.request_datatype_information.encoding);
      decode_string(pb_method.request_datatype_information.descriptor_information, method.request_datatype_information.descriptor);

      decode_string(pb_method.response_datatype_information.name, method.response_datatype_information.name);
      decode_string(pb_method.response_datatype_information.encoding, method.response_datatype_information.encoding);
      decode_string(pb_method.response_datatype_information.descriptor_information, method.response_datatype_information.descriptor);

      // decode it
      if (!pb_decode(stream, eCAL_pb_Method_fields, &pb_method))
      {
        return false;
      }

      // apply method values
      method.call_count = pb_method.call_count;

      return true;
    }

    void decode_service_methods(pb_callback_t& pb_callback, eCAL::Util::CExpandingVector<eCAL::Service::Method>& method_vec)
    {
      pb_callback.funcs.decode = &decode_service_methods_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = &method_vec;
    }
  }
}
