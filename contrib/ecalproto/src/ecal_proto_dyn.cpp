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
 * dynamic protobuf message decoder
**/

#include <ecal/protobuf/ecal_proto_dyn.h>

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>

namespace eCAL
{
namespace protobuf
{

  class ParserErrorCollector : public google::protobuf::io::ErrorCollector
  {
  public:
    ParserErrorCollector() {}
    ~ParserErrorCollector() {}

    std::string Get() { return(m_ss.str()); }

    // Indicates that there was an error in the input at the given line and
    // column numbers.  The numbers are zero-based, so you may want to add
    // 1 to each before printing them.
    void AddError(int line_, int column_, const std::string& msg_)
    {
      Add(line_, column_, "ERROR: " + msg_);
    }

    // Indicates that there was a warning in the input at the given line and
    // column numbers.  The numbers are zero-based, so you may want to add
    // 1 to each before printing them.
    void AddWarning(int line_, int column_, const std::string& msg_)
    {
      Add(line_, column_, "WARNING: " + msg_);
    }

  private:
    void Add(int line_, int column_, const std::string& msg_)
    {
      m_ss << line_ << ":" << column_ << " " << msg_ << std::endl;
    }

    std::stringstream m_ss;
  };

  class DescriptorErrorCollector : public google::protobuf::DescriptorPool::ErrorCollector
  {
  public:
    DescriptorErrorCollector() {}
    ~DescriptorErrorCollector() {}

    std::string Get() { return(m_ss.str()); }

    void AddError(
      const std::string& filename,                    // File name in which the error occurred.
      const std::string& element_name,                // Full name of the erroneous element.
      const google::protobuf::Message* descriptor,    // Descriptor of the erroneous element.
      ErrorLocation location,                         // One of the location constants, above.
      const std::string& message                      // Human-readable error message.
    )
    {
      Add(filename, element_name, descriptor, location, "ERROR: " + message);
    }

    void AddWarning(
      const std::string& filename,                    // File name in which the error occurred.
      const std::string& element_name,                // Full name of the erroneous element.
      const google::protobuf::Message* descriptor,    // Descriptor of the erroneous element.
      ErrorLocation location,                         // One of the location constants, above.
      const std::string& message                      // Human-readable error message.
    )
    {
      Add(filename, element_name, descriptor, location, "WARNING: " + message);
    }

  private:
    void Add(
      const std::string& filename,
      const std::string& element_name,
      const google::protobuf::Message* /*descriptor*/,
      ErrorLocation location,
      const std::string& message
    )
    {
      m_ss << filename << " " << element_name << " " << location << " " << message << std::endl;
    }

    std::stringstream m_ss;
  };

  google::protobuf::Message* CProtoDynDecoder::GetProtoMessageFromFile(const std::string& proto_filename_, const std::string& msg_type_, std::string& error_s_)
  {
    google::protobuf::FileDescriptorProto proto;
    if (!GetFileDescriptorFromFile(proto_filename_, &proto, error_s_)) return(nullptr);
    google::protobuf::FileDescriptorSet pset;
    google::protobuf::FileDescriptorProto* pdesc = pset.add_file();
    pdesc->CopyFrom(proto);
    return(GetProtoMessageFromDescriptorSet(pset, msg_type_, error_s_));
  }

  google::protobuf::Message* CProtoDynDecoder::GetProtoMessageFromString(const std::string& proto_string_, const std::string& msg_type_, std::string& error_s_)
  {
    google::protobuf::FileDescriptorProto proto;
    if (!GetFileDescriptorFromString(proto_string_, &proto, error_s_)) return(nullptr);
    google::protobuf::FileDescriptorSet pset;
    google::protobuf::FileDescriptorProto* pdesc = pset.add_file();
    pdesc->CopyFrom(proto);
    return(GetProtoMessageFromDescriptorSet(pset, msg_type_, error_s_));
  }

