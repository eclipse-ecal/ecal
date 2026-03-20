/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

/**
 * @file   config/service.h
 * @brief  eCAL configuration for the service layer
**/

#pragma once

namespace eCAL
{
  namespace Service
  {
    struct Configuration
    {
      /**
       * @brief Timeout (ms) the server waits for a newly connected client to
       *        send its entity identity via the internal __ecal_client_id__ call.
       *
       * New eCAL clients send this call immediately after the TCP handshake.
       * Old (pre-change) clients never send it.  The server fires the
       * Connected event with the real client SServiceId if the identity call
       * arrives within this window, or with an empty placeholder once the
       * timeout expires (backward-compatible behaviour).
       *
       * Default: 1000 ms
       * Set to 0 to disable waiting (fire Connected immediately with an empty
       * placeholder, equivalent to the behaviour before this feature was added).
       */
      unsigned int server_client_id_timeout_ms { 1000 };
    };
  }
}
