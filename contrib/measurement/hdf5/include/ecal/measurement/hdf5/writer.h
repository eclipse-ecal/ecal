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

/**
 * @file   eh5_writer.h
 * @brief  Hdf5 based Writer implementation
**/

#pragma once

#include <functional>
#include <set>
#include <string>
#include <memory>

#include <ecal/measurement/base/writer.h>

namespace eCAL
{
  namespace experimental
  {
    namespace measurement
    {
      namespace hdf5
      {
        std::unique_ptr<measurement::base::Writer> CreateWriter(const base::WriterOptions& writer);
      }  // namespace hdf5
    }  // namespace measurement
  }  // namespace experimental 
}  // namespace eCAL
