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
 * @file   ecal_serialize_sample_payload.cpp
 * @brief  eCAL payload sample (de)serialization
**/

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "nanopb/ecal.pb.h"

#include "ecal_serialize_common.h"
#include "ecal_struct_sample_payload.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace
{
  void CreatePayloadStruct(const eCAL::Payload::Sample& payload_, eCAL::nanopb::SNanoBytes& nano_bytes_)
  {
    // extract payload
    // payload may be stored as std::vector<char> or raw pointer + size
    const char* payload_addr = nullptr;
    size_t      payload_size = 0;
    switch (payload_.content.payload.type)
    {
    case eCAL::Payload::pl_raw:
      payload_addr = payload_.content.payload.raw_addr;
      payload_size = payload_.content.payload.raw_size;
      break;
    case eCAL::Payload::pl_vec:
      payload_addr = payload_.content.payload.vec.data();
      payload_size = payload_.content.payload.vec.size();
      break;
    default:
      break;
    }

    // topic content payload
    if ((payload_addr != nullptr) && (payload_size > 0))
    {
      nano_bytes_.content = (pb_byte_t*)(payload_addr); // NOLINT(*-pro-type-cstyle-cast)
      nano_bytes_.length  = payload_size;
    }
  }
    
  // TODO: The size must be a multiple of 8.
  size_t PayloadStruct2PbSample(const eCAL::Payload::Sample& payload_, const eCAL::nanopb::SNanoBytes& nano_bytes_, eCAL_pb_Sample& pb_sample_)
  {
    // command type
    pb_sample_.cmd_type = static_cast<eCAL_pb_eCmdType>(payload_.cmd_type);

    // topic information
    pb_sample_.has_topic = true;
    // hname
    eCAL::nanopb::encode_string(pb_sample_.topic.hname, payload_.topic.hname);
    // tid
    eCAL::nanopb::encode_string(pb_sample_.topic.tid, payload_.topic.tid);
    // tname
    eCAL::nanopb::encode_string(pb_sample_.topic.tname, payload_.topic.tname);

    // topic content
    pb_sample_.has_content = true;
    pb_sample_.content.id    = payload_.content.id;
    pb_sample_.content.clock = payload_.content.clock;
    pb_sample_.content.time  = payload_.content.time;
    pb_sample_.content.hash  = payload_.content.hash;
    pb_sample_.content.size  = payload_.content.size;

    // topic content payload
    eCAL::nanopb::encode_bytes(pb_sample_.content.payload, nano_bytes_);

    // padding
    eCAL::nanopb::encode_bytes(pb_sample_.padding, payload_.padding);

    ///////////////////////////////////////////////
    // evaluate byte size
    ///////////////////////////////////////////////
    pb_ostream_t pb_sizestream = { nullptr, nullptr, 0, 0, nullptr};
    pb_encode(&pb_sizestream, eCAL_pb_Sample_fields, &pb_sample_);

    // return encoding byte size
    return pb_sizestream.bytes_written;
  }

  template <typename T>
  bool PayloadStruct2Buffer(const eCAL::Payload::Sample& payload_, T& target_buffer_)
  {
    target_buffer_.clear();

    // create payload helper struct
    eCAL::nanopb::SNanoBytes nano_bytes;
    CreatePayloadStruct(payload_, nano_bytes);

    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    eCAL_pb_Sample pb_sample = eCAL_pb_Sample_init_default;
    size_t target_size = PayloadStruct2PbSample(payload_, nano_bytes, pb_sample);

    ///////////////////////////////////////////////
    // encode it
    ///////////////////////////////////////////////
    target_buffer_.resize(target_size);
    pb_ostream_t pb_ostream;
    pb_ostream = pb_ostream_from_buffer((pb_byte_t*)(target_buffer_.data()), target_buffer_.size());
    if (!pb_encode(&pb_ostream, eCAL_pb_Sample_fields, &pb_sample))
    {
      std::cerr << "NanoPb eCAL::Payload::Sample encode failed: " << pb_ostream.errmsg << '\n';
    }
    else
    {
      return true;
    }
    
    return false;
  }

  bool Buffer2PayloadStruct(const char* data_, size_t size_, eCAL::Payload::Sample& payload_)
  {
    if (data_ == nullptr) return false;
    if (size_ == 0)       return false;

    // initialize
    eCAL_pb_Sample pb_sample = eCAL_pb_Sample_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_sample.topic.hname, payload_.topic.hname);
    // tid
    eCAL::nanopb::decode_string(pb_sample.topic.tid, payload_.topic.tid);
    // tname
    eCAL::nanopb::decode_string(pb_sample.topic.tname, payload_.topic.tname);
    // topic content payload
    payload_.content.payload.type = eCAL::Payload::pl_vec;
    eCAL::nanopb::decode_bytes(pb_sample.content.payload, payload_.content.payload.vec);

    // padding
    eCAL::nanopb::decode_bytes(pb_sample.padding, payload_.padding);

    ///////////////////////////////////////////////
    // decode it
    ///////////////////////////////////////////////
    pb_istream_t pb_istream;
    pb_istream = pb_istream_from_buffer((pb_byte_t*)data_, size_); // NOLINT(*-pro-type-cstyle-cast)
    if (!pb_decode(&pb_istream, eCAL_pb_Sample_fields, &pb_sample))
    {
      std::cerr << "NanoPb eCAL::Payload::Sample decode failed: " << pb_istream.errmsg << '\n';
      return false;
    }

    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // command type
    payload_.cmd_type = static_cast<eCAL::eCmdType>(pb_sample.cmd_type);

    // topic content
    payload_.content.id    = pb_sample.content.id;
    payload_.content.clock = pb_sample.content.clock;
    payload_.content.time  = pb_sample.content.time;
    payload_.content.hash  = pb_sample.content.hash;
    payload_.content.size  = pb_sample.content.size;

    return true;
  }
}

namespace eCAL
{
  bool SerializeToBuffer(const Payload::Sample& source_sample_, std::vector<char>& target_buffer_)
  {
    return PayloadStruct2Buffer(source_sample_, target_buffer_);
  }

  bool SerializeToBuffer(const Payload::Sample& source_sample_, std::string& target_buffer_)
  {
    return PayloadStruct2Buffer(source_sample_, target_buffer_);
  }

  bool DeserializeFromBuffer(const char* data_, size_t size_, Payload::Sample& target_sample_)
  {
    return Buffer2PayloadStruct(data_, size_, target_sample_);
  }
}
