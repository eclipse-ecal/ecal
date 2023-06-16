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


#pragma once

#include <ecal/service/logger.h>

/////////////////////////////////////////////
// Enable / disable debug logging
/////////////////////////////////////////////

//
// If DEBUG_VERBOSE is not set by the user, we enable it when compiling in
// Debug mode. The user can still omit those verbose messages by setting a
// logger function that drops the verbose messages.
//
#ifndef ECAL_SERVICE_LOG_DEBUG_VERBOSE_ENABLED
  #ifdef NDEBUG
    #define ECAL_SERVICE_LOG_DEBUG_VERBOSE_ENABLED 0
  #else
    #define ECAL_SERVICE_LOG_DEBUG_VERBOSE_ENABLED 1
  #endif // NDEBUG
#endif // !ECAL_SERVICE_LOG_DEBUG_VERBOSE_ENABLED

//
// If the DEBUG_VERBOSE is enabled, we also enable the normal DEBUG logging
//
#if (ECAL_SERVICE_LOG_DEBUG_VERBOSE_ENABLED)
  #define ECAL_SERVICE_LOG_DEBUG_ENABLED 1
#endif

//
// If we haven't decided yet whether DEBUG logging shall be enabled, we enable
// it when compiling in Debug mode and disable it otherwise.
//
#ifndef ECAL_SERVICE_LOG_DEBUG_ENABLED
  #ifdef NDEBUG
    #define ECAL_SERVICE_LOG_DEBUG_ENABLED 0
  #else
    #define ECAL_SERVICE_LOG_DEBUG_ENABLED 1
  #endif // NDEBUG
#endif // !ECAL_SERVICE_LOG_DEBUG_ENABLED

/////////////////////////////////////////////
// Debug logging macros
/////////////////////////////////////////////

//
// Debug logging macro that evaluates to nothing, if Debug logging is disabled.
// This lets the compiler better optimize the code. 
//
#if ECAL_SERVICE_LOG_DEBUG_ENABLED
  #define ECAL_SERVICE_LOG_DEBUG(logger, msg) logger(eCAL::service::LogLevel::Debug, msg)
#else
  #define ECAL_SERVICE_LOG_DEBUG(...) /**/
#endif

//
// Debug Verbose logging macro that evaluates to nothing, if Debug Verbose logging is disabled.
// This lets the compiler better optimize the code. 
//
#if ECAL_SERVICE_LOG_DEBUG_VERBOSE_ENABLED
  #define ECAL_SERVICE_LOG_DEBUG_VERBOSE(logger,msg) do { logger(eCAL::service::LogLevel::DebugVerbose, msg); } while (0)
#else
  #define ECAL_SERVICE_LOG_DEBUG_VERBOSE(...) /**/
#endif

