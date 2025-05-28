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
 * @file   helper/bytestring_property.h
 * @brief  eCAL core function interface
**/

#pragma once

#include <string>
#include <nanobind/nanobind.h>

inline nanobind::bytes std_string_to_nb_bytes(const std::string& cpp_string)
{
  return nanobind::bytes(cpp_string.c_str(), cpp_string.size());
}

inline std::string nb_bytes_to_std_string(const nanobind::bytes& nb_bytes)
{
  return std::string(nb_bytes.c_str(), nb_bytes.size());
}


// This is a nb::def_visitor, which binds a std::string to a nb::bytes object
// In eCAL, we often use std::string as a container for bytes, so these types should rather be 
// mapped to python bytes objects and not strings.
template <class T>
struct bytestring_property : nanobind::def_visitor<bytestring_property<T>> {
    bytestring_property(const char* name, std::string T::*mptr) : name(name), mptr(mptr) {}
    const char* name;
    std::string T::*mptr;
    
    template <typename Class, typename... Extra>
    void execute(Class &cl, const Extra&... extra) {
        cl.def_prop_rw(name,
                       [mptr=mptr](const T& self) { return std_string_to_nb_bytes(self.*mptr); },
                       [mptr=mptr](T& self, nanobind::bytes val) { (self.*mptr) = nb_bytes_to_std_string(val); });
    }
};

