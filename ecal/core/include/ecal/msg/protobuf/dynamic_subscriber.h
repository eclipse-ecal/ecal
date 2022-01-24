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
 * @file   dynamic_subscriber.h
 * @brief  dynamic protobuf message subscriber
**/

#pragma once

#include <exception>
#include <sstream>
#include <ecal/ecal.h>
#include <ecal/protobuf/ecal_proto_dyn.h>
#include <ecal/msg/dynamic.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <google/protobuf/message.h>
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
    class CDynamicSubscriber
    {
    public:
      /**
       * @brief Constructor.
      **/
      CDynamicSubscriber();

      /**
       * @brief Constructor.
       *
       * @param topic_name_  Unique topic name.
      **/
      CDynamicSubscriber(const std::string& topic_name_);

      /**
       * @brief Destructor.
      **/
      virtual ~CDynamicSubscriber();

      /**
       * @brief Creates this object.
       *
       * @param topic_name_   Unique topic name.
       *
       * @return  true if it succeeds, false if it fails.
      **/
      void Create(const std::string& topic_name_);

      /**
       * @brief Destroys this object.
       *
       * @return  true if it succeeds, false if it fails.
      **/
      void Destroy();

      /**
       * @brief Query if this object is created.
       *
       * @return  true if created, false if not.
      **/
      bool IsCreated() { return(created); }

      /**
       * @brief eCAL protobuf message receive callback function
       *
       * @param topic_name_  Topic name of the data source (publisher).
       * @param msg_         Protobuf message content.
       * @param time_        Message time stamp.
      **/
      typedef std::function<void(const char* topic_name_, const google::protobuf::Message& msg_, long long time_)> ProtoMsgCallbackT;

      /**
       * @brief Add callback function for incoming receives.
       *
       * @param callback_  The callback function to add.
       *
       * @return  True if succeeded, false if not.
      **/
      bool AddReceiveCallback(ProtoMsgCallbackT callback_);

      /**
       * @brief get a Pointer to a temporary message that can be passed to receive
      **/
      google::protobuf::Message* getMessagePointer();

      /**
       * @brief Manually receive the next sample
      **/
      bool Receive(google::protobuf::Message& msg_, long long* time_ = nullptr, int rcv_timeout_ = 0);

      /**
       * @brief Remove callback function for incoming receives.
       *
       * @return  True if succeeded, false if not.
      **/
      bool RemReceiveCallback();

      /**
       * @brief Callback function in case an error occurs.
       *
       * @param error  The error message string.
      **/
      typedef std::function<void(const std::string& error)> ProtoErrorCallbackT;

      /**
       * @brief Add callback function in case an error occurs.
       *
       * @param callback_  The callback function to add.
       *
       * @return  True if succeeded, false if not.
      **/
      bool AddErrorCallback(ProtoErrorCallbackT callback_);

      /**
       * @brief Remove callback function in case an error occurs.
       *
       * @return  True if succeeded, false if not.
      **/
      bool RemErrorCallback();

    protected:
      void OnReceive(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_);

      google::protobuf::Message* CreateMessagePointer(const std::string& topic_name_);

      bool                              created;
      std::string                       topic_name;
      eCAL::protobuf::CProtoDynDecoder* msg_decoder;
      google::protobuf::Message*        msg_ptr;
      eCAL::CSubscriber                 msg_sub;
      ProtoMsgCallbackT                 msg_callback;
      ProtoErrorCallbackT               err_callback;

    private:
      // this object must not be copied.
      CDynamicSubscriber(const CDynamicSubscriber&);
      CDynamicSubscriber& operator=(const CDynamicSubscriber&);
    };
    /** @example proto_dyn.cpp
     * This is an example how to use CDynamicSubscriber to receive dynamic google::protobuf data with eCAL.
    **/

    inline CDynamicSubscriber::CDynamicSubscriber() :
      created(false),
      msg_decoder(nullptr),
      msg_ptr(nullptr)
    {
    }

    inline CDynamicSubscriber::CDynamicSubscriber(const std::string& topic_name_) :
      created(false),
      msg_decoder(nullptr),
      msg_ptr(nullptr)
    {
      Create(topic_name_);
    }

    inline CDynamicSubscriber::~CDynamicSubscriber()
    {
      Destroy();
    };

    inline void CDynamicSubscriber::Create(const std::string& topic_name_)
    {
      if (created) return;

      // save the topic name (required for receive polling)
      topic_name = topic_name_;

      // create message decoder
      msg_decoder = new eCAL::protobuf::CProtoDynDecoder();

      // create subscriber
      msg_sub.Create(topic_name_);

      created = true;
    }

    inline void CDynamicSubscriber::Destroy()
    {
      if (!created) return;

      // destroy subscriber
      msg_sub.Destroy();

      // delete message pointer
      delete msg_ptr;

      // delete message decoder
      delete msg_decoder;

      created = false;
    }

    inline google::protobuf::Message* CDynamicSubscriber::getMessagePointer()
    {
      try
      {
        // Create Message Pointer for our topic name.
        if (msg_ptr == nullptr)
        {
          msg_ptr = CreateMessagePointer(topic_name);
        }
      }
      catch (DynamicReflectionException& /*e*/)
      {
        return nullptr;
      }
      return msg_ptr;
    }

    inline bool CDynamicSubscriber::Receive(google::protobuf::Message& msg_, long long* time_, int rcv_timeout_)
    {
      std::string rec_buf;
      bool success = msg_sub.ReceiveBuffer(rec_buf, time_, rcv_timeout_);
      if (!success) return(false);

      // Parse current message content
      auto parsed = msg_.ParseFromString(rec_buf);
      return parsed;
    }

    inline bool CDynamicSubscriber::AddReceiveCallback(ProtoMsgCallbackT callback_)
    {
      msg_callback = callback_;
      msg_sub.AddReceiveCallback(std::bind(&CDynamicSubscriber::OnReceive, this, std::placeholders::_1, std::placeholders::_2));
      return(true);
    }

    inline bool CDynamicSubscriber::RemReceiveCallback()
    {
      msg_sub.RemReceiveCallback();
      msg_callback = nullptr;
      return(true);
    }

    inline bool CDynamicSubscriber::AddErrorCallback(ProtoErrorCallbackT callback_)
    {
      err_callback = callback_;
      return(true);
    }

    inline bool CDynamicSubscriber::RemErrorCallback()
    {
      err_callback = nullptr;
      return(true);
    }

    inline void CDynamicSubscriber::OnReceive(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
    {
      try
      {
        // Create Message Pointer for our topic name.
        if (msg_ptr == nullptr)
        {
          msg_ptr = CreateMessagePointer(topic_name_);
        }

        if (msg_callback && msg_ptr)
        {
          // Parse current message content
          auto parsed = msg_ptr->ParseFromArray(data_->buf, data_->size);
          if (parsed)
          {
            msg_callback(topic_name_, *msg_ptr, data_->time);
          }
          else
          {
            // If deserialization failed we check for message size.
            // Empty messages will set to a minimum size of 1 byte
            // by the protobuf::CPublisher::GetSize function to force
            // the transport through all layers.
            // In this case we clear the message object and
            // return success.
            if (data_->size == 1)
            {
              msg_ptr->Clear();
              msg_callback(topic_name_, *msg_ptr, data_->time);
            }
            else
            {
              throw DynamicReflectionException("CDynamicSubscriber: DataContent could not be parsed");
            }

          }
        }
      }
      catch (DynamicReflectionException& e)
      {
        if (err_callback)
        {
          err_callback(e.what());
        }
      }
    }

    /*
    * Might throw DynamicReflectionException!
    */
    inline google::protobuf::Message* CDynamicSubscriber::CreateMessagePointer(const std::string& topic_name_)
    {
      // get topic type
      std::string topic_type = eCAL::Util::GetTypeName(topic_name_);
      topic_type = topic_type.substr(topic_type.find_first_of(':') + 1, topic_type.size());
      topic_type = topic_type.substr(topic_type.find_last_of('.') + 1, topic_type.size());
      if (StrEmptyOrNull(topic_type))
      {
        throw DynamicReflectionException("CDynamicSubscriber: Could not get type for topic " + std::string(topic_name_));
      }

      std::string topic_desc = eCAL::Util::GetDescription(topic_name_);
      if (StrEmptyOrNull(topic_desc))
      {
        throw DynamicReflectionException("CDynamicSubscriber: Could not get description for topic " + std::string(topic_name_));
      }

      google::protobuf::FileDescriptorSet proto_desc;
      proto_desc.ParseFromString(topic_desc);
      std::string error_s;
      msg_ptr = msg_decoder->GetProtoMessageFromDescriptorSet(proto_desc, topic_type, error_s);
      if (msg_ptr == nullptr)
      {
        std::stringstream s;
        s << "CDynamicSubscriber: Message of type " + std::string(topic_name_) << " could not be decoded" << std::endl;
        s << error_s;
        throw DynamicReflectionException(s.str());
      }

      return msg_ptr;
    }
  }
}
