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
 * @file   ecal_serialize_logging.cpp
 * @brief  eCAL logging (de)serialization
**/

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "nanopb/logging.pb.h"

#include "ecal_serialize_common.h"
#include "ecal_serialize_logging.h"

#include <cstddef>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace
{
  /////////////////////////////////////////////////////////////////////////////////
  // eCAL::Logging::LogMessage
  /////////////////////////////////////////////////////////////////////////////////
  void PrepareEncoding(const eCAL::Logging::SLogMessage& log_message_, eCAL_pb_LogMessage& pb_log_message_)
  {
    // time
    pb_log_message_.time = log_message_.time;
    // hname
    eCAL::nanopb::encode_string(pb_log_message_.hname, log_message_.hname);
    // pid
    pb_log_message_.pid = log_message_.pid;
    // pname
    eCAL::nanopb::encode_string(pb_log_message_.pname, log_message_.pname);
    // uname
    eCAL::nanopb::encode_string(pb_log_message_.uname, log_message_.uname);
    // level
    pb_log_message_.level = log_message_.level;
    // content
    eCAL::nanopb::encode_string(pb_log_message_.content, log_message_.content);
  }

  size_t LogMessageStruct2PbLogMessage(const eCAL::Logging::SLogMessage& log_message_, eCAL_pb_LogMessage& pb_log_message_)
  {
    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    PrepareEncoding(log_message_, pb_log_message_);

    ///////////////////////////////////////////////
    // evaluate byte size
    ///////////////////////////////////////////////
    pb_ostream_t pb_sizestream = { nullptr, nullptr, 0, 0, nullptr};
    pb_encode(&pb_sizestream, eCAL_pb_LogMessage_fields, &pb_log_message_);

    // return encoding byte size
    return pb_sizestream.bytes_written;
  }

  template <typename T>
  bool LogMessageStruct2Buffer(const eCAL::Logging::SLogMessage& log_message_, T& target_buffer_)
  {
    target_buffer_.clear();

    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    eCAL_pb_LogMessage pb_log_message = eCAL_pb_LogMessage_init_default;
    size_t target_size = LogMessageStruct2PbLogMessage(log_message_, pb_log_message);

    ///////////////////////////////////////////////
    // encode it
    ///////////////////////////////////////////////
    target_buffer_.resize(target_size);
    pb_ostream_t pb_ostream;
    pb_ostream = pb_ostream_from_buffer((pb_byte_t*)(target_buffer_.data()), target_buffer_.size());
    if (!pb_encode(&pb_ostream, eCAL_pb_LogMessage_fields, &pb_log_message))
    {
      std::cerr << "NanoPb eCAL::Logging::LogMessage encode failed: " << pb_ostream.errmsg << '\n';
    }
    else
    {
      return true;
    }

    return false;
  }

  void PrepareDecoding(eCAL_pb_LogMessage& pb_log_message_, eCAL::Logging::SLogMessage& log_message_)
  {
    // initialize
    pb_log_message_ = eCAL_pb_LogMessage_init_default;

    ///////////////////////////////////////////////
    // assign decoder
    ///////////////////////////////////////////////

    // hname
    eCAL::nanopb::decode_string(pb_log_message_.hname, log_message_.hname);
    // pname
    eCAL::nanopb::decode_string(pb_log_message_.pname, log_message_.pname);
    // uname
    eCAL::nanopb::decode_string(pb_log_message_.uname, log_message_.uname);
    // content
    eCAL::nanopb::decode_string(pb_log_message_.content, log_message_.content);
  }

  void AssignValues(const eCAL_pb_LogMessage& pb_log_message_, eCAL::Logging::SLogMessage& log_message_)
  {
    ///////////////////////////////////////////////
    // assign values
    ///////////////////////////////////////////////
    // time
    log_message_.time = pb_log_message_.time;
    // pid
    log_message_.pid = pb_log_message_.pid;
    // level
    log_message_.level = static_cast<eCAL_Logging_eLogLevel>(pb_log_message_.level);
  }

  bool Buffer2LogMessageStruct(const char* data_, size_t size_, eCAL::Logging::SLogMessage& log_message_)
  {
    if (data_ == nullptr) return false;
    if (size_ == 0)       return false;

    // initialize
    eCAL_pb_LogMessage pb_log_message = eCAL_pb_LogMessage_init_default;

    ///////////////////////////////////////////////
    // prepare sample for decoding
    ///////////////////////////////////////////////
    PrepareDecoding(pb_log_message, log_message_);

    ///////////////////////////////////////////////
    // decode it
    ///////////////////////////////////////////////
    pb_istream_t pb_istream;
    pb_istream = pb_istream_from_buffer((pb_byte_t*)data_, size_); // NOLINT(*-pro-type-cstyle-cast)
    if (!pb_decode(&pb_istream, eCAL_pb_LogMessage_fields, &pb_log_message))
    {
      std::cerr << "NanoPb eCAL::Logging::LogMessage decode failed: " << pb_istream.errmsg << '\n';
      return false;
    }

    ///////////////////////////////////////////////
    // assign sample values
    ///////////////////////////////////////////////
    AssignValues(pb_log_message, log_message_);

    return true;
  }

  /////////////////////////////////////////////////////////////////////////////////
  // eCAL::Logging::LogMessageList
  /////////////////////////////////////////////////////////////////////////////////
  bool encode_log_message_list_field(pb_ostream_t* stream, const pb_field_iter_t* field, void* const* arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    auto* sample_list = static_cast<std::list<eCAL::Logging::SLogMessage>*>(*arg);

    for (const auto& sample : *sample_list)
    {
      // encode sample tag
      if (!pb_encode_tag_for_field(stream, field))
      {
        return false;
      }

      // encode single sample
      eCAL_pb_LogMessage pb_log_message = eCAL_pb_LogMessage_init_default;
      PrepareEncoding(sample, pb_log_message);

      // encode submessage
      if (!pb_encode_submessage(stream, eCAL_pb_LogMessage_fields, &pb_log_message))
      {
        return false;
      }
    }

    return true;
  }

  size_t LogMessageListStruct2PbLogMessageList(const eCAL::Logging::SLogging& log_message_list_, eCAL_pb_LogMessageList& pb_log_message_list_)
  {
    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    pb_log_message_list_.log_messages.funcs.encode = &encode_log_message_list_field; // NOLINT(*-pro-type-union-access)
    pb_log_message_list_.log_messages.arg = (void*)(&log_message_list_.log_messages);

    ///////////////////////////////////////////////
    // evaluate byte size
    ///////////////////////////////////////////////
    pb_ostream_t pb_sizestream = { nullptr, nullptr, 0, 0, nullptr};
    pb_encode(&pb_sizestream, eCAL_pb_LogMessageList_fields, &pb_log_message_list_);

    // return encoding byte size
    return pb_sizestream.bytes_written;
  }

  template <typename T>
  bool LogMessageListStruct2Buffer(const eCAL::Logging::SLogging& log_message_list_, T& target_buffer_)
  {
    ///////////////////////////////////////////////
    // prepare sample for encoding
    ///////////////////////////////////////////////
    eCAL_pb_LogMessageList pb_log_message_list = eCAL_pb_LogMessageList_init_default;
    size_t target_size = LogMessageListStruct2PbLogMessageList(log_message_list_, pb_log_message_list);

    ///////////////////////////////////////////////
    // encode it
    ///////////////////////////////////////////////
    target_buffer_.resize(target_size);
    pb_ostream_t pb_ostream;
    pb_ostream = pb_ostream_from_buffer((pb_byte_t*)(target_buffer_.data()), target_buffer_.size());
    if (!pb_encode(&pb_ostream, eCAL_pb_LogMessageList_fields, &pb_log_message_list))
    {
      std::cerr << "NanoPb eCAL::Logging::LogMessageList encode failed: " << pb_ostream.errmsg << '\n';
    }
    else
    {
      return true;
    }

    return false;
  }

  bool decode_log_message_list_field(pb_istream_t* stream, const pb_field_iter_t* /*field*/, void** arg)
  {
    if (arg == nullptr)  return false;
    if (*arg == nullptr) return false;

    eCAL_pb_LogMessage pb_log_message = eCAL_pb_LogMessage_init_default;
    eCAL::Logging::SLogMessage sample{};

    // prepare sample for decoding
    PrepareDecoding(pb_log_message, sample);

    // decode it
    if (!pb_decode(stream, eCAL_pb_LogMessage_fields, &pb_log_message))
    {
      return false;
    }

    // apply sample values
    AssignValues(pb_log_message, sample);

    // add sample to list
    auto* sample_list = static_cast<std::list<eCAL::Logging::SLogMessage>*>(*arg);
    sample_list->push_back(sample);

    return true;
  }

  bool Buffer2LogMessageListStruct(const char* data_, size_t size_, eCAL::Logging::SLogging& log_message_list_)
  {
    if (data_ == nullptr) return false;
    if (size_ == 0)       return false;

    // initialize
    eCAL_pb_LogMessageList pb_log_message_list = eCAL_pb_LogMessageList_init_default;

    ///////////////////////////////////////////////
    // prepare sample for decoding
    ///////////////////////////////////////////////
    pb_log_message_list.log_messages.funcs.decode = &decode_log_message_list_field; // NOLINT(*-pro-type-union-access)
    pb_log_message_list.log_messages.arg = &log_message_list_.log_messages;

    ///////////////////////////////////////////////
    // decode it
    ///////////////////////////////////////////////
    pb_istream_t pb_istream;
    pb_istream = pb_istream_from_buffer((pb_byte_t*)data_, size_); // NOLINT(*-pro-type-cstyle-cast)
    if (!pb_decode(&pb_istream, eCAL_pb_LogMessageList_fields, &pb_log_message_list))
    {
      std::cerr << "NanoPb eCAL::Logging::LogMessageList decode failed: " << pb_istream.errmsg << '\n';
      return false;
    }

    return true;
  }
}

