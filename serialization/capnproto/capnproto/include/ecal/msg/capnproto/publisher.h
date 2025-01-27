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
 * @file   publisher.h
 * @brief  eCAL publisher interface for Cap'n Proto message definitions
**/

#pragma once

#include <ecal/pubsub/publisher.h>
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

#include <memory>

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
    template <typename message_type>
    class CPublisher : public eCAL::CPublisher
    {
      class CPayload : public eCAL::CPayloadWriter
      {
      public:
        CPayload(const capnp::MallocMessageBuilder& message_builder_) :
          message_builder(message_builder_) {};

        ~CPayload() override = default;

        CPayload(const CPayload&) = default;
        CPayload(CPayload&&) noexcept = default;

        CPayload& operator=(const CPayload&) = delete;
        CPayload& operator=(CPayload&&) noexcept = delete;

        bool WriteFull(void* buf_, size_t len_) override
        {
          kj::Array<capnp::word> words = capnp::messageToFlatArray(const_cast<capnp::MallocMessageBuilder&>(message_builder));
          kj::ArrayPtr<kj::byte> bytes = words.asBytes();
          if (len_ < bytes.size()) return(false);
          memcpy(buf_, bytes.begin(), bytes.size());
          return(true);
        }

        size_t GetSize() override
        {
          return(capnp::computeSerializedSizeInWords(const_cast<capnp::MallocMessageBuilder&>(message_builder)) * sizeof(capnp::word));
        };

      private:
        const capnp::MallocMessageBuilder& message_builder;
      };

    public:
      /**
       * @brief  Constructor.
       *
       * @param topic_name_  Unique topic name.
       * @param config_      Optional configuration parameters.
      **/
      explicit CPublisher(const std::string& topic_name_, const eCAL::Publisher::Configuration& config_ = GetPublisherConfiguration()) : eCAL::CPublisher(topic_name_, GetDataTypeInformation(), config_)
        , builder(std::make_unique<capnp::MallocMessageBuilder>())
        , root_builder(builder->initRoot<message_type>())
      {
      }

      /**
       * @brief  Constructor.
       *
       * @param topic_name_      Unique topic name.
       * @param event_callback_  The publisher event callback funtion.
       * @param config_          Optional configuration parameters.
      **/
      explicit CPublisher(const std::string& topic_name_, const eCAL::PubEventCallbackT& event_callback_, const eCAL::Publisher::Configuration& config_ = GetPublisherConfiguration()) : eCAL::CPublisher(topic_name_, GetDataTypeInformation(), event_callback_, config_)
        , builder(std::make_unique<capnp::MallocMessageBuilder>())
        , root_builder(builder->initRoot<message_type>())
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
      * @brief  Destructor -> cannot throw exceptions!
      **/
      ~CPublisher()
      {
        try {
          builder.reset();
        }
        catch (...)
        {
        }

      }

      typename message_type::Builder GetBuilder()
      {
        return root_builder;
      }

      bool Send()
      {
        CPayload payload{ *builder };
        return eCAL::CPublisher::Send(payload);
      }

    private:
      /**
       * @brief   Get topic information of the message.
       *
       * @return  Topic information.
      **/
      SDataTypeInformation GetDataTypeInformation() const
      {
        SDataTypeInformation data_type_info;
        data_type_info.encoding   = eCAL::capnproto::EncodingAsString();
        data_type_info.name       = eCAL::capnproto::TypeAsString<message_type>();
        data_type_info.descriptor = eCAL::capnproto::SchemaAsString<message_type>();
        return data_type_info;
      }

      std::unique_ptr<capnp::MallocMessageBuilder>    builder;
      typename message_type::Builder                  root_builder;
    };
    /** @example addressbook_snd.cpp
    * This is an example how to use eCAL::capnproto::CPublisher to send capnp data with eCAL. To receive the data, see @ref addressbook_rec.cpp .
    */
  }
}
