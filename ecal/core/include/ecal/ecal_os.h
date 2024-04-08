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
 * @file   ecal_os.h
 * @brief  eCAL os interface
**/

#pragma once

#if defined(_WIN32)
#define ECAL_OS_WINDOWS
#endif

#if defined(__linux__)
#define ECAL_OS_LINUX
#endif

#if defined(__APPLE__)
#define ECAL_OS_LINUX // macOS does not differ that much from linux
#define ECAL_OS_MACOS
#endif

#if defined(__QNXNTO__)
#define ECAL_OS_LINUX
#define ECAL_OS_QNX
#endif

#if defined(__FreeBSD__)
#define ECAL_OS_LINUX
#define ECAL_OS_FREEBSD
#endif

// The following is adapted from https://gcc.gnu.org/wiki/Visibility
// Helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define ECAL_HELPER_DLL_IMPORT __declspec(dllimport)
  #define ECAL_HELPER_DLL_EXPORT __declspec(dllexport)
  #define ECAL_HELPER_DLL_LOCAL
  #define ECAL_HELPER_DEPRECATED __declspec(deprecated)
#elif defined(__GNUC__) || defined(__clang__)
  #define ECAL_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
  #define ECAL_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
  #define ECAL_HELPER_DLL_LOCAL __attribute__ ((visibility ("hidden")))
  #define ECAL_HELPER_DEPRECATED __attribute__ ((visibility ("default")))
#else
  #error "Symbol visibility scheme unknown for your compiler, please update ecal_os.h"
#endif

// Helper definitions for API marking
#ifdef ECAL_STATIC // eCAL is static library
  #define ECAL_API
  #define ECAL_API_DEPRECATED
  #define ECALC_API
  #define ECALC_API_DEPRECATED
#else // eCAL is a shared library
  #ifdef eCAL_EXPORTS // We are building eCAL
    #define ECALC_API ECAL_HELPER_DLL_EXPORT
    #define ECALC_API_DEPRECATED ECAL_HELPER_DLL_EXPORT ECAL_HELPER_DEPRECATED
  #else // We are using eCAL
    #define ECALC_API ECAL_HELPER_DLL_IMPORT
    #define ECALC_API_DEPRECATED ECAL_HELPER_DLL_IMPORT ECAL_HELPER_DEPRECATED
  #endif

  #ifdef ECAL_C_DLL //Dont interpret eCAL C++ symbols as available
    #define ECAL_API
    #define ECAL_API_DEPRECATED
  #else
    #define ECAL_API ECALC_API
    #define ECAL_API_DEPRECATED ECALC_API_DEPRECATED
  #endif
#endif
