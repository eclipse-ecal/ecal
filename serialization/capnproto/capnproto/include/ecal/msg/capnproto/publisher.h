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

#include <ecal/msg/publisher.h>
#include <ecal/msg/capnproto/serializer.h>

    // capnp includes
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif /*_MSC_VER*/
#include <capnp/serialize.h>
#include <capnp/message.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif /*_MSC_VER*/

namespace eCAL
{
  namespace capnproto
  {
    /**
     * @brief  eCAL capnp publisher class.
     *
     * Publisher template class for capnp messages. For details see documentation of CSubscriber class.
     *
    **/
    template <typename T>
    using CPublisher = CMessagePublisher<typename capnp::MallocMessageBuilder, internal::Serializer<T, SDataTypeInformation>>;

    /** @example addressbook_snd.cpp
    * This is an example how to use eCAL::capnproto::CPublisher to send capnp data with eCAL. To receive the data, see @ref addressbook_rec.cpp .
    */
  }
}
