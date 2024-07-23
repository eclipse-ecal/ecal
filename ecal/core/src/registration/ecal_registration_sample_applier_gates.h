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
 * @brief  eCAL Sample Applier Gates
 *
 * This class applies incoming samples to the registration gates
**/

#pragma once

#include "serialization/ecal_struct_sample_registration.h"

namespace eCAL
{
  namespace Registration
  {
    class CSampleApplierGates
    {
    public:
      static void ApplySample(const eCAL::Registration::Sample& sample_);
    };
  }
}
