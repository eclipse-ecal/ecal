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

#include <nanobind/stl/shared_ptr.h>

#include <memory>
#include <utility>

// Native construction and destruction may wait for callbacks that need the GIL.
// Call this factory with the GIL held, after converting Python arguments.
template <typename T, typename... Args>
std::shared_ptr<T> make_gil_released_shared(Args&&... args)
{
  T* object;
  {
    nanobind::gil_scoped_release release;
    object = new T(std::forward<Args>(args)...);
  }
  // Python owns the returned pointer and drops its last reference with the GIL held.
  return std::shared_ptr<T>(object, [](T* pointer) {
    nanobind::gil_scoped_release release;
    delete pointer;
  });
}
