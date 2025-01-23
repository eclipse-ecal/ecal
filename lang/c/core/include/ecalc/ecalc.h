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
 * @file   ecalc.h
 * @brief  eCAL main c header file
**/

#ifndef ecalc_h_included
#define ecalc_h_included

#ifndef _MSC_VER
  #include <string.h>
#endif

// all ecal c includes
#include <ecal/ecalc_export.h>
#include <ecal/ecalc_types.h>
#include <ecalc/client_cimpl.h>
#include <ecalc/core_cimpl.h>
#include <ecalc/log_cimpl.h>
#include <ecalc/monitoring_cimpl.h>
#include <ecalc/process_cimpl.h>
#include <ecalc/publisher_cimpl.h>
#include <ecalc/server_cimpl.h>
#include <ecalc/subscriber_cimpl.h>
#include <ecalc/time_cimpl.h>
#include <ecalc/timer_cimpl.h>
#include <ecalc/tlayer_cimpl.h>
#include <ecalc/util_cimpl.h>

#endif /* ecalc_h_included */
