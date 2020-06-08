/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
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

#pragma once

#include <unordered_set>

#include "common_types.h"

namespace eCAL
{
  namespace rec
  {
    namespace addon
    {
      namespace function_descriptor
      {
        static const FunctionDescriptor info
        {
          "info",
          {},
          {
            { "name", addon::Variant::ValueType::String },
            { "id", addon::Variant::ValueType::String },
            { "description", addon::Variant::ValueType::String }
          }
        };

        static const FunctionDescriptor initialize
        {
          "initialize",
          {},
          {}
        };

        static const FunctionDescriptor deinitialize
        {
          "deinitialize",
          {},
          {}
        };

        static const FunctionDescriptor set_prebuffer_length
        {
          "set_prebuffer_length",
          {
            { "duration", addon::Variant::ValueType::Integer }
          },
          {}
        };

        static const FunctionDescriptor enable_prebuffering
        {
          "enable_prebuffering",
          {},
          {}
        };

        static const FunctionDescriptor disable_prebuffering
        {
          "disable_prebuffering",
          {},
          {}
        };

        static const FunctionDescriptor prebuffer_count
        {
          "prebuffer_count",
          {},
          {
            { "frame_count", addon::Variant::ValueType::Integer }
          }
        };

        static const FunctionDescriptor start_recording
        {
          "start_recording",
          {
            { "id", Variant::ValueType::Integer },
            { "path", Variant::ValueType::String }
          },
          {}
        };

        static const FunctionDescriptor stop_recording
        {
          "stop_recording",
          {
            { "id", Variant::ValueType::Integer }
          },
          {}
        };

        static const FunctionDescriptor job_statuses
        {
          "job_statuses",
          {},
          {
            { "id", Variant::ValueType::Integer },
            { "state", Variant::ValueType::String },
            { "healthy", Variant::ValueType::Boolean},
            { "status_description", Variant::ValueType::String },
            { "frame_count", Variant::ValueType::Integer },
            { "queue_count", Variant::ValueType::Integer }
          }
        };

        static const FunctionDescriptor save_prebuffer
        {
          "save_prebuffer",
          {
            { "id", Variant::ValueType::Integer },
            { "path", Variant::ValueType::String }
          },
          {}
        };

        static const FunctionDescriptor api_version
        {
          "api_version",
          {},
          {
            { "version", Variant::ValueType::Integer }
          }
        };
      }
    }
  }
}