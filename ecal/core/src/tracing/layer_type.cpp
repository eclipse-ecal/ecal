/* ========================= eCAL LICENSE =================================
 *
 * Copyright 2026 AUMOVIO and subsidiaries. All rights reserved.
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

#include "layer_type.h"

namespace eCAL
{
  namespace tracing
  {
    eTracingLayerType ToTracingLayerType(eTLayerType layer)
    {
      switch (layer)
      {
      case tl_ecal_shm:
        return tl_trace_shm;
      case tl_ecal_udp:
        return tl_trace_udp;
      case tl_ecal_tcp:
        return tl_trace_tcp;
      case tl_all:
        return tl_trace_all;
      default:
        return tl_trace_none;
      }
    }
  }
}