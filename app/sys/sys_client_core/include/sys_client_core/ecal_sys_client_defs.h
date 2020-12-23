/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
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

#define ECAL_SYS_CLIENT_VERSION_MAJOR              2
#define ECAL_SYS_CLIENT_VERSION_MINOR              0
#define ECAL_SYS_CLIENT_VERSION_PATCH              0

#define __ECAL_SYS_CLIENT_FUNCTION_TO_STR(x) #x                                // Stringify any input
#define __ECAL_SYS_CLIENT_FUNCTION_TO_STR2(x) __ECAL_SYS_CLIENT_FUNCTION_TO_STR(x)    // Evaluate the input, then stringify it

#define ECAL_SYS_CLIENT_VERSION_STRING             __ECAL_SYS_CLIENT_FUNCTION_TO_STR2(ECAL_SYS_CLIENT_VERSION_MAJOR) "." __ECAL_SYS_CLIENT_FUNCTION_TO_STR2(ECAL_SYS_CLIENT_VERSION_MINOR) "." __ECAL_SYS_CLIENT_FUNCTION_TO_STR2(ECAL_SYS_CLIENT_VERSION_PATCH)

#define ECAL_SYS_CLIENT_NAME                       "eCALSys"

namespace eCAL
{
  namespace sys_client
  {
    static constexpr int Version()
    {
      return (ECAL_SYS_CLIENT_VERSION_PATCH
        + (ECAL_SYS_CLIENT_VERSION_MINOR * 1000)
        + (ECAL_SYS_CLIENT_VERSION_MAJOR * 1000 * 1000));
    }
  }
}
