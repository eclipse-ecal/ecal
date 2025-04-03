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

#pragma once

namespace ecal_service
{
  enum class State
  {
    NOT_CONNECTED,  //!< Initial state
    HANDSHAKE,      //!< The connection is currently in handshake state.
    CONNECTED,      //!< The connection is established and ready to exchange data.
    FAILED,         //!< The connection has been closed due to an error or by the user
  };
}
