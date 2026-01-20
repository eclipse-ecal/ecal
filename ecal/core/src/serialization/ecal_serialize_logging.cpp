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

#include "ecal_serialize_common.h"
#include "ecal_serialize_logging.h"

#include <cstddef>
#include <iostream>
#include <list>
#include <string>
#include <vector>

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


