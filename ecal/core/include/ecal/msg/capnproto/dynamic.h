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
 * @file   dynamic.h
 * @brief  eCAL dynamic subscriber interface for Cap'n Proto  message definitions
**/

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /*_MSC_VER*/
#include <capnp/schema.h>
#include <capnp/schema-loader.h>
#include <capnp/dynamic.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif /*_MSC_VER*/

#include <ecal/msg/capnproto/subscriber.h>
#include <ecal/msg/capnproto/helper.h>

namespace eCAL
{

  namespace capnproto
  {
    class CDynamicSubscriber
    {
    public:
      /**
      * @brief  Constructor.
      **/
      CDynamicSubscriber()
        : subscriber()
        , builder()
        , initialized(false)
      {
      }

      /**
      * @brief  Constructor.
      *
      * @param topic_name_  Unique topic name.
      **/
      CDynamicSubscriber(const std::string& topic_name_)
        : subscriber(topic_name_, GetTypeName(), GetDescription())
        , builder()
        , initialized(false)
      {
      }

      /**
      * @brief  Copy Constructor is not available.
      **/
      CDynamicSubscriber(const CDynamicSubscriber&) = delete;

      /**
      * @brief  Copy Constructor is not available.
      **/
      CDynamicSubscriber& operator=(const CDynamicSubscriber&) = delete;

      /**
      * @brief  Move Constructor
      **/
      CDynamicSubscriber(CDynamicSubscriber&&) = default;

      /**
      * @brief  Move assignment
      **/
      CDynamicSubscriber& operator=(CDynamicSubscriber&&) = default;

      /**
      * @brief eCAL protobuf message receive callback function
      *
      * @param topic_name_  Topic name of the data source (publisher).
      * @param msg_         Protobuf message content.
      * @param time_        Message time stamp.
      **/
      typedef std::function<void(const char* topic_name_, typename capnp::DynamicStruct::Reader msg_, long long time_, long long clock_, long long id_)> CapnpDynamicMsgCallbackT;

      /**
      * @brief Add callback function for incoming receives.
      *
      * @param callback_  The callback function to add.
      *
      * @return  True if succeeded, false if not.
      **/
      bool AddReceiveCallback(CapnpDynamicMsgCallbackT callback_)
      {
        msg_callback = callback_;
        return subscriber.AddReceiveCallback(std::bind(&CDynamicSubscriber::OnReceive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
      }

      bool RemReceiveCallback()
      {
        auto ret = subscriber.RemReceiveCallback();
        msg_callback = nullptr;
        return ret;
      }

      void OnReceive(const char* topic_name_, const capnp::MallocMessageBuilder& msg_, long long time_, long long clock_, long long id_)
      {
        if (!initialized)
        {
          std::string topic_desc = eCAL::Util::GetDescription(topic_name_);
          if (!topic_desc.empty())
          {
            // We initialize the builder from the string
            schema = eCAL::capnproto::SchemaFromDescriptor(topic_desc, loader);
            initialized = true;
          }
          else
          {
            return;
          }
        }

        auto root = const_cast<capnp::MallocMessageBuilder&>(msg_).getRoot<capnp::DynamicStruct>(schema.asStruct());
        msg_callback(topic_name_, root.asReader(), time_, clock_, id_);
      }

      /**
      * @brief get a Pointer to a temporary message that can be passed to receive
      **/
      typename capnp::DynamicStruct::Reader getReader()
      {
        return root_builder.asReader();
      }

      /**
      * @brief Manually receive the next sample
      **/
      bool Receive(long long* time_ = nullptr, int rcv_timeout_ = 0)
      {
        return  subscriber.Receive(builder, time_, rcv_timeout_);
      }

      /**
      * @brief  Creates this object.
      *
      * @param topic_name_  Unique topic name.
      *
      * @return  True if it succeeds, false if it fails.
      **/
      bool Create(const std::string& topic_name_)
      {
        return(subscriber.Create(topic_name_, "", ""));
      }

      /**
      * @brief  Get type name of the capnp message.
      *
      * @return  Type name.
      **/
      std::string GetTypeName() const
      {
        return ("");
      }

    private:

      CBuilderSubscriber subscriber;
      capnp::MallocMessageBuilder builder;
      capnp::DynamicStruct::Builder root_builder;
      CapnpDynamicMsgCallbackT msg_callback;

      capnp::schema::Node::Reader reader;
      capnp::SchemaLoader loader;
      capnp::Schema schema;

      bool initialized;

      /**
      * @brief  Get file descriptor string of the capnp message.
      *
      * @return  Description string.
      **/
      std::string GetDescription() const
      {
        return("");
      }
    };

  }
  
}