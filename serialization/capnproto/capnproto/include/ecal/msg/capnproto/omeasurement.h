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
 * @file   subscriber.h
 * @brief  eCAL subscriber interface for google::protobuf message definitions
**/

#pragma once

#include <ecal/msg/omeasurement.h>
#include <ecal/msg/capnproto/serializer.h>

namespace eCAL
{
  namespace capnproto
  {

    /**
     * @brief  eCAL capnproto channel class.
     *
     * Publisher template class for capnp messages. For details see documentation of CSubscriber class.
     *
    **/
    template <typename T>
    using OChannel = ::eCAL::measurement::OMessageChannel<typename capnp::MallocMessageBuilder, internal::Serializer<T, eCAL::experimental::measurement::base::DataTypeInformation>>;

    /** @example addressbook_write.cpp
    * This is an example how to use eCAL::capnproto::OChannel to write capnproto data from a measurement.
    */
  }
}

