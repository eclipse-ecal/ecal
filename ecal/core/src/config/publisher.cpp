/* =========================== LICENSE =================================
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
 * =========================== LICENSE =================================
 */

#include "ecal/config/publisher.h"

namespace eCAL
{
  namespace Publisher
  {
    bool Validate(Configuration& config_)
    {
      bool isValid = true;
      isValid &= config_.layer.shm.memfile_min_size_bytes >= 4096 && config_.layer.shm.memfile_min_size_bytes % 4096 == 0;
      isValid &= config_.layer.shm.memfile_reserve_percent >= 50 && config_.layer.shm.memfile_reserve_percent <= 100;      
      return isValid;
    }
  }
}