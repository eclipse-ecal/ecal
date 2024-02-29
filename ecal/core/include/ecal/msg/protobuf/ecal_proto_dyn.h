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
 * @file   ecal_proto_dyn.h
 * @brief  dynamic protobuf message decoder
**/

#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/compiler/parser.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <ecal/msg/protobuf/ecal_proto_hlp.h>

namespace eCAL
{
  namespace protobuf
  {
    /**
      * @brief eCAL dynamic protobuf decoder.
      *
      * The CProtoDynDecoder class is used to decode a protobuf message using protobuf reflection interface. The returned
      * google message objects are owned by CProtoDynDecoder and freed on destruction.
      *
      * @code
      *            // create dynamic decoder
      *            std::string error_s;
      *            eCAL::CProtoDynDecoder decoder;
      *            std::shared_ptr<google::protobuf::Message> msg_obj(decoder.GetProtoMessageFromFile("foo.proto", "FooMessage", error_s));
      *
      *            // receive a message
      *            std::string msg_s = ReceiveMessageFromAnyWhere("foo");
      *
      *            // decode message
      *            msg_obj->ParseFromString(msg_s);
      *
      *            // print message
      *            std::cout << msg_obj->DebugString() << std::endl;
      * @endcode
    **/
    class CProtoDynDecoder
    {
    public:
      /**
        * @brief Create message from proto file.
        * 
        * Note: Ownership of the google::protobuf::Message pointer is passed to the caller.
        *
        * @param       proto_filename_  Proto file name.
        * @param       msg_type_        Type name.
        * @param [out] error_s_         Error string.
        *
        * @return google message object or nullptr if generation failed (details see error_s_)
      **/
      google::protobuf::Message* GetProtoMessageFromFile(const std::string& proto_filename_, const std::string& msg_type_, std::string& error_s_);

      /**
        * @brief Create message from proto string.
        *
        * Note: Ownership of the google::protobuf::Message pointer is passed to the caller.
        *
        * @param       proto_string_  Proto string.
        * @param       msg_type_      Type name.
        * @param [out] error_s_       Error string.
        *
        * @return google message object or nullptr if generation failed (details see error_s_)
      **/
      google::protobuf::Message* GetProtoMessageFromString(const std::string& proto_string_, const std::string& msg_type_, std::string& error_s_);

      /**
        * @brief Create message from proto descriptor.
        *
        * Note: Ownership of the google::protobuf::Message pointer is passed to the caller.
        *
        * @param       proto_desc_  Proto descriptor.
        * @param       msg_type_    Type name.
        * @param [out] error_s_     Error string.
        *
        * @return google message object or nullptr if generation failed (details see error_s_)
      **/
      google::protobuf::Message* GetProtoMessageFromDescriptor(const google::protobuf::FileDescriptorProto& proto_desc_, const std::string& msg_type_, std::string& error_s_);

      /**
        * @brief Create message from serialized proto descriptor string.
        *
        * Note: Ownership of the google::protobuf::Message pointer is passed to the caller.
        *
        * @param       msg_desc_    Serialized message descriptor.
        * @param       msg_type_    Type name.
        * @param [out] error_s_     Error string.
        *
        * @return google message object or nullptr if generation failed (details see error_s_)
      **/
      google::protobuf::Message* GetProtoMessageFromDescriptor(const std::string& msg_desc_, const std::string& msg_type_, std::string& error_s_);

      /**
        * @brief Create message from proto descriptor set.
        *
        * Note: Ownership of the google::protobuf::Message pointer is passed to the caller.
        *
        * @param       proto_desc_set_  Proto descriptor set.
        * @param       msg_type_        Type name.
        * @param [out] error_s_         Error string.
        *
        * @return google message object or nullptr if generation failed (details see error_s_)
      **/
      google::protobuf::Message* GetProtoMessageFromDescriptorSet(const google::protobuf::FileDescriptorSet& proto_desc_set_, const std::string& msg_type_, std::string& error_s_);

      /**
        * @brief Returns the DescriptorPool member variable
      **/
      google::protobuf::DescriptorPool* GetDescriptorPool();

      /**
        * @brief Create proto descriptor from proto file.
        *
        * @param       proto_filename_   Proto file name.
        * @param [out] file_desc_proto_  Type name.
        * @param [out] error_s_          Error string.
        *
        * @return true if succeeded otherwise false (details see error_s_)
      **/
      bool GetFileDescriptorFromFile(const std::string& proto_filename_, google::protobuf::FileDescriptorProto* file_desc_proto_, std::string& error_s_);

