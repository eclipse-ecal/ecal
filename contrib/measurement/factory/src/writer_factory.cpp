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

#include <ecal/measurement/writer_factory.h>

#ifdef ECAL_MEASUREMENT_SUPPORTS_HDF5
#include <ecalhdf5/eh5_writer.h>
#endif

#ifdef ECAL_MEASUREMENT_SUPPORTS_MCAP
#include <ecal/measurement/mcap/writer.h>
#endif

namespace eCAL
{
 namespace measurement
 {
   std::unique_ptr<base::Writer> CreateWriter()
   {
     return std::make_unique<eCAL::measurement::mcap::Writer>();
   }
      
   std::unique_ptr<base::Writer> CreateWriter(MeasurementFormat format)
   {
     switch (format)
     {
     case MeasurementFormat::HDF5:
       return std::make_unique<eh5::Writer>();
     case MeasurementFormat::MCAP:
       return std::make_unique<eCAL::measurement::mcap::Writer>();
     default:
       return std::make_unique<eh5::Writer>();
     }
   }   
 } 
}

