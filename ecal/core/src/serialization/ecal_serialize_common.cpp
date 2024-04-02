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
 * @file   ecal_serialize_common.cpp
 * @brief  eCAL common (de)serialization
**/

#include "nanopb/ecal.pb.h"
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
    // map<string,string>
    ///////////////////////////////////////////////
    bool encode_map_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      auto* attr_map = static_cast<std::map<std::string, std::string>*>(*arg);
      for (const auto& iter : *attr_map)
      {
        if (!pb_encode_tag_for_field(stream, field))
        {
          return false;
        }

        eCAL_pb_Topic_AttrEntry pb_attr_map = eCAL_pb_Topic_AttrEntry_init_default;
        encode_string(pb_attr_map.key, iter.first);
        encode_string(pb_attr_map.value, iter.second);

        if (!pb_encode_submessage(stream, eCAL_pb_Topic_AttrEntry_fields, &pb_attr_map))
        {
          return false;
        }
      }

      return true;
    }

    void encode_map(pb_callback_t& pb_callback, const std::map<std::string, std::string>& str_map)
    {
      pb_callback.funcs.encode = &encode_map_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = (void*)(&str_map);
    }

    bool decode_map_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      eCAL_pb_Topic_AttrEntry attr_entry = eCAL_pb_Topic_AttrEntry_init_default;
      std::string key;
      std::string value;
      decode_string(attr_entry.key, key);
      decode_string(attr_entry.value, value);

      if (!pb_decode(stream, eCAL_pb_Topic_AttrEntry_fields, &attr_entry))
      {
        return false;
      }

      auto* tgt_map = static_cast<std::map<std::string, std::string>*>(*arg);
      (*tgt_map)[key] = value;

      return true;
    }

    void decode_map(pb_callback_t& pb_callback, std::map<std::string, std::string>& str_map)
    {
      pb_callback.funcs.decode = &decode_map_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = &str_map;
    }

    ///////////////////////////////////////////////
    // list<string>
    ///////////////////////////////////////////////
    bool encode_string_list_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      auto* str_list = static_cast<std::list<std::string>*>(*arg);

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

    bool decode_string_list_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      size_t len = stream->bytes_left;
      std::string tgt_string;
      tgt_string.resize(len);

      if (!pb_read(stream, (pb_byte_t*)(tgt_string.data()), tgt_string.size())) // NOLINT(*-pro-type-cstyle-cast)
        return false;

      auto* tgt_list = static_cast<std::list<std::string>*>(*arg);
      tgt_list->push_back(tgt_string);

      return true;
    }

    ///////////////////////////////////////////////
    // registration_layer
    ///////////////////////////////////////////////
    bool encode_registration_layer_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      auto* layer_vec = static_cast<std::vector<eCAL::Registration::TLayer>*>(*arg);

      for (auto layer : *layer_vec)
      {
        if (!pb_encode_tag_for_field(stream, field))
        {
          return false;
        }

        eCAL_pb_TLayer pb_layer = eCAL_pb_TLayer_init_default;
        pb_layer.type      = static_cast<eCAL_pb_eTLayerType>(layer.type);
        pb_layer.version   = layer.version;
        pb_layer.confirmed = layer.confirmed;

        // layer
        pb_layer.has_par_layer = true;
        
        // udp layer parameter
        pb_layer.par_layer.has_layer_par_udpmc = false;

        // tcp layer parameter
        pb_layer.par_layer.has_layer_par_tcp = true;
        pb_layer.par_layer.layer_par_tcp.port = layer.par_layer.layer_par_tcp.port;

        // shm layer parameter
        pb_layer.par_layer.has_layer_par_shm = true;
        pb_layer.par_layer.layer_par_shm.memory_file_list.funcs.encode = &encode_string_list_field; // NOLINT(*-pro-type-union-access)
        pb_layer.par_layer.layer_par_shm.memory_file_list.arg          = (void*)(&layer.par_layer.layer_par_shm.memory_file_list);

        if (!pb_encode_submessage(stream, eCAL_pb_TLayer_fields, &pb_layer))
        {
          return false;
        }
      }

      return true;
    }

    void encode_registration_layer(pb_callback_t& pb_callback, const std::vector<eCAL::Registration::TLayer>& layer_vec)
    {
      pb_callback.funcs.encode = &encode_registration_layer_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = (void*)(&layer_vec);
    }

    bool decode_registration_layer_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      eCAL_pb_TLayer             pb_layer = eCAL_pb_TLayer_init_default;
      eCAL::Registration::TLayer layer{};

      // decode shm layer parameter
      pb_layer.par_layer.layer_par_shm.memory_file_list.funcs.decode = &decode_string_list_field; // NOLINT(*-pro-type-union-access)
      pb_layer.par_layer.layer_par_shm.memory_file_list.arg          = (void*)(&layer.par_layer.layer_par_shm.memory_file_list);

      if (!pb_decode(stream, eCAL_pb_TLayer_fields, &pb_layer))
      {
        return false;
      }

      // apply layer values
      layer.type      = static_cast<eCAL::eTLayerType>(pb_layer.type);
      layer.version   = pb_layer.version;
      layer.confirmed = pb_layer.confirmed;

      // apply tcp layer parameter
      layer.par_layer.layer_par_tcp.port = pb_layer.par_layer.layer_par_tcp.port;

      // add layer
      auto* tgt_vector = static_cast<std::vector<eCAL::Registration::TLayer>*>(*arg);
      tgt_vector->push_back(layer);

      return true;
    }

    void decode_registration_layer(pb_callback_t& pb_callback, std::vector<eCAL::Registration::TLayer>& layer_vec)
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

      auto* method_vec = static_cast<std::vector<eCAL::Service::Method>*>(*arg);

      for (const auto& method : *method_vec)
      {
        if (!pb_encode_tag_for_field(stream, field))
        {
          return false;
        }

        eCAL_pb_Method pb_method = eCAL_pb_Method_init_default;
        encode_string(pb_method.mname, method.mname);
        encode_string(pb_method.req_type, method.req_type);
        encode_string(pb_method.resp_type, method.resp_type);
        encode_string(pb_method.req_desc, method.req_desc);
        encode_string(pb_method.resp_desc, method.resp_desc);
        pb_method.call_count = method.call_count;

        if (!pb_encode_submessage(stream, eCAL_pb_Method_fields, &pb_method))
        {
          return false;
        }
      }

      return true;
    }

    void encode_service_methods(pb_callback_t& pb_callback, const std::vector<eCAL::Service::Method>& method_vec)
    {
      pb_callback.funcs.encode = &encode_service_methods_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = (void*)(&method_vec);
    }

    bool decode_service_methods_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
    {
      if (arg == nullptr)  return false;
      if (*arg == nullptr) return false;

      eCAL_pb_Method pb_method = eCAL_pb_Method_init_default;
      eCAL::Service::Method method{};

      // decode method parameter
      decode_string(pb_method.mname, method.mname);
      decode_string(pb_method.req_type, method.req_type);
      decode_string(pb_method.req_desc, method.req_desc);
      decode_string(pb_method.resp_type, method.resp_type);
      decode_string(pb_method.resp_desc, method.resp_desc);

      // decode it
      if (!pb_decode(stream, eCAL_pb_Method_fields, &pb_method))
      {
        return false;
      }

      // apply method values
      method.call_count = pb_method.call_count;

      // add method to vector
      auto* method_vec = static_cast<std::vector<eCAL::Service::Method>*>(*arg);
      method_vec->emplace_back(method);

      return true;
    }

    void decode_service_methods(pb_callback_t& pb_callback, std::vector<eCAL::Service::Method>& method_vec)
    {
      pb_callback.funcs.decode = &decode_service_methods_field; // NOLINT(*-pro-type-union-access)
      pb_callback.arg = &method_vec;
    }
  }
}
