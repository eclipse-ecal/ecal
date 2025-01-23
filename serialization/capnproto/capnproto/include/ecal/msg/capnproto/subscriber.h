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
 * @file   subscriber.h
 * @brief  eCAL subscriber interface for Cap'n Proto  message definitions
**/

#pragma once

#include <ecal/deprecate.h>
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
  namespace internal
  {
    template <typename T>
    class CapnprotoDeserializer
    {
    public:
      SDataTypeInformation GetDataTypeInformation()
      {
        SDataTypeInformation topic_info;
        topic_info.encoding   = eCAL::capnproto::EncodingAsString();
        topic_info.name       = eCAL::capnproto::TypeAsString<T>();
        topic_info.descriptor = eCAL::capnproto::SchemaAsString<T>();
        return topic_info;
      }

      // This function is NOT threadsafe!!!
      bool Deserialize(typename T::Reader& msg_, const void* buffer_, size_t size_)
      {     
        kj::ArrayPtr<const capnp::word> words = kj::arrayPtr(reinterpret_cast<const capnp::word*>(buffer_), size_ / sizeof(capnp::word));
        kj::ArrayPtr<const capnp::word> rest = initMessageBuilderFromFlatArrayCopy(words, m_msg_builder);

        typename T::Builder root_builder = typename T::Builder(m_msg_builder.getRoot<T>());
        msg_ = root_builder.asReader();

        return(rest.size() == 0);
      }

    private:
      capnp::MallocMessageBuilder m_msg_builder;
    };
  }

  namespace capnproto
  {
    /**
     * @brief  eCAL capnp subscriber class.
     *
     * Subscriber template  class for capnp messages. For details see documentation of CSubscriber class.
     *
    **/
    template <typename T>
    using CSubscriber = CMessageSubscriber<typename T::Reader, internal::CapnprotoDeserializer<T>>;

    /** @example addressbook_rec.cpp
    * This is an example how to use eCAL::capnproto::CSubscriber to receive capnp data with eCAL. To receive the data, see @ref addressbook_rec.cpp .
    */
  }
}
