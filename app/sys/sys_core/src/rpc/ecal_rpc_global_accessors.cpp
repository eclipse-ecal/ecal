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

/**
 * @brief  eCAL rpc global accessors
**/

#include <atomic>
#include <mutex>

#include "ecal_rpc_global_accessors.h"
#include "ecal_rpc_globals.h"

namespace eCAL
{
  namespace RPC
  {
    CGlobals*         g_globals_ctx(nullptr);
    std::atomic<int>  g_globals_ctx_ref_cnt;
    std::mutex        g_rpc_mutex;


    CGlobals* g_globals()
    {
      return g_globals_ctx;
    }

    CServiceClient* g_rpc()
    {
      if (!g_globals()) return(nullptr);
      return(g_globals()->rpc().get());
    }
  }
}
