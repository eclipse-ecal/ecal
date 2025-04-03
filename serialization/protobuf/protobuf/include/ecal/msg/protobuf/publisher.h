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
 * @brief  eCAL publisher interface for google::protobuf message definitions
**/

#pragma once

#include <ecal/msg/publisher.h>
#include <ecal/msg/protobuf/serializer.h>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief eCAL google::protobuf publisher class.
     *
     * Publisher template class for google::protobuf messages. For details see documentation of CPublisher class.
     *
    **/
    template <typename T>
    using CPublisher = CMessagePublisher<T, internal::Serializer<T, ::eCAL::SDataTypeInformation>>;
  }
  /** @example person_snd.cpp
   * This is an example how to use eCAL::CPublisher to send google::protobuf data with eCAL. To receive the data, see @ref person_rec.cpp .
  */
}