  google::protobuf::Message* CProtoDynDecoder::GetProtoMessageFromDescriptor(const google::protobuf::FileDescriptorProto& proto_desc_, const std::string& msg_type_, std::string& error_s_)
  {
    google::protobuf::FileDescriptorSet pset;
    google::protobuf::FileDescriptorProto* pdesc = pset.add_file();
    pdesc->CopyFrom(proto_desc_);
    return(GetProtoMessageFromDescriptorSet(pset, msg_type_, error_s_));
  }

  google::protobuf::Message* CProtoDynDecoder::GetProtoMessageFromDescriptorSet(const google::protobuf::FileDescriptorSet& proto_desc_set_, const std::string& msg_type_, std::string& error_s_)
  {
    // suppose you want to parse a message type with a specific type name.
    DescriptorErrorCollector error_collector;
    const google::protobuf::FileDescriptor* file_desc = nullptr;
    for (auto it = 0; it < proto_desc_set_.file_size(); ++it)
    {
      file_desc = m_descriptor_pool.BuildFileCollectingErrors(proto_desc_set_.file(it), &error_collector);
      if (file_desc == NULL)
      {
        error_s_ = error_collector.Get();
        return(NULL);
      }
      if ((file_desc->message_type_count() > 0) &&
        (file_desc->message_type(0)->name() == msg_type_))
      {
        break;
      }
    }
    if (file_desc == NULL)
    {
      error_s_ = "Cannot get file descriptor of message: " + msg_type_;
      return(NULL);
    }
    const google::protobuf::Descriptor* message_desc = file_desc->FindMessageTypeByName(msg_type_);
    if (message_desc == NULL)
    {
      error_s_ = "Cannot get message descriptor of message: " + msg_type_;
      return(NULL);
    }

    const google::protobuf::Message* prototype_msg = m_message_factory.GetPrototype(message_desc);
    if (prototype_msg == NULL)
    {
      error_s_ = "Cannot create prototype message from message descriptor";
      return(NULL);
    }
    google::protobuf::Message* proto_msg = prototype_msg->New();
    if (proto_msg == NULL)
    {
      error_s_ = "Failed in prototype_msg->New(); to create mutable message";
      return(NULL);
    }
    return(proto_msg);
  }

  google::protobuf::DescriptorPool* CProtoDynDecoder::GetDescriptorPool()
  {
    return &m_descriptor_pool;
  }

  bool CProtoDynDecoder::GetFileDescriptorFromFile(const std::string& proto_filename_, google::protobuf::FileDescriptorProto* file_desc_proto_, std::string& error_s_)
  {
    using namespace google::protobuf;
    using namespace google::protobuf::io;
    using namespace google::protobuf::compiler;

    std::ifstream fs(proto_filename_);
    if (!fs.is_open())
    {
      std::cout << "Cannot open .proto file: " << proto_filename_;
      return(false);
    }
    std::stringstream ss;
    ss << fs.rdbuf();

    std::string proto_str = ss.str();

    return(GetFileDescriptorFromString(proto_str, file_desc_proto_, error_s_));
  }

  bool CProtoDynDecoder::GetFileDescriptorFromString(const std::string& proto_string_, google::protobuf::FileDescriptorProto* file_desc_proto_, std::string& error_s_)
  {
    using namespace google::protobuf;
    using namespace google::protobuf::io;
    using namespace google::protobuf::compiler;

    std::stringstream ss;
    ss << proto_string_;
    IstreamInputStream proto_string_input_stream(&ss);

    Tokenizer tokenizer(&proto_string_input_stream, NULL);
    Parser parser;
    ParserErrorCollector error_collector;
    parser.RecordErrorsTo(&error_collector);
    if (!parser.Parse(&tokenizer, file_desc_proto_))
    {
      error_s_ = error_collector.Get();
      return(false);
    }

    // here we walk around a bug in protocol buffers that
    // |Parser::Parse| does not set name (.proto filename) in
    // file_desc_proto.
    if (!file_desc_proto_->has_name())
    {
      file_desc_proto_->set_name("foo.proto");
    }

    return(true);
  }
}
}
