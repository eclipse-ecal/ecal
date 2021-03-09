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
 * @brief  dynamic protobuf message decoder
**/

#pragma once

#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
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

#include <ecal/protobuf/ecal_proto_hlp.h>

namespace eCAL
{
namespace protobuf
{
  /**
    * @brief eCAL dynamic protobuf decoder.
    *
    * The CProtoDynDecoder class is used to decode a protobuf message using protobuf reflection interface.
    *
  **/
  /**
    * @code
    *            // create dynamic decoder
    *            std::string error_s;
    *            eCAL::CProtoDynDecoder decoder;
    *            google::protobuf::Message* msg_obj = decoder.GetProtoMessageFromFile("foo.proto", "FooMessage", error_s);
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
      * @param       proto_filename_  Proto file name.
      * @param       msg_type_        Type name.
      * @param [out] error_s_         Error string.
    **/
    google::protobuf::Message* GetProtoMessageFromFile(const std::string& proto_filename_, const std::string& msg_type_, std::string& error_s_);

    /**
      * @brief Create message from proto string.
      *
      * @param       proto_string_  Proto string.
      * @param       msg_type_      Type name.
      * @param [out] error_s_       Error string.
    **/
    google::protobuf::Message* GetProtoMessageFromString(const std::string& proto_string_, const std::string& msg_type_, std::string& error_s_);

    /**
      * @brief Create message from proto descriptor.
      *
      * @param       proto_desc_  Proto descriptor.
      * @param       msg_type_    Type name.
      * @param [out] error_s_     Error string.
    **/
    google::protobuf::Message* GetProtoMessageFromDescriptor(const google::protobuf::FileDescriptorProto& proto_desc_, const std::string& msg_type_, std::string& error_s_);

    /**
      * @brief Create message from proto descriptor set.
      *
      * @param       proto_desc_set_  Proto descriptor set.
      * @param       msg_type_        Type name.
      * @param [out] error_s_         Error string.
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
    **/
    static bool GetFileDescriptorFromFile(const std::string& proto_filename_, google::protobuf::FileDescriptorProto* file_desc_proto_, std::string& error_s_);

    /**
      * @brief Create proto descriptor from proto string.
      *
      * @param       proto_string_     Proto string.
      * @param [out] file_desc_proto_  Type name.
      * @param [out] error_s_          Error string.
    **/
    static bool GetFileDescriptorFromString(const std::string& proto_string_, google::protobuf::FileDescriptorProto* file_desc_proto_, std::string& error_s_);

  protected:
    google::protobuf::DescriptorPool         m_descriptor_pool;
    google::protobuf::DynamicMessageFactory  m_message_factory;
  };
}
}
