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
 * @file   writer_factory.h
 * @brief  Class for creating writer objects
**/

#pragma once

#include <ecal/measurement/base/writer.h>

namespace eCAL
{
 namespace measurement
 {
   enum class MeasurementFormat {
     HDF5 = 0,
     MCAP = 1
   };
   
   std::unique_ptr<base::Writer> CreateWriter();
   std::unique_ptr<base::Writer> CreateWriter(MeasurementFormat format);
 } 
}


