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
 * @file   subscriber.h
 * @brief  eCAL subscriber interface for Cap'n Proto  message definitions
**/

#pragma once

#include <ecal/msg/subscriber.h>
#include <ecal/msg/capnproto/helper.h>

// capnp includes
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /*_MSC_VER*/
#include <capnp/serialize.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif /*_MSC_VER*/


namespace eCAL
{
  namespace capnproto
  {
    /**
     * @brief  eCAL capnp subscriber class.
     *
     * Subscriber template  class for capnp messages. For details see documentation of CSubscriber class.
     *
    **/
    class CBuilderSubscriber : public CMsgSubscriber<capnp::MallocMessageBuilder>
    {
    public:

      /**
      * @brief  Constructor.
      **/
      CBuilderSubscriber() : CMsgSubscriber<capnp::MallocMessageBuilder>()
      {
      }

      /**
      * @brief  Constructor.
      *
      * @param topic_name_  Unique topic name.
      **/
      CBuilderSubscriber(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_) : CMsgSubscriber<capnp::MallocMessageBuilder>(topic_name_, topic_type_, topic_desc_)
      {
      }

      /**
      * @brief  Copy Constructor is not available.
      **/
      CBuilderSubscriber(const CBuilderSubscriber&) = delete;

      /**
      * @brief  Copy Constructor is not available.
      **/
      CBuilderSubscriber& operator=(const CBuilderSubscriber&) = delete;

      /**
      * @brief  Move Constructor
      **/
      CBuilderSubscriber(CBuilderSubscriber&&) = default;

      /**
      * @brief  Move assignment
      **/
      CBuilderSubscriber& operator=(CBuilderSubscriber&&) = default;

      /**
       * @brief  Creates this object.
       *
       * @param topic_name_  Unique topic name.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_)
      {
        return(CMsgSubscriber<capnp::MallocMessageBuilder>::Create(topic_name_, topic_type_, topic_desc_));
      }

      /**
       * @brief  Get type name of the capnp message.
       *
       * @return  Type name.
      **/
      std::string GetTypeName() const
      {
        return("capnp:");
      }

    private:
      /**
       * @brief  Get file descriptor string of the capnp message.
       *
       * @return  Description string.
      **/
      std::string GetDescription() const
      {
        return("");
      }

      /**
       * @brief  Deserialize the message object from a message buffer.
       *
       * @param [out] msg_     The message object.
       * @param       buffer_  Source buffer.
       * @param       size_    Source buffer size.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Deserialize(capnp::MallocMessageBuilder& msg_, const void* buffer_, size_t size_) const
      {
        kj::ArrayPtr<const capnp::word> words = kj::arrayPtr(reinterpret_cast<const capnp::word*>(buffer_), size_ / sizeof(capnp::word));
        kj::ArrayPtr<const capnp::word> rest = initMessageBuilderFromFlatArrayCopy(words, msg_);
        return(rest.size() == 0);
      }
    };
    /** @example addressbook_rec.cpp
    * This is an example how to use eCAL::CCapnpSubscriber to receive capnp data with eCAL. To send the data, see @ref addressbook_snd.cpp .
    */

    /**
    * @brief  eCAL capnp subscriber class.
    *
    * Subscriber template  class for capnp messages. For details see documentation of CSubscriber class.
    *
    **/
    template <typename message_type>
    class CSubscriber
    {
    public:
      /**
      * @brief  Constructor.
      **/
      CSubscriber()
        : subscriber()
        , builder()
        , root_builder(builder.getRoot<message_type>())
      {
      }

      /**
      * @brief  Constructor.
      *
      * @param topic_name_  Unique topic name.
      **/
      CSubscriber(const std::string& topic_name_)
        : subscriber(topic_name_, GetTypeName(), GetDescription())
        , builder()
        , root_builder(builder.getRoot<message_type>())
      {
      }

      /**
      * @brief  Copy Constructor is not available.
      **/
      CSubscriber(const CSubscriber&) = delete;

      /**
      * @brief  Copy Constructor is not available.
      **/
      CSubscriber& operator=(const CSubscriber&) = delete;

      /**
      * @brief  Move Constructor
      **/
      CSubscriber(CSubscriber&&) = default;

      /**
      * @brief  Move assignment
      **/
      CSubscriber& operator=(CSubscriber&&) = default;

      /**
      * @brief eCAL protobuf message receive callback function
      *
      * @param topic_name_  Topic name of the data source (publisher).
      * @param msg_         Protobuf message content.
      * @param time_        Message time stamp.
      **/
      typedef std::function<void(const char* topic_name_, typename message_type::Reader msg_, long long time_, long long clock_, long long id_)> MsgCallbackT;

      /**
      * @brief Add callback function for incoming receives.
      *
      * @param callback_  The callback function to add.
      *
      * @return  True if succeeded, false if not.
      **/
      bool AddReceiveCallback(MsgCallbackT callback_)
      {
        msg_callback = callback_;
        return subscriber.AddReceiveCallback(std::bind(&CSubscriber::OnReceive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
      }

      bool RemReceiveCallback()
      {
        auto ret{subscriber.RemReceiveCallback()};
        msg_callback = nullptr;
        return ret;
      }

      void OnReceive(const char* topic_name_, const capnp::MallocMessageBuilder& msg_, long long time_, long long clock_, long long id_)
      {
        auto root = const_cast<capnp::MallocMessageBuilder&>(msg_).getRoot<message_type>();
        msg_callback(topic_name_, root, time_, clock_, id_);
      }

      /**
      * @brief get a Pointer to a temporary message that can be passed to receive
      **/
      typename message_type::Reader getReader()
      {
        return root_builder.asReader();
      }

      /**
      * @brief Manually receive the next sample
      **/
      bool Receive(long long* time_ = nullptr, int rcv_timeout_ = 0)
      {
        bool success = subscriber.Receive(builder, time_, rcv_timeout_);
        // Update the Reader
        root_builder = typename message_type::Builder(builder.getRoot<message_type>());
        return success;
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
        return(subscriber.Create(topic_name_, GetTypeName(), GetDescription()));
      }

      /**
      * @brief  Get type name of the capnp message.
      *
      * @return  Type name.
      **/
      std::string GetTypeName() const
      {
        return eCAL::capnproto::TypeAsString<message_type>();
      }

    private:

      CBuilderSubscriber subscriber;
      capnp::MallocMessageBuilder builder;
      typename message_type::Builder root_builder;
      MsgCallbackT msg_callback;

      /**
      * @brief  Get file descriptor string of the capnp message.
      *
      * @return  Description string.
      **/
      std::string GetDescription() const
      {
        return eCAL::capnproto::SchemaAsString<message_type>();
      }
    };

  }
}
