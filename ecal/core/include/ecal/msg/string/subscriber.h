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
 * @brief  eCAL subscriber interface for std::string message definitions
**/

#pragma once

#include <ecal/msg/subscriber.h>
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
    template <typename T>
    using CSubscriber = CMessageSubscriber<T, internal::Serializer<T>>;

    /** @example minimal_rec.cpp
    * This is an example how to use eCAL::CSubscriber to receive a std::string with eCAL. To send the strings, see @ref minimal_snd.cpp .
    */
  }
}
