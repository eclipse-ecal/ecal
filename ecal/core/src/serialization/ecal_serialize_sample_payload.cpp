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
 * @file   ecal_serialize_sample_payload.cpp
 * @brief  eCAL payload sample (de)serialization
**/

#include "ecal_serialize_common.h"
#include "ecal_struct_sample_payload.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include <ecal/core/pb/ecal.pbftags.h>
#include <ecal/core/pb/layer.pbftags.h>
#include <ecal/core/pb/topic.pbftags.h>
#include <protozero/pbf_writer.hpp>
#include <protozero/buffer_vector.hpp>
#include <protozero/pbf_reader.hpp>
#include <protozero/ecal_helper.h>

namespace
{
  template<typename Writer>
  void SerializePayload(Writer& writer, const ::eCAL::Payload::Payload& payload)
  {
    const char* payload_addr = nullptr;
    size_t      payload_size = 0;
    switch (payload.type)
    {
    case eCAL::Payload::pl_raw:
      payload_addr = payload.raw_addr;
      payload_size = payload.raw_size;
      break;
    case eCAL::Payload::pl_vec:
      payload_addr = payload.vec.data();
      payload_size = payload.vec.size();
      break;
    default:
      break;
    }
    if ((payload_addr != nullptr) && (payload_size > 0))
    {
      writer.add_bytes(+eCAL::pb::Content::optional_bytes_payload, payload_addr, payload_size);
    }
  }

  template<typename Writer>
  void SerializePayloadSample(Writer& writer, const ::eCAL::Payload::Sample& sample)
  {
    // we need to properly match the enums / make sure that they have the same values
    writer.add_enum(+eCAL::pb::Sample::optional_enum_cmd_type, sample.cmd_type);
    // Additional information is not written to the content but the topic field.
    {
      Writer topic_writer{ writer, +eCAL::pb::Sample::optional_message_topic };
      topic_writer.add_string(+eCAL::pb::Topic::optional_string_topic_name, sample.topic_info.topic_name);
      topic_writer.add_string(+eCAL::pb::Topic::optional_string_topic_id, std::to_string(sample.topic_info.topic_id));
      topic_writer.add_int32(+eCAL::pb::Topic::optional_int32_process_id, sample.topic_info.process_id);
      topic_writer.add_string(+eCAL::pb::Topic::optional_string_host_name, sample.topic_info.host_name);
    }
    {
      Writer content_writer{ writer, +eCAL::pb::Sample::optional_message_content };
      content_writer.add_int64(+eCAL::pb::Content::optional_int64_id, sample.content.id);
      content_writer.add_int64(+eCAL::pb::Content::optional_int64_clock, sample.content.clock);
      content_writer.add_int64(+eCAL::pb::Content::optional_int64_time,  sample.content.time);
      content_writer.add_int32(+eCAL::pb::Content::optional_int32_size, sample.content.size);
      SerializePayload(content_writer, sample.content.payload);
      content_writer.add_int64(+eCAL::pb::Content::optional_int64_hash,  sample.content.hash);
    }
    writer.add_bytes(+eCAL::pb::Sample::optional_bytes_padding, sample.padding.data(), sample.padding.size());
  }

  void DeserializeTopicInfo(protozero::pbf_reader& reader, ::eCAL::Payload::TopicInfo& topic_info)
  {
    while (reader.next())
    {
      switch (reader.tag())
      {
      case +eCAL::pb::Topic::optional_string_topic_name:
        AssignString(reader, topic_info.topic_name);
        break;
      case +eCAL::pb::Topic::optional_string_topic_id:
        {
          static thread_local std::string topic_id_string;
          AssignString(reader, topic_id_string);
          topic_info.topic_id = std::stoull(topic_id_string);
        }
        break;
      case +eCAL::pb::Topic::optional_int32_process_id:
        topic_info.process_id = reader.get_int32();
        break;
      case +eCAL::pb::Topic::optional_string_host_name:
        AssignString(reader, topic_info.host_name);
        break;
      default:
        reader.skip();
        break;
      }
    }
  }

  void DeserializeContent(::protozero::pbf_reader& reader, ::eCAL::Payload::Content& content)
  {
    // We always need to set it (at least for the current testcases...)
    content.payload.type = eCAL::Payload::pl_vec;
    
    while (reader.next())
    {
      switch (reader.tag())
      {
      case +eCAL::pb::Content::optional_int64_id:
        content.id = reader.get_int64();
        break;
      case +eCAL::pb::Content::optional_int64_clock:
        content.clock = reader.get_int64();
        break;
      case +eCAL::pb::Content::optional_int64_time:
        content.time = reader.get_int64();
        break;
      case +eCAL::pb::Content::optional_int32_size:
        content.size = reader.get_int32();
        break;
      case +eCAL::pb::Content::optional_bytes_payload:
        AssignBytes(reader, content.payload.vec);
        break;
      case +eCAL::pb::Content::optional_int64_hash:
        content.hash = reader.get_int64();
        break;
      default:
        reader.skip();
        break;
      }
    }
  }

  void DeserializePayloadSample(::protozero::pbf_reader& reader, ::eCAL::Payload::Sample& sample)
  {
    while (reader.next())
    {
      switch (reader.tag())
      {
      case +eCAL::pb::Sample::optional_enum_cmd_type:
        sample.cmd_type = static_cast<eCAL::eCmdType>(reader.get_enum());
        break;
      case +eCAL::pb::Sample::optional_message_topic:
        AssignMessage(reader, sample.topic_info, DeserializeTopicInfo);
        break;
      case +eCAL::pb::Sample::optional_message_content:
        AssignMessage(reader, sample.content, DeserializeContent);
        break;
      case +eCAL::pb::Sample::optional_bytes_padding:
        AssignBytes(reader, sample.padding);
        break;
      default:
        reader.skip();
      }
    }
  }
}


namespace eCAL
{
  namespace protozero
  {
    bool SerializeToBuffer(const Payload::Sample& source_sample_, std::vector<char>& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::basic_pbf_writer<std::vector<char>> writer{ target_buffer_ };
      SerializePayloadSample(writer, source_sample_);
      return true;
    }
  
    bool SerializeToBuffer(const Payload::Sample& source_sample_, std::string& target_buffer_)
    {
      target_buffer_.clear();
      ::protozero::pbf_writer writer{ target_buffer_ };
      SerializePayloadSample(writer, source_sample_);
      return true;
    }
  
    bool DeserializeFromBuffer(const char* data_, size_t size_, Payload::Sample& target_sample_)
    {
      try
      {
        // @todo we clear the target sample before deserialization, but Payload::Sample doesn't have a clear function
        // we should check this;
        ::protozero::pbf_reader message{ data_, size_ };
        DeserializePayloadSample(message, target_sample_);
        return true;
      }
      catch (const std::exception& exception)
      {
        LogDeserializationException(exception, "eCAL::Payload::Sample");
        return false;
      }
    }
  }
}
