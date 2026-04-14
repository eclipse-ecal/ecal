/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2025 Continental Corporation
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

#pragma once

#include <ecal/types.h>

namespace eCAL
{
  namespace Util
  {
    /*
    * Generates a unique EntityIdT using a combination of the process ID, a random namespace, and an atomic counter.
    * This function is thread-safe.
    * For design decisions, reference ADR-0001-entity-id-generation.md in the eCAL documentation.
    */
    EntityIdT GenerateUniqueEntityId();
  } // namespace Util
} // namespace eCAL
