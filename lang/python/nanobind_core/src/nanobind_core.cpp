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
#include <ecal/types.h>

#include <core/py_config.h>
#include <core/py_core.h>
#include <core/py_init.h>
#include <core/py_log.h>
#include <core/py_log_level.h>
#include <core/py_monitoring.h>
#include <core/py_process.h>
#include <core/py_process_severity.h>
#include <core/py_registration.h>
#include <core/py_time.h>
#include <core/py_timer.h>
#include <core/py_types.h>
#include <core/py_util.h>
#include <core/config/py_application.h>
#include <core/config/py_configuration.h>
#include <core/config/py_logging.h>
#include <core/config/py_publisher.h>
#include <core/config/py_registration.h>
#include <core/config/py_subscriber.h>
#include <core/config/py_time.h>
#include <core/config/py_transport_layer.h>
#include <core/pubsub/py_payload_writer.h>
#include <core/pubsub/py_publisher.h>
#include <core/pubsub/py_subscriber.h>
#include <core/pubsub/py_types.h>
#include <core/service/py_client.h>
#include <core/service/py_client_instance.h>
#include <core/service/py_server.h>
#include <core/service/py_types.h>
#include <core/types/py_custom_data_types.h>
#include <core/types/py_logging.h>
#include <core/types/py_monitoring.h>


NB_MODULE(nanobind_core, m) {
  AddConfig(m);
  AddCore(m);
}
