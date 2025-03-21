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
 * @file   ecal_c/ecal.h
 * @brief  eCAL main c header file
**/

#ifndef ecal_c_ecal_h_included
#define ecal_c_ecal_h_included

#ifndef _MSC_VER
  #include <string.h>
#endif

// all ecal c includes
#include <ecal_c/config/application.h>
#include <ecal_c/config/configuration.h>
#include <ecal_c/config/logging.h>
#include <ecal_c/config/publisher.h>
#include <ecal_c/config/registration.h>
#include <ecal_c/config/subscriber.h>
#include <ecal_c/config/time.h>
#include <ecal_c/config/transport_layer.h>
#include <ecal_c/pubsub/payload_writer.h>
#include <ecal_c/pubsub/publisher.h>
#include <ecal_c/pubsub/subscriber.h>
#include <ecal_c/pubsub/types.h>
#include <ecal_c/service/client.h>
#include <ecal_c/service/client_instance.h>
#include <ecal_c/service/server.h>
#include <ecal_c/service/types.h>
#include <ecal_c/types/custom_data_types.h>
#include <ecal_c/types/monitoring.h>
#include <ecal_c/auxiliaries.h>
#include <ecal_c/config.h>
#include <ecal_c/core.h>
#include <ecal_c/export.h>
#include <ecal_c/init.h>
#include <ecal_c/log.h>
#include <ecal_c/log_level.h>
#include <ecal_c/monitoring.h>
#include <ecal_c/process.h>
#include <ecal_c/process_severity.h>
#include <ecal_c/registration.h>
#include <ecal_c/time.h>
#include <ecal_c/timer.h>
#include <ecal_c/types.h>
#include <ecal_c/util.h>

#endif /* ecal_c_ecal_h_included */
