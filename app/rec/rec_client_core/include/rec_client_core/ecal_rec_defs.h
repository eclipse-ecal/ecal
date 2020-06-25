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

#define ECAL_REC_VERSION_MAJOR              2
#define ECAL_REC_VERSION_MINOR              3
#define ECAL_REC_VERSION_PATCH              0

#define __ECAL_REC_FUNCTION_TO_STR(x) #x                                // Stringify any input
#define __ECAL_REC_FUNCTION_TO_STR2(x) __ECAL_REC_FUNCTION_TO_STR(x)    // Evaluate the input, then stringify it

#define ECAL_REC_VERSION_STRING             __ECAL_REC_FUNCTION_TO_STR2(ECAL_REC_VERSION_MAJOR) "." __ECAL_REC_FUNCTION_TO_STR2(ECAL_REC_VERSION_MINOR)

#define ECAL_REC_NAME                       "eCALRec"

namespace eCAL
{
  namespace rec
  {
    static constexpr int Version()
    {
      return (ECAL_REC_VERSION_PATCH
        + (ECAL_REC_VERSION_MINOR * 1000)
        + (ECAL_REC_VERSION_MAJOR * 1000 * 1000));
    }
  }
}
