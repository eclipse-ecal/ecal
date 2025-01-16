
.. _program_listing_file_ecal_core_include_ecal_ecal_os.h:

Program Listing for File ecal_os.h
==================================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_ecal_os.h>` (``ecal/core/include/ecal/ecal_os.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
   
   #ifdef _MSC_VER
     #ifdef eCAL_EXPORTS
       #define ECALC_API __declspec(dllexport)
     #else /* eCAL_EXPORTS */
       #define ECALC_API __declspec(dllimport)
     #endif /* eCAL_EXPORTS */
     #ifdef ECAL_C_DLL
       #define ECAL_API
     #else /* ECAL_C_DLL */
       #define ECAL_API ECALC_API
     #endif /* ECAL_C_DLL */
   #else /* _MSC_VER */
     #define ECALC_API
     #define ECAL_API
   #endif
   
   #ifdef _MSC_VER
     #ifdef eCAL_EXPORTS
       #define ECALC_API_DEPRECATED __declspec(dllexport deprecated)
     #else /* eCAL_EXPORTS */
       #define ECALC_API_DEPRECATED __declspec(dllimport deprecated)
     #endif /* eCAL_EXPORTS */
   #elif defined(__GNUC__) || defined(__clang__)
     #define ECALC_API_DEPRECATED __attribute__((deprecated))
   #else
     #pragma message("WARNING: You need to implement DEPRECATED for this compiler")
     #define ECALC_API_DEPRECATED
   #endif
