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
 * @brief  eCAL subscriber interface for google::protobuf message definitions
**/

#pragma once

#include <ecal/msg/subscriber.h>
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
     * @brief  eCAL google::protobuf subscriber class.
     *
     * Subscriber template  class for google::protobuf messages. For details see documentation of CSubscriber class.
     *
    **/
    template <typename T>
    class CSubscriber : public CMsgSubscriber<T>
    {
    public:
      /**
       * @brief  Constructor.
      **/
      CSubscriber() : CMsgSubscriber<T>()
      {
      }

      /**
       * @brief  Constructor.
       *
       * @param topic_name_  Unique topic name.
      **/

      // call the function via its class becase it's a virtual function that is called in constructor/destructor,-
      // where the vtable is not created yet or it's destructed.
      CSubscriber(const std::string& topic_name_) : CMsgSubscriber<T>(topic_name_, CSubscriber::GetDataTypeInformation())
      {
      }

      /**
      * @brief  Copy Constructor is not available.
      **/
      CSubscriber(const CSubscriber&) = delete;

      /**
      * @brief Copy Assignment is not available.
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
       * @brief  Creates this object.
       *
       * @param topic_name_  Unique topic name.
       *
       * @return  True if it succeeds, false if it fails.
      **/
      bool Create(const std::string& topic_name_)
      {
        return(CMsgSubscriber<T>::Create(topic_name_, GetDataTypeInformation()));
      }

    private:
      /**
      * @brief  Get topic information of the protobuf message.
      *
      * @return  Topic information.
      **/
      SDataTypeInformation GetDataTypeInformation() const override
      {
        SDataTypeInformation topic_info;
        static T msg{};
        topic_info.encoding = "proto";
        topic_info.name = msg.GetTypeName();
        topic_info.descriptor = protobuf::GetProtoMessageDescription(msg);
        return topic_info;
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
      bool Deserialize(T& msg_, const void* buffer_, size_t size_) const override
      {
        // we try to parse the message from the received buffer
        if (msg_.ParseFromArray(buffer_, static_cast<int>(size_)))
        {
          return(true);
        }
        return(false);
      }

    };
    /** @example person_rec.cpp
    * This is an example how to use eCAL::CSubscriber to receive google::protobuf data with eCAL. To send the data, see @ref person_snd.cpp .
    */
  }
}
