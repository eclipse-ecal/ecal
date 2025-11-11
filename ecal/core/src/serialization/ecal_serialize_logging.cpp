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
 * @file   ecal_serialize_logging.cpp
 * @brief  eCAL logging (de)serialization
**/

#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "nanopb/ecal/core/pb/logging.npb.h"

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
    // host_name
    eCAL::nanopb::encode_string(pb_log_message_.host_name, log_message_.host_name);
    // process_id
    pb_log_message_.process_id = log_message_.process_id;
    // process_name
    eCAL::nanopb::encode_string(pb_log_message_.process_name, log_message_.process_name);
    // unit_name
    eCAL::nanopb::encode_string(pb_log_message_.unit_name, log_message_.unit_name);
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

    // host_name
    eCAL::nanopb::decode_string(pb_log_message_.host_name, log_message_.host_name);
    // process_name
    eCAL::nanopb::decode_string(pb_log_message_.process_name, log_message_.process_name);
    // unit_name
    eCAL::nanopb::decode_string(pb_log_message_.unit_name, log_message_.unit_name);
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
    // process_id
    log_message_.process_id = pb_log_message_.process_id;
    // level
    log_message_.level = static_cast<eCAL::Logging::eLogLevel>(pb_log_message_.level);
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
  namespace nanopb
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
}

///////////////////////////////////////////////////////////////////////////////


#include <ecal/core/pb/logging.pbftags.h>
#include <protozero/pbf_writer.hpp>
#include <protozero/buffer_vector.hpp>
#include <protozero/pbf_reader.hpp>
#include <protozero/ecal_helper.h>


namespace
{
  template <typename Writer>
  void SerializeLogMessage(Writer& writer, const eCAL::Logging::SLogMessage& log_message)
  {
    writer.add_int64(+eCAL::pb::LogMessage::optional_int64_time, log_message.time);
    writer.add_string(+eCAL::pb::LogMessage::optional_string_host_name, log_message.host_name);
    writer.add_int32(+eCAL::pb::LogMessage::optional_int32_process_id, log_message.process_id);
    writer.add_string(+eCAL::pb::LogMessage::optional_string_process_name, log_message.process_name);
    writer.add_string(+eCAL::pb::LogMessage::optional_string_unit_name, log_message.unit_name);
    writer.add_int32(+eCAL::pb::LogMessage::optional_int32_level, log_message.level);
    writer.add_string(+eCAL::pb::LogMessage::optional_string_content, log_message.content);
  }

  void DeserializeLogMessage(::protozero::pbf_reader& reader, eCAL::Logging::SLogMessage& log_message)
  {
    while (reader.next())
    {
      switch (reader.tag())
      {
      case +eCAL::pb::LogMessage::optional_int64_time:
        log_message.time = reader.get_int64();
        break;
      case +eCAL::pb::LogMessage::optional_string_host_name:
        AssignString(reader, log_message.host_name);
        break;
      case +eCAL::pb::LogMessage::optional_int32_process_id:
        log_message.process_id = reader.get_int32();
        break;
      case +eCAL::pb::LogMessage::optional_string_process_name:
        AssignString(reader, log_message.process_name);
        break;
      case +eCAL::pb::LogMessage::optional_string_unit_name:
        AssignString(reader, log_message.unit_name);
        break;
      case +eCAL::pb::LogMessage::optional_int32_level:
        log_message.level = static_cast<eCAL::Logging::eLogLevel>(reader.get_int32());
        break;
      case +eCAL::pb::LogMessage::optional_string_content:
        AssignString(reader, log_message.content);
        break;
      default:
        reader.skip();
        break;
      }
    }
  }

  template <typename Writer>
  void SerializeLogMessageList(Writer& writer, const eCAL::Logging::SLogging& log_message_list)
  {
    for (const auto& log_message : log_message_list.log_messages)
    {
      Writer log_message_writer{ writer, +eCAL::pb::LogMessageList::repeated_message_log_messages };
      SerializeLogMessage(log_message_writer, log_message);
    }
  }

  void DeserializeLogMessageList(::protozero::pbf_reader& reader, eCAL::Logging::SLogging& log_message_list)
  {
    while (reader.next())
    {
      switch (reader.tag())
      {
      case +eCAL::pb::LogMessageList::repeated_message_log_messages:
      {
        log_message_list.log_messages.emplace_back();
        eCAL::Logging::SLogMessage& log_message = log_message_list.log_messages.back();
        AssignMessage(reader, log_message, DeserializeLogMessage);
        break;
      }
      default:
        reader.skip();
        break;
      }
    }
  }
}

namespace eCAL
{
  namespace protozero
  {
    // log message - serialize/deserialize
    bool SerializeToBuffer(const Logging::SLogMessage& source_sample_, std::vector<char>& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::basic_pbf_writer<std::vector<char>> writer{ target_buffer_ };
      SerializeLogMessage(writer, source_sample_);
      return true;
    }
  
    bool SerializeToBuffer(const Logging::SLogMessage& source_sample_, std::string& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::pbf_writer writer{ target_buffer_ };
      SerializeLogMessage(writer, source_sample_);
      return true;
    }
  
    bool DeserializeFromBuffer(const char* data_, size_t size_, Logging::SLogMessage& target_sample_)
    {
      try
      {
        // @todo what about clearing the objects? do we expect empty objects?
        target_sample_ = Logging::SLogMessage{};
        ::protozero::pbf_reader message{ data_, size_ };
        DeserializeLogMessage(message, target_sample_);
        return true;
      }
      catch (const std::exception& exception)
      {
        LogDeserializationException(exception, "eCAL::Logging::SLogMessage");
        return false;
      }
    }
  
    // log message list - serialize/deserialize
    bool SerializeToBuffer(const Logging::SLogging& source_sample_, std::vector<char>& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::basic_pbf_writer<std::vector<char>> writer{ target_buffer_ };
      SerializeLogMessageList(writer, source_sample_);
      return true;
    }
  
    bool SerializeToBuffer(const Logging::SLogging& source_sample_, std::string& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::pbf_writer writer{ target_buffer_ };
      SerializeLogMessageList(writer, source_sample_);
      return true;
    }
  
    bool DeserializeFromBuffer(const char* data_, size_t size_, Logging::SLogging& target_sample_)
    {
      try
      {
        // @todo what about clearing the objects? do we expect empty objects?
        target_sample_ = Logging::SLogging{};
        ::protozero::pbf_reader message{ data_, size_ };
        DeserializeLogMessageList(message, target_sample_);
        return true;
      }
      catch (const std::exception& exception)
      {
        LogDeserializationException(exception, "eCAL::Logging::SLogging");
        return false;
      }
    }
  }
}


