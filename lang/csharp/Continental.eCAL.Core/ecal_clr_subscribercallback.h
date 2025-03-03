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
 * @file  ecal_clr_subscribercallback.h
**/

#pragma once

#include "ecal_clr_topicid.h"
#include "ecal_clr_datatype.h"
#include "ecal_clr_receivecallbackdata.h"

using namespace System;

namespace Continental
{
  namespace eCAL
  {
    namespace Core
    {
      /**
       * @brief Delegate for subscriber receive callbacks.
       *
       * @param publisherId Managed wrapper for the publisher's topic identifier.
       * @param dataTypeInfo Managed wrapper for the publisher's data type information.
       * @param data Managed wrapper for the received callback data.
       */
      public delegate void ReceiveCallbackDelegate(TopicId^ publisherId, DataTypeInformation^ dataTypeInfo, ReceiveCallbackData^ data);
    }
  }
}
