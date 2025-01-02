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

#include <cstddef>
#include <ecal/msg/protobuf/ecal_proto_hlp.h>
#include <ecal/msg/subscriber.h>
#include <ecal/msg/protobuf/serializer.h>

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
    using CSubscriber = CMessageSubscriber<T, internal::Serializer<T>>;

    /** @example person_rec.cpp
    * This is an example how to use eCAL::CSubscriber to receive google::protobuf data with eCAL. To send the data, see @ref person_snd.cpp .
    */
  }
}

