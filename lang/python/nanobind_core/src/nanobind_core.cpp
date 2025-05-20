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
 * @brief  Main Nanobind module which Adds all other needed classes and functions
**/

#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <ecal/ecal.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/tuple.h>
#include <cstdint>
#include <chrono>
#include <memory>
#include <string>
#include <cstddef>
#include <ecal/ecal_types.h>

#include <modules/module_client.h>
#include <modules/module_core.h>
#include <modules/module_datatypeinfo.h>
#include <modules/module_publisher.h>
#include <modules/module_application_config.h>
#include <modules/module_configuration_config.h>
#include <modules/module_monitoring_config.h>
#include <modules/module_publisher_config.h>
#include <modules/module_registration_config.h>
#include <modules/module_service_config.h>
#include <modules/module_server.h>
#include <modules/module_subscriber.h>
#include <modules/module_logging_config.h>
#include <modules/module_subscriber_config.h>
#include <modules/module_time_config.h>
#include <modules/module_transport_layer_config.h>
#include <modules/module_user_arguments_config.h>
#include <modules/module_util.h>


NB_MODULE(nanobind_core, m) {

    AddDataTypeInfoStructToModule(m);
    AddSubscriberClassToModule(m);
    AddPublisherClassToModule(m);
    AddClientClassToModule(m);
    AddServerClassToModule(m);
    AddApplicationConfigStructToModule(m);
    AddConfigurationConfigStructToModule(m);
    AddLoggingConfigStructToModule(m);
    AddServiceConfigStructToModule(m);
    AddRegistrationConfigStructToModule(m);
    AddMonitoringConfigStructToModule(m);
    AddPublisherConfigStructToModule(m);
    AddSubscriberConfigStructToModule(m);
    AddTimeConfigStructToModule(m);
    AddTransportLayerConfigStructToModule(m);
    AddUserArgumentsConfigStructToModule(m);

    AddCoreFuncToModule(m);
    AddUtilFuncToModule(m);
}
