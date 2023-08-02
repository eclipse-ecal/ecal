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
 * @brief  eCAL publisher interface for google::protobuf message definitions
**/

#pragma once

#include <ecal/ecal_deprecate.h>
#include <ecal/msg/publisher.h>
#include <ecal/protobuf/ecal_proto_hlp.h>

// protobuf includes
#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#include <google/protobuf/descriptor.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

// stl includes
#include <map>
#include <memory>
#include <string>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief eCAL google::protobuf publisher class.
     *
     * Publisher template  class for google::protobuf messages. For details see documentation of CPublisher class.
     *
    **/
    template <typename T>
    class CPublisher : public eCAL::CPublisher
    {
      class CPayload : public eCAL::CPayloadWriter
      {
      public:
        CPayload(const google::protobuf::Message& message_) :
          message(message_) {};

        ~CPayload() override = default;

        CPayload(const CPayload&) = default;
        CPayload(CPayload&&) noexcept = default;

        CPayload& operator=(const CPayload&) = delete;
        CPayload& operator=(CPayload&&) noexcept = delete;

        bool WriteFull(void* buf_, size_t len_) override
        {
          return message.SerializeToArray(buf_, static_cast<int>(len_));
        }

        size_t GetSize() override {
          size_t size(0);
#if GOOGLE_PROTOBUF_VERSION >= 3001000
          size = static_cast<size_t>(message.ByteSizeLong());
#else
          size = static_cast<size_t>(message.ByteSize());
#endif
          return(size);
          };

      private:
        const google::protobuf::Message& message;
      };

    public:
      /**
       * @brief  Constructor.
      **/
      CPublisher() : eCAL::CPublisher()
      {
      }

      /**
       * @brief  Constructor.
       *
       * @param topic_name_  Unique topic name.
      **/

      // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
      // where the vtable is not created yet or it's destructed.
      // Probably we can handle the Message publishers differently. One message publisher class and then one class for payloads and getting type
      // descriptor information.
      CPublisher(const std::string& topic_name_) : eCAL::CPublisher(topic_name_, CPublisher::GetDataTypeInformation())
      {
      }

      /**
      * @brief  Copy Constructor is not available.
      **/
      CPublisher(const CPublisher&) = delete;

      /**
      * @brief  Move Constructor
      **/
      CPublisher(CPublisher&&) = default;

      /**
      * @brief  Destructor.
      **/
      ~CPublisher() override = default;

      /**
      * @brief  Copy assignment is not available.
      **/
      CPublisher& operator=(const CPublisher&) = delete;

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
        return(eCAL::CPublisher::Create(topic_name_, GetDataTypeInformation()));
      }

      size_t Send(const T& msg_, long long time_ = -1)
      {
        return Send(msg_, time_, -1);
      }

      /**
       * @brief Send a serialized message to all subscribers synchronized with acknowledge timeout (see also ShmSetAcknowledgeTimeout).
       *
       * This synchronized mode is currently implemented for local interprocess communication (shm-ecal layer) only.
       *
       * @param msg_                     The message object.
       * @param time_                    Time stamp.
       * @param acknowledge_timeout_ms_  Maximum time to wait for all subscribers acknowledge feedback in ms (buffer received and processed).
       *
       * @return  Number of bytes sent.
      **/
      size_t Send(const T& msg_, long long time_, long long acknowledge_timeout_ms_)
      {
        CPayload payload{ msg_ };
        eCAL::CPublisher::Send(payload, time_, acknowledge_timeout_ms_);
        return(0);
      }


      /**
       * @brief  Get type name of the protobuf message.
       *
       * @return  Type name.
      **/
      ECAL_DEPRECATE_SINCE_5_13("Please use SDataTypeInformation GetDataTypeInformation() instead. This function will be removed in eCAL6.")
      std::string GetTypeName() const
      {
        SDataTypeInformation topic_info{ GetDataTypeInformation() };
        return Util::CombinedTopicEncodingAndType(topic_info.encoding, topic_info.name);
      }

    private:
      /**
       * @brief  Get file descriptor string of the protobuf message.
       *
       * @return  Description string.
      **/
      ECAL_DEPRECATE_SINCE_5_13("Please use SDataTypeInformation GetDataTypeInformation() instead. This function will be removed in eCAL6.")
      std::string GetDescription() const
      {
        return GetDataTypeInformation().descriptor;
      }

      /**
      * @brief   Get datatype description of the protobuf message.
      *
      * @return  Topic information.
      **/
      SDataTypeInformation GetDataTypeInformation() const
      {
        SDataTypeInformation topic_info;
        static T msg{};
        topic_info.encoding = "proto";
        topic_info.name = msg.GetTypeName();
        topic_info.descriptor = protobuf::GetProtoMessageDescription(msg);
        return topic_info;
      }

    };
    /** @example person_snd.cpp
    * This is an example how to use eCAL::CPublisher to send google::protobuf data with eCAL. To receive the data, see @ref person_rec.cpp .
    */
  }
}
