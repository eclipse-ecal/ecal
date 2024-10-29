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

#pragma once

#include <optional>

template<typename base>
struct Invalidated;

template<typename base>
struct Validated 
 : base
 {
private:
  Validated() = default;
  Validated(const base& base_)
   : base(base_)
    {};

  friend struct Invalidated<base>;  
 };

template<typename base>
struct Invalidated
 : base
 {

  std::optional<const Validated<base>> GetValidated()
  {
    if (Validate(*this))
    {
       return Validated<base>(*this);
    }
    else
    {
        return std::nullopt;
    }
  }

 };