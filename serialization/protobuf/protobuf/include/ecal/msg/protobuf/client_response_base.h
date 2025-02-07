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
 * @file   client_response_base.h
**/

#pragma once

#include <ecal/msg/protobuf/client_base.h>

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief Base class for synchronous (blocking) response calls.
     *
     * This class will serve as the parent for the typed and untyped variants.
     */
    template <typename T>
    class CServiceClientResponseBase : public CServiceClientBase<T>
    {
    public:
      using CServiceClientBase<T>::CServiceClientBase; // Inherit constructors
      virtual ~CServiceClientResponseBase() override = default;
    };
  }
}