      /**
        * @brief Create proto descriptor from proto string.
        *
        * @param       proto_string_     Proto string.
        * @param [out] file_desc_proto_  Type name.
        * @param [out] error_s_          Error string.
        *
        * @return true if succeeded otherwise false (details see error_s_)
      **/
      bool GetFileDescriptorFromString(const std::string& proto_string_, google::protobuf::FileDescriptorProto* file_desc_proto_, std::string& error_s_);

    protected:
      google::protobuf::DescriptorPool        m_descriptor_pool;
      google::protobuf::DynamicMessageFactory m_message_factory;
    };

    class ParserErrorCollector : public google::protobuf::io::ErrorCollector
    {
    public:
      ParserErrorCollector() = default;
      ~ParserErrorCollector() override = default;

      std::string Get() { return (m_ss.str()); }

      // Indicates that there was an error in the input at the given line and
      // column numbers.  The numbers are zero-based, so you may want to add
      // 1 to each before printing them.
      void AddError(int line_, int column_, const std::string& msg_) override
      {
        Add(line_, column_, "ERROR: " + msg_);
      }

      // Indicates that there was a warning in the input at the given line and
      // column numbers.  The numbers are zero-based, so you may want to add
      // 1 to each before printing them.
      void AddWarning(int line_, int column_, const std::string& msg_) override
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
      DescriptorErrorCollector() = default;
      ~DescriptorErrorCollector() override {}

      std::string Get() { return (m_ss.str()); }

      void AddError(
        const std::string& filename,                    // File name in which the error occurred.
        const std::string& element_name,                // Full name of the erroneous element.
        const google::protobuf::Message* descriptor,    // Descriptor of the erroneous element.
        ErrorLocation location,                         // One of the location constants, above.
        const std::string& message                      // Human-readable error message.
      ) override
      {
        Add(filename, element_name, descriptor, location, "ERROR: " + message);
      }

      void AddWarning(
        const std::string& filename,                    // File name in which the error occurred.
        const std::string& element_name,                // Full name of the erroneous element.
        const google::protobuf::Message* descriptor,    // Descriptor of the erroneous element.
        ErrorLocation location,                         // One of the location constants, above.
        const std::string& message                      // Human-readable error message.
      ) override
      {
        Add(filename, element_name, descriptor, location, "WARNING: " + message);
      }

    private:
      void Add(
          const std::string& filename,
          const std::string& element_name,
          const google::protobuf::Message* /*descriptor*/,
          ErrorLocation location,
          const std::string& message)
      {
        m_ss << filename << " " << element_name << " " << location << " " << message << std::endl;
      }

      std::stringstream m_ss;
    };

    inline google::protobuf::Message* CProtoDynDecoder::GetProtoMessageFromFile(const std::string& proto_filename_, const std::string& msg_type_, std::string& error_s_)
    {
      google::protobuf::FileDescriptorProto proto;
      if (!GetFileDescriptorFromFile(proto_filename_, &proto, error_s_)) return (nullptr);
      google::protobuf::FileDescriptorSet pset;
      google::protobuf::FileDescriptorProto* pdesc = pset.add_file();
      pdesc->CopyFrom(proto);
      return (GetProtoMessageFromDescriptorSet(pset, msg_type_, error_s_));
    }

    inline google::protobuf::Message* CProtoDynDecoder::GetProtoMessageFromString(const std::string& proto_string_, const std::string& msg_type_, std::string& error_s_)
    {
      google::protobuf::FileDescriptorProto proto;
      if (!GetFileDescriptorFromString(proto_string_, &proto, error_s_)) return (nullptr);
      google::protobuf::FileDescriptorSet pset;
      google::protobuf::FileDescriptorProto* pdesc = pset.add_file();
      pdesc->CopyFrom(proto);
      return (GetProtoMessageFromDescriptorSet(pset, msg_type_, error_s_));
    }

    inline google::protobuf::Message* CProtoDynDecoder::GetProtoMessageFromDescriptor(const google::protobuf::FileDescriptorProto& proto_desc_, const std::string& msg_type_, std::string& error_s_)
    {
      google::protobuf::FileDescriptorSet pset;
      google::protobuf::FileDescriptorProto* pdesc = pset.add_file();
      pdesc->CopyFrom(proto_desc_);
      return (GetProtoMessageFromDescriptorSet(pset, msg_type_, error_s_));
    }

