/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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

#include <cstddef>
#include <ecal/ecal_deprecate.h>
#include <ecal/msg/protobuf/ecal_proto_hlp.h>
#include <ecal/msg/publisher.h>

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
        explicit CPayload(const google::protobuf::Message& message_) :
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
#if GOOGLE_PROTOBUF_VERSION >= 3001000
          size_t size = static_cast<size_t>(message.ByteSizeLong());
#else
          size_t size = static_cast<size_t>(message.ByteSize());
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
       * @param config_      Optional configuration parameters.
      **/

      // call the function via its class because it's a virtual function that is called in constructor/destructor,-
      // where the vtable is not created yet, or it's destructed.
      // Probably we can handle the Message publishers differently. One message publisher class and then one class for payloads and getting type
      // descriptor information.
      explicit CPublisher(const std::string& topic_name_, const eCAL::Publisher::Configuration& config_ = {}) : eCAL::CPublisher(topic_name_, CPublisher::GetDataTypeInformation(), config_)
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
       * @param config_      Optional configuration parameters.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Create(const std::string& topic_name_, const eCAL::Publisher::Configuration& config_ = {})
      {
        return(eCAL::CPublisher::Create(topic_name_, GetDataTypeInformation(), config_));
      }

      /**
       * @brief Send a serialized message to all subscribers.
       *
       * @param msg_                     The message object.
       * @param time_                    Time stamp.
       *
       * @return  Number of bytes sent.
      **/
      size_t Send(const T& msg_, long long time_ = DEFAULT_TIME_ARGUMENT)
      {
        CPayload payload{ msg_ };
        return eCAL::CPublisher::Send(payload, time_);
      }

    private:
      /**
       * @brief   Get datatype description of the protobuf message.
       *
       * @return  Topic information.
      **/
      struct SDataTypeInformation GetDataTypeInformation() const
      {
        struct SDataTypeInformation data_type_info;
        static T msg{};
        data_type_info.encoding   = "proto";
        data_type_info.name       = msg.GetTypeName();
        data_type_info.descriptor = protobuf::GetProtoMessageDescription(msg);
        return data_type_info;
      }

    };
    /** @example person_snd.cpp
     * This is an example how to use eCAL::CPublisher to send google::protobuf data with eCAL. To receive the data, see @ref person_rec.cpp .
    */
  }
}
