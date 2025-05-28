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
 * @file   helper/make_gil_safe_shared.h
 * @brief  Create GIL safe shared pointer that will aquire GIL before deleting object.
**/

#pragma once

#include <nanobind/nanobind.h>
#include <memory>


/**
 * @brief Wrap any nanobind object in a std::shared_ptr that
 *        re-acquires the Python GIL when the object is destroyed.
 *
 * @tparam T  The nanobind object type (e.g. nb::callable, nb::object, ...)
 * @param  obj The object to wrap.
 * @return A shared_ptr<T> whose custom deleter holds the GIL before deleting.
 */
template <typename T>
std::shared_ptr<T> make_gil_safe_shared(T obj) {
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