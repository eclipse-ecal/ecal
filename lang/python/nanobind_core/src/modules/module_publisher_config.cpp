/* ========================= eCAL LICENSE =================================
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
 * ========================= eCAL LICENSE =================================
*/

/**
 * @brief  Add ServiceClient class to nanobind module
**/


#include <modules/module_publisher_config.h>
#include <wrappers/wrapper_publisher_config.h>

void AddPublisherConfigStructToModule(nanobind::module_& module)
{
    // Struct eCAL::SDataTypeInformation
  nanobind::class_<eCAL::CNBPublisherConfigStruct>(module, "PublisherConfiguration");
     //   .def(nanobind::init<>())
     //   .def_rw("name", &eCAL::CNBDataTypeInformation::name)
     //   .def_rw("encoding", &eCAL::CNBDataTypeInformation::encoding)
     //   .def_rw("descriptor", &eCAL::CNBDataTypeInformation::descriptor);
}
