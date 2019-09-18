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
 * @file   publisher.h
 * @brief  eCAL publisher interface for Cap'n Proto message definitions
**/

#pragma once

#include <ecal/msg/publisher.h>
#include <ecal/msg/capnproto/helper.h>

// capnp includes
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /*_MSC_VER*/
#include <capnp/serialize.h>
#include <capnp/schema.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif /*_MSC_VER*/


namespace eCAL
{
  namespace capnproto
  {
    /**
     * @brief eCAL capnp publisher class.
     *
     * Publisher template  class for capnp messages. For details see documentation of CPublisher class.
     *
    **/
    class CBuilderPublisher : public CMsgPublisher<capnp::MallocMessageBuilder>
    {
    public:
      /**
       * @brief  Constructor.
      **/
      CBuilderPublisher() : CMsgPublisher<capnp::MallocMessageBuilder>()
      {
      }

      /**
       * @brief  Constructor.
       *
       * @param topic_name_  Unique topic name.
      **/
      CBuilderPublisher(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_) : CMsgPublisher<capnp::MallocMessageBuilder>(topic_name_, topic_type_, topic_desc_)
      {
      }

      /**
      * @brief  Copy Constructor is not available.
      **/
      CBuilderPublisher(const CBuilderPublisher&) = delete;

      /**
      * @brief  Copy Constructor is not available.
      **/
      CBuilderPublisher& operator=(const CBuilderPublisher&) = delete;

      /**
      * @brief  Move Constructor
      **/
      CBuilderPublisher(CBuilderPublisher&&) = default;

      /**
      * @brief  Move assignment
      **/
      CBuilderPublisher& operator=(CBuilderPublisher&&) = default;

      /**
       * @brief  Creates this object.
       *
       * @param topic_name_  Unique topic name.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Create(const std::string& topic_name_, const std::string& topic_type_, const std::string& topic_desc_)
      {
        return(CMsgPublisher<capnp::MallocMessageBuilder>::Create(topic_name_, topic_type_, topic_desc_));
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
        //auto schema = capnp::Schema::from<T>();
        return("");
      }

      /**
       * @brief  Get size for serialized message object.
       *
       * @param msg_  The message object.
       *
       * @return  Message size.
      **/
      size_t GetSize(const capnp::MallocMessageBuilder& msg_) const
      {
        return(capnp::computeSerializedSizeInWords(const_cast<capnp::MallocMessageBuilder&>(msg_)) * sizeof(capnp::word));
      }

      /**
       * @brief  Serialize the message object into a preallocated char buffer.
       *
       * @param       msg_     The message object.
       * @param [out] buffer_  Target buffer.
       * @param       size_    Target buffer size.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Serialize(const capnp::MallocMessageBuilder& msg_, char* buffer_, size_t size_) const
      {
        kj::Array<capnp::word> words = capnp::messageToFlatArray(const_cast<capnp::MallocMessageBuilder&>(msg_));
        kj::ArrayPtr<kj::byte> bytes = words.asBytes();
        if (size_ < bytes.size()) return(false);
        memcpy(buffer_, bytes.begin(), bytes.size());
        return(true);
      }
    };
    /** @example addressbook_snd.cpp
    * This is an example how to use eCAL::CPublisher to send capnp data with eCAL. To receive the data, see @ref addressbook_rec.cpp .
    */

    /**
    * @brief eCAL capnp publisher class.
    *
    * Publisher template  class for capnp messages. For details see documentation of CPublisher class.
    *
    **/
    template <typename message_type>
    class CPublisher
    {
    public:
      /**
      * @brief  Constructor.
      **/
      CPublisher()
        : publisher()
        , builder()
        , root_builder(builder.initRoot<message_type>())
      {
      }

      /**
      * @brief  Constructor.
      *
      * @param topic_name_  Unique topic name.
      **/
      CPublisher(const std::string& topic_name_)
        : publisher(topic_name_, GetTypeName(), GetDescription())
        , builder()
        , root_builder(builder.initRoot<message_type>())
      {
      }

      /**
      * @brief  Copy Constructor is not available.
      **/
      CPublisher(const CPublisher&) = delete;

      /**
      * @brief  Copy Constructor is not available.
      **/
      CPublisher& operator=(const CPublisher&) = delete;

      /**
      * @brief  Move Constructor
      **/
      CPublisher(CPublisher&&) = default;

      /**
      * @brief  Move assignment
      **/
      CPublisher& operator=(CPublisher&&) = default;

      /**
      * @brief  Creates this object.
      *
      * @param topic_name_  Unique topic name.
      *
      * @return  True if it succeeds, false if it fails.
      **/
      bool Create(const std::string& topic_name_)
      {
        return(publisher.Create(topic_name_, GetTypeName(), GetDescription()));
      }

      typename message_type::Builder GetBuilder()
      {
        return root_builder;
      }

      void Send()
      {
        publisher.Send(builder);
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

      CBuilderPublisher publisher;
      capnp::MallocMessageBuilder builder;
      typename message_type::Builder root_builder;

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
    /** @example addressbook_snd.cpp
    * This is an example how to use eCAL::CPublisher to send capnp data with eCAL. To receive the data, see @ref addressbook_rec.cpp .
    */
  }
}
