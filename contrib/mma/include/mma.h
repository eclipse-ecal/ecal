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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include "ecal/pb/mma/mma.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

class MMAImpl;

class MMA
{
 public:
  /**
  * @brief  Constructor.
  **/
  MMA();

  /**
  * @brief Destructor.
  **/
  ~MMA();

  /**
  * @brief  Get machine statistics: CPU, Memory, Disk, Network
  *
  * @param state  Machine state reference
  *
  * @return true if successful, false otherwise
  **/
  bool Get(eCAL::pb::mma::State& state);
 private:
  MMAImpl* pImpl;
};
