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
 * @file   ecal_c/export.h
 * @brief  eCAL dll / so export commands
**/

#ifndef ecal_c_export_h_included
#define ecal_c_export_h_included

#ifdef _MSC_VER
  #ifdef ECAL_CORE_C_EXPORTS
    #define ECALC_API __declspec(dllexport)
    #define ECALC_API_DEPRECATED __declspec(dllexport deprecated)
  #elif defined(ECAL_CORE_C_IMPORTS)
    #define ECALC_API __declspec(dllimport)
    #define ECALC_API_DEPRECATED __declspec(dllimport deprecated)
  #else 
    #define ECALC_API
    #define ECALC_API_DEPRECATED __declspec(deprecated)
  #endif
#else /* _MSC_VER */
  #ifdef ECAL_CORE_C_EXPORTS 
    #define ECALC_API __attribute__((visibility("default")))
  #else
    #define ECALC_API
  #endif
  #define ECALC_API_DEPRECATED __attribute__((deprecated)) ECALC_API 
#endif

#endif /*ecal_c_export_h_included*/