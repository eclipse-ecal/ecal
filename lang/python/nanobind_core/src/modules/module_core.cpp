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
 * @brief  Add ecal_core functions to nanobind module
**/

#include <modules/module_core.h>
#include <ecal/ecal_core.h>
#include <nanobind/stl/tuple.h>

void AddCoreFuncToModule(nanobind::module_& module)
{
    // Functions from ecal_core.h
    module.def("get_version_string", []() { return eCAL::GetVersionString(); });
    module.def("get_version_date", []() { return eCAL::GetVersionDateString(); });
    module.def("set_unitname", [](const std::string& nb_unit_name) { return eCAL::SetUnitName(nb_unit_name.c_str()); });
    module.def("is_initialized", []() { return eCAL::IsInitialized(); });
    module.def("finalize", []() { return eCAL::Finalize(); });
    module.def("ok", []() { return eCAL::Ok(); });

    module.def("get_version", []()
        {
          int nb_major;
          int nb_minor; 
          int nb_patch;
          int status = eCAL::GetVersion(&nb_major, &nb_minor, &nb_patch);
          return std::make_tuple(status, nb_major, nb_minor, nb_patch);
        });
    //   m.def("initialize", [](int nb_argc_, char *nb_argv_, const char* nb_unit_name_, unsigned int nb_components_)
    //       { return eCAL::Initialize(nb_argc_, nb_argv_, nb_unit_name_, nb_components_); });
    module.def("initialize", [](std::vector<std::string> nb_args_, std::string nb_unit_name_)
        { return eCAL::Initialize(nb_args_, nb_unit_name_.c_str(), eCAL::Init::Default); });
    module.def("initialize", []()
        { return eCAL::Initialize(); });

}