    inline google::protobuf::Message* CProtoDynDecoder::GetProtoMessageFromDescriptor(const std::string& msg_desc_, const std::string& msg_type_, std::string& error_s_)
    {
      // create file descriptor set
      google::protobuf::FileDescriptorSet pset;
      if (!pset.ParseFromString(msg_desc_))
      {
        error_s_ = "Cannot get file descriptor of message: " + msg_type_;
        return (nullptr);
      }

      // create message object
      google::protobuf::Message* proto_msg = GetProtoMessageFromDescriptorSet(pset, msg_type_, error_s_);
      if (proto_msg == nullptr)
      {
        return (nullptr);
      }

      return (proto_msg);
    }

    inline google::protobuf::Message* CProtoDynDecoder::GetProtoMessageFromDescriptorSet(const google::protobuf::FileDescriptorSet& proto_desc_set_, const std::string& msg_type_, std::string& error_s_)
    {
      // check if msg_type_ is available in descriptor pool
      const google::protobuf::Descriptor* desc = m_descriptor_pool.FindMessageTypeByName(msg_type_);
      if (desc != nullptr)
      {
        const google::protobuf::Message* prototype_msg = m_message_factory.GetPrototype(desc);
        if (prototype_msg != nullptr)
        {
          // ownership passed to caller here !
          google::protobuf::Message* proto_msg = prototype_msg->New();
          if (proto_msg == nullptr)
          {
            error_s_ = "Failed in prototype_msg->New(); to create mutable message";
            return (nullptr);
          }
          return (proto_msg);
        }
      }

      // suppose you want to parse a message type with a specific type name.
      DescriptorErrorCollector error_collector;
      const google::protobuf::FileDescriptor* file_desc = nullptr;
      for (auto it = 0; it < proto_desc_set_.file_size(); ++it)
      {
        file_desc = m_descriptor_pool.BuildFileCollectingErrors(proto_desc_set_.file(it), &error_collector);
        if (file_desc == nullptr)
        {
          error_s_ = error_collector.Get();
          return (nullptr);
        }
        if ((file_desc->message_type_count() > 0) &&
          (file_desc->message_type(0)->name() == msg_type_))
        {
          break;
        }
      }
      if (file_desc == nullptr)
      {
        error_s_ = "Cannot get file descriptor of message: " + msg_type_;
        return (nullptr);
      }
      const google::protobuf::Descriptor* message_desc = file_desc->FindMessageTypeByName(msg_type_);
      if (message_desc == nullptr)
      {
        error_s_ = "Cannot get message descriptor of message: " + msg_type_;
        return (nullptr);
      }

      const google::protobuf::Message* prototype_msg = m_message_factory.GetPrototype(message_desc);
      if (prototype_msg == nullptr)
      {
        error_s_ = "Cannot create prototype message from message descriptor";
        return (nullptr);
      }

      // ownership passed to caller here !
      google::protobuf::Message* proto_msg = prototype_msg->New();
      if (proto_msg == nullptr)
      {
        error_s_ = "Failed in prototype_msg->New(); to create mutable message";
        return (nullptr);
      }
      return (proto_msg);
    }

    inline google::protobuf::DescriptorPool* CProtoDynDecoder::GetDescriptorPool()
    {
      return &m_descriptor_pool;
    }

    inline bool CProtoDynDecoder::GetFileDescriptorFromFile(const std::string& proto_filename_, google::protobuf::FileDescriptorProto* file_desc_proto_, std::string& error_s_)
    {
      std::ifstream fs(proto_filename_);
      if (!fs.is_open())
      {
        std::cout << "Cannot open .proto file: " << proto_filename_;
        return (false);
      }
      std::stringstream ss;
      ss << fs.rdbuf();

      std::string proto_str = ss.str();

      return (GetFileDescriptorFromString(proto_str, file_desc_proto_, error_s_));
    }

    inline bool CProtoDynDecoder::GetFileDescriptorFromString(const std::string& proto_string_, google::protobuf::FileDescriptorProto* file_desc_proto_, std::string& error_s_)
    {
      std::stringstream ss;
      ss << proto_string_;
      google::protobuf::io::IstreamInputStream proto_string_input_stream(&ss);

      google::protobuf::io::Tokenizer tokenizer(&proto_string_input_stream, nullptr);
      google::protobuf::compiler::Parser parser;
      ParserErrorCollector error_collector;
      parser.RecordErrorsTo(&error_collector);
      if (!parser.Parse(&tokenizer, file_desc_proto_))
      {
        error_s_ = error_collector.Get();
        return (false);
      }

      // here we walk around a bug in protocol buffers that
      // |Parser::Parse| does not set name (.proto filename) in
      // file_desc_proto.
      if (!file_desc_proto_->has_name())
      {
        file_desc_proto_->set_name("foo.proto");
      }

      return (true);
    }
  }
}
