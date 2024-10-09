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
 * @file   ecal_serialize_service.cpp
 * @brief  eCAL service (de)serialization
**/

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "nanopb/ecal.pb.h"

#include "ecal_serialize_common.h"
#include "ecal_serialize_service.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace
{
  size_t RequestStruct2PbRequest(const eCAL::Service::Request& request_, eCAL_pb_Request& pb_request_)
  {
    pb_request_ = eCAL_pb_Request_init_default;

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    pb_request_.has_header = true;

    // hname
    eCAL::nanopb::encode_string(pb_request_.header.hname, request_.header.hname);
    // sname
    eCAL::nanopb::encode_string(pb_request_.header.sname, request_.header.sname);
    // sid
    eCAL::nanopb::encode_string(pb_request_.header.sid, request_.header.sid);
    // mname
    eCAL::nanopb::encode_string(pb_request_.header.mname, request_.header.mname);
    // error
    eCAL::nanopb::encode_string(pb_request_.header.error, request_.header.error);
    // id
    pb_request_.header.id = request_.header.id;
    // state
    pb_request_.header.state = static_cast<eCAL_pb_ServiceHeader_eCallState>(request_.header.state);

    ///////////////////////////////////////////////
    // request
    ///////////////////////////////////////////////
    eCAL::nanopb::encode_string(pb_request_.request, request_.request);

    ///////////////////////////////////////////////
    // evaluate byte size
    ///////////////////////////////////////////////
    pb_ostream_t pb_sizestream = { nullptr, nullptr, 0, 0, nullptr};
    pb_encode(&pb_sizestream, eCAL_pb_Request_fields, &pb_request_);

    // return encoding byte size
    return pb_sizestream.bytes_written;
  }

  template <typename T>
  bool RequestStruct2Buffer(const eCAL::Service::Request& request_, T& target_buffer_)
  {
    target_buffer_.clear();

    ///////////////////////////////////////////////
    // prepare request for encoding
    ///////////////////////////////////////////////
    eCAL_pb_Request pb_request = eCAL_pb_Request_init_default;
    size_t target_size = RequestStruct2PbRequest(request_, pb_request);

    ///////////////////////////////////////////////
    // encode it
    ///////////////////////////////////////////////
    target_buffer_.resize(target_size);
    pb_ostream_t pb_ostream;
    pb_ostream = pb_ostream_from_buffer((pb_byte_t*)(target_buffer_.data()), target_buffer_.size());
    if (!pb_encode(&pb_ostream, eCAL_pb_Request_fields, &pb_request))
    {
      std::cerr << "NanoPb eCAL::Service::Request encode failed: " << pb_ostream.errmsg << '\n';
    }
    else
    {
      return true;
    }

    return false;
  }

  bool Buffer2RequestStruct(const char* data_, size_t size_, eCAL::Service::Request& request_)
  {
    if (data_ == nullptr) return false;
    if (size_ == 0)       return false;

    // initialize
    eCAL_pb_Request pb_request = eCAL_pb_Request_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_request.header.hname, request_.header.hname);
    // sname
    eCAL::nanopb::decode_string(pb_request.header.sname, request_.header.sname);
    // sid
    eCAL::nanopb::decode_string(pb_request.header.sid, request_.header.sid);
    // mname
    eCAL::nanopb::decode_string(pb_request.header.mname, request_.header.mname);
    // error
    eCAL::nanopb::decode_string(pb_request.header.error, request_.header.error);

    ///////////////////////////////////////////////
    // request
    ///////////////////////////////////////////////
    // request
    eCAL::nanopb::decode_string(pb_request.request, request_.request);

    ///////////////////////////////////////////////
    // decode it
    ///////////////////////////////////////////////
    pb_istream_t pb_istream;
    pb_istream = pb_istream_from_buffer((pb_byte_t*)data_, size_); // NOLINT(*-pro-type-cstyle-cast)
    if (!pb_decode(&pb_istream, eCAL_pb_Request_fields, &pb_request))
    {
      std::cerr << "NanoPb eCAL::Service::Request decode failed: " << pb_istream.errmsg << '\n';
      return false;
    }

    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    // id
    request_.header.id = pb_request.header.id;
    // state
    request_.header.state = static_cast<eCAL::Service::eMethodCallState>(pb_request.header.state);

    return true;
  }

  size_t ResponseStruct2PbResponse(const eCAL::Service::Response& response_, eCAL_pb_Response& pb_response_)
  {
    pb_response_ = eCAL_pb_Response_init_default;

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    pb_response_.has_header = true;

    // hname
    eCAL::nanopb::encode_string(pb_response_.header.hname, response_.header.hname);
    // sname
    eCAL::nanopb::encode_string(pb_response_.header.sname, response_.header.sname);
    // sid
    eCAL::nanopb::encode_string(pb_response_.header.sid, response_.header.sid);
    // mname
    eCAL::nanopb::encode_string(pb_response_.header.mname, response_.header.mname);
    // error
    eCAL::nanopb::encode_string(pb_response_.header.error, response_.header.error);
    // id
    pb_response_.header.id = response_.header.id;
    // state
    pb_response_.header.state = static_cast<eCAL_pb_ServiceHeader_eCallState>(response_.header.state);

    ///////////////////////////////////////////////
    // response
    ///////////////////////////////////////////////
    eCAL::nanopb::encode_string(pb_response_.response, response_.response);
    pb_response_.ret_state = response_.ret_state;

    ///////////////////////////////////////////////
    // evaluate byte size
    ///////////////////////////////////////////////
    pb_ostream_t pb_sizestream = { nullptr, nullptr, 0, 0, nullptr};
    pb_encode(&pb_sizestream, eCAL_pb_Response_fields, &pb_response_);

    // return encoding byte size
    return pb_sizestream.bytes_written;
  }

  template <typename T>
  bool ResponseStruct2Buffer(const eCAL::Service::Response& response_, T& target_buffer_)
  {
    target_buffer_.clear();

    ///////////////////////////////////////////////
    // prepare response for encoding
    ///////////////////////////////////////////////
    eCAL_pb_Response pb_response = eCAL_pb_Response_init_default;
    size_t target_size = ResponseStruct2PbResponse(response_, pb_response);

    ///////////////////////////////////////////////
    // encode it
    ///////////////////////////////////////////////
    target_buffer_.resize(target_size);
    pb_ostream_t pb_ostream;
    pb_ostream = pb_ostream_from_buffer((pb_byte_t*)(target_buffer_.data()), target_buffer_.size());
    if (!pb_encode(&pb_ostream, eCAL_pb_Response_fields, &pb_response))
    {
      std::cerr << "NanoPb eCAL::Service::Response encode failed: " << pb_ostream.errmsg << '\n';
    }
    else
    {
      return true;
    }

    return false;
  }

  bool Buffer2ResponseStruct(const char* data_, size_t size_, eCAL::Service::Response& response_)
  {
    if (data_ == nullptr) return false;
    if (size_ == 0)       return false;

    // initialize
    eCAL_pb_Response pb_response = eCAL_pb_Response_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    // hname
    eCAL::nanopb::decode_string(pb_response.header.hname, response_.header.hname);
    // sname
    eCAL::nanopb::decode_string(pb_response.header.sname, response_.header.sname);
    // sid
    eCAL::nanopb::decode_string(pb_response.header.sid, response_.header.sid);
    // mname
    eCAL::nanopb::decode_string(pb_response.header.mname, response_.header.mname);
    // error
    eCAL::nanopb::decode_string(pb_response.header.error, response_.header.error);

    ///////////////////////////////////////////////
    // response
    ///////////////////////////////////////////////
    // response
    eCAL::nanopb::decode_string(pb_response.response, response_.response);

    ///////////////////////////////////////////////
    // decode it
    ///////////////////////////////////////////////
    pb_istream_t pb_istream;
    pb_istream = pb_istream_from_buffer((pb_byte_t*)data_, size_); // NOLINT(*-pro-type-cstyle-cast)
    if (!pb_decode(&pb_istream, eCAL_pb_Response_fields, &pb_response))
    {
      std::cerr << "NanoPb eCAL::Service::Response decode failed: " << pb_istream.errmsg << '\n';
      return false;
    }

    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////

    ///////////////////////////////////////////////
    // header information
    ///////////////////////////////////////////////
    // id
    response_.header.id = pb_response.header.id;
    // state
    response_.header.state = static_cast<eCAL::Service::eMethodCallState>(pb_response.header.state);

    ///////////////////////////////////////////////
    // response
    ///////////////////////////////////////////////
    response_.ret_state = pb_response.ret_state;

    return true;
  }
}

namespace eCAL
{
  // service request - serialize/deserialize
  bool SerializeToBuffer(const Service::Request& source_sample_, std::vector<char>& target_buffer_)
  {
    return RequestStruct2Buffer(source_sample_, target_buffer_);
  }

  bool SerializeToBuffer(const Service::Request& source_sample_, std::string& target_buffer_)
  {
    return RequestStruct2Buffer(source_sample_, target_buffer_);
  }

  bool DeserializeFromBuffer(const char* data_, size_t size_, Service::Request& target_sample_)
  {
    return Buffer2RequestStruct(data_, size_, target_sample_);
  }

  // service response - serialize/deserialize
  bool SerializeToBuffer(const Service::Response& source_sample_, std::vector<char>& target_buffer_)
  {
    return ResponseStruct2Buffer(source_sample_, target_buffer_);
  }

  bool SerializeToBuffer(const Service::Response& source_sample_, std::string& target_buffer_)
  {
    return ResponseStruct2Buffer(source_sample_, target_buffer_);
  }

  bool DeserializeFromBuffer(const char* data_, size_t size_, Service::Response& target_sample_)
  {
    return Buffer2ResponseStruct(data_, size_, target_sample_);
  }
}
