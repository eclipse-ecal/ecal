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

#include <ecal/msg/subscriber.h>
#include <ecal/msg/capnproto/serializer.h>

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
    using CDynamicSubscriber = CMessageSubscriber<typename capnp::DynamicStruct::Reader, internal::DynamicSerializer<SDataTypeInformation>>;

    /** @example addressbook_rec.cpp
    * This is an example how to use eCAL::capnproto::CSubscriber to receive capnp data with eCAL. To receive the data, see @ref addressbook_rec.cpp .
    */
  }
}
