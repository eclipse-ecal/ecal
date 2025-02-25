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
 * @brief  eCAL publisher interface for google::flatbuffers message definitions
**/

#pragma once

#include <ecal/msg/publisher.h>
#include <ecal/msg/flatbuffers/serializer.h>

namespace eCAL
{
  namespace flatbuffers
  {
    /**
     * @brief eCAL google::flatbuffers publisher class.
     *
     * Publisher template  class for goggle::flatbuffers messages. For details see documentation of CPublisher class.
     *
    **/
    template <typename T>
    using CObjectPublisher = CMessagePublisher<T, internal::ObjectSerializer<T>>;

    /** @example monster_snd.cpp
    * This is an example how to use eCAL::CPublisher to send goggle::flatbuffers data with eCAL. To receive the data, see @ref monster_rec.cpp .
    */
  }
}
