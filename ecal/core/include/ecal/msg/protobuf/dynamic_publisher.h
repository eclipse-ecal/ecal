/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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
 * @file   dynamic_publisher.h
 * @brief  dynamic protobuf message publisher
**/

#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <sstream>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/ecal_proto_hlp.h>
#include <ecal/msg/publisher.h>
#include <stdexcept>
#include <string>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief eCAL dynamic protobuf subscriber.
    **/
    class CDynamicPublisher : public eCAL::v5::CMsgPublisher<google::protobuf::Message>
    {
    public:
      /**
       * @brief Constructor.
       *
       * @param topic_name_  Unique topic name.
       * @param msg_         Protobuf message object.
       * @param config_      Optional configuration parameters.
      **/
      CDynamicPublisher(const std::string& topic_name_, const std::shared_ptr<google::protobuf::Message>& msg_, const eCAL::Publisher::Configuration& config_ = eCAL::GetPublisherConfiguration())
        : CMsgPublisher<google::protobuf::Message>(topic_name_, GetTopicInformationFromMessage(msg_.get()), config_)
        , m_msg{ msg_ } {}

      /**
       * @brief Constructor.
       *
       * @param topic_name_       Unique topic name.
       * @param proto_type_name_  Protobuf message type name.
      **/
      CDynamicPublisher(const std::string& topic_name_, const std::string& proto_type_name_)
        : CMsgPublisher<google::protobuf::Message>(topic_name_, GetTopicInformationFromMessage(CreateMessageByName(proto_type_name_).get()))
        , m_msg{ CreateMessageByName(proto_type_name_) } {}

      CDynamicPublisher(const CDynamicPublisher&) = delete;
      CDynamicPublisher& operator=(const CDynamicPublisher&) = delete;
      CDynamicPublisher(CDynamicPublisher&&) = default;
      CDynamicPublisher& operator=(CDynamicPublisher&&) = default;

      /**
       * @brief  Send message.
       *
       * @param time_  Optional time stamp.
       *
       * @return  Number of bytes sent.
      **/
      size_t Send(long long time_ = -1)
      {
        return CMsgPublisher::Send(*m_msg, time_);
      }

      /**
       * @brief  Get underlying protobuf message object.
       *
       * @return  Message object <google::protobuf::Message>.
      **/
      std::shared_ptr<google::protobuf::Message> GetMessage()
      {
        return m_msg;
      }

      /**
       * @brief  Get underlying protobuf message object as T.
       *
       * @return  Message object <T>.
      **/
      template<typename T>
      std::shared_ptr<T> GetAs()
      {
        return std::dynamic_pointer_cast<T>(m_msg);
      }

      size_t Send(const google::protobuf::Message& msg_, long long time_ = -1) = delete;

    private:
      SDataTypeInformation GetDataTypeInformation() const override
      {
        return GetTopicInformationFromMessage(m_msg.get());
      }

      size_t GetSize(const google::protobuf::Message& msg_) const override
      {
#if GOOGLE_PROTOBUF_VERSION >= 3001000
        return ((size_t)msg_.ByteSizeLong());
#else
        return ((size_t)msg_.ByteSize());
#endif
      }

      bool Serialize(const google::protobuf::Message& msg_, char* buffer_, size_t size_) const override
      {
        return (msg_.SerializeToArray((void*)buffer_, (int)size_));
      }

      static SDataTypeInformation GetTopicInformationFromMessage(const google::protobuf::Message* msg_ptr_)
      {
        assert(msg_ptr_);
        if(msg_ptr_ == nullptr) return SDataTypeInformation();

        SDataTypeInformation data_type_info;
        data_type_info.encoding   = "proto";
        data_type_info.name       = msg_ptr_->GetTypeName();
        data_type_info.descriptor = GetDescriptorFromMessage(msg_ptr_);
        return data_type_info;
      }

      static std::string GetDescriptorFromMessage(const google::protobuf::Message* msg_ptr_)
      {
        assert(msg_ptr_);
        if(msg_ptr_ == nullptr) return "";

        const google::protobuf::Descriptor* desc = msg_ptr_->GetDescriptor();
        google::protobuf::FileDescriptorSet pset;

        if (eCAL::protobuf::GetFileDescriptor(desc, pset))
        {
          std::string desc_s = pset.SerializeAsString();
          return (desc_s);
        }

        return ("");
      }

      static std::shared_ptr<google::protobuf::Message> CreateMessageByName(const std::string& type_name_)
      {
        const google::protobuf::Descriptor* desc =
          google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name_);

        if (desc == nullptr)
        {
          std::stringstream errmsg;
          errmsg << "Unable to find message type " << type_name_ << std::endl;
          throw std::runtime_error(errmsg.str());
        }

        std::shared_ptr<google::protobuf::Message> message{
            google::protobuf::MessageFactory::generated_factory()->GetPrototype(desc)->New() };

        if (message == nullptr)
        {
          std::stringstream errmsg;
          errmsg << "Unable to create new message from type " << type_name_ << std::endl;
          throw std::runtime_error(errmsg.str());
        }

        return message;
      }

      std::shared_ptr<google::protobuf::Message> m_msg;
    };
  }
}