namespace eCAL
{
  // log message - serialize/deserialize
  bool SerializeToBuffer(const Logging::SLogMessage& source_sample_, std::vector<char>& target_buffer_)
  {
    return LogMessageStruct2Buffer(source_sample_, target_buffer_);
  }

  bool SerializeToBuffer(const Logging::SLogMessage& source_sample_, std::string& target_buffer_)
  {
    return LogMessageStruct2Buffer(source_sample_, target_buffer_);
  }

  bool DeserializeFromBuffer(const char* data_, size_t size_, Logging::SLogMessage& target_sample_)
  {
    return Buffer2LogMessageStruct(data_, size_, target_sample_);
  }

  bool SerializeToBuffer(const Logging::SLogging& source_sample_, std::vector<char>& target_buffer_)
  {
    target_buffer_.clear();
    return LogMessageListStruct2Buffer(source_sample_, target_buffer_);
  }

  bool SerializeToBuffer(const Logging::SLogging& source_sample_, std::string& target_buffer_)
  {
    target_buffer_.clear();
    return LogMessageListStruct2Buffer(source_sample_, target_buffer_);
  }

  bool DeserializeFromBuffer(const char* data_, size_t size_, Logging::SLogging& target_sample_)
  {
    return Buffer2LogMessageListStruct(data_, size_, target_sample_);
  }
}
