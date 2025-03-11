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
 * @brief  eCAL publisher interface for std::string message definitions
**/

#pragma once

#include <ecal/msg/publisher.h>
#include <ecal/msg/string/serializer.h>

#include <string>

namespace eCAL
{
  namespace string
  {

    /**
     * @brief  eCAL subscriber class for standard strings.
     *
     * Subscriber class for STL standard strings. For details see documentation of CSubscriber class.
     *
    **/
    using CPublisher = CMessagePublisher<std::string, internal::Serializer<std::string>>;

    /** @example minimal_snd.cpp
    * This is an example how to use eCAL::string::CPublisher to receive a std::string with eCAL.
    */
  }
}
