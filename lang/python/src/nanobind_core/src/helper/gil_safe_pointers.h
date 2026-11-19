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

/**
 * @file   helper/gil_safe_pointers.h
 * @brief  Shared pointers with GIL-aware construction and deletion.
**/

#pragma once

#include <nanobind/stl/shared_ptr.h>

#include <memory>
#include <utility>


/**
 * @brief Wrap any nanobind object in a std::shared_ptr that
 *        re-acquires the Python GIL when the object is destroyed.
 *
 * @tparam T  The nanobind object type (e.g. nb::callable, nb::object, ...)
 * @param  obj The object to wrap.
 * @return A shared_ptr<T> whose custom deleter holds the GIL before deleting.
 *
 * Call with the GIL held. The last reference may be dropped on any thread
 * while the interpreter is running.
 */
template <typename T>
std::shared_ptr<T> make_shared_with_gil_acquired_on_delete(T obj) {
  // allocate on the heap so we control deletion
  return std::shared_ptr<T>(
    new T(std::move(obj)),
    [](T *p) {
      // re-acquire GIL before we destroy the Python object
      nanobind::gil_scoped_acquire acquire;
      delete p;
    }
  );
}

/**
 * @brief Construct and delete a native object with the GIL released.
 *
 * Call with the GIL held, after converting Python arguments. Python must own
 * the returned pointer and drop its last reference with the GIL held.
 * Native construction and destruction may wait for callbacks that need the GIL.
 */
template <typename T, typename... Args>
std::shared_ptr<T> make_shared_with_gil_released_on_construct_and_delete(Args&&... args)
{
  T* object;
  {
    nanobind::gil_scoped_release release;
    object = new T(std::forward<Args>(args)...);
  }
  return std::shared_ptr<T>(object, [](T* pointer) {
    nanobind::gil_scoped_release release;
    delete pointer;
  });
}