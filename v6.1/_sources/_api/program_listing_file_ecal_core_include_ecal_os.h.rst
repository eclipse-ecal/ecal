
.. _program_listing_file_ecal_core_include_ecal_os.h:

Program Listing for File os.h
=============================

|exhale_lsh| :ref:`Return to documentation for file <file_ecal_core_include_ecal_os.h>` (``ecal/core/include/ecal/os.h``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

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
     #ifdef ECAL_CORE_EXPORTS
       #define ECAL_API __declspec(dllexport)
     #elif defined(ECAL_CORE_IMPORTS)
       #define ECAL_API __declspec(dllimport)
     #else 
       #define ECAL_API
     #endif
   
     #define ECAL_API_CLASS
     #define ECAL_API_EXPORTED_MEMBER ECAL_API
   #else /* _MSC_VER */
     #define ECAL_API_CLASS __attribute__((visibility("default")))
     #ifdef ECAL_CORE_EXPORTS 
       #define ECAL_API __attribute__((visibility("default")))
     #else
       #define ECAL_API
     #endif
   
     #define ECAL_API_CLASS __attribute__((visibility("default")))
     #define ECAL_API_EXPORTED_MEMBER 
   #endif
