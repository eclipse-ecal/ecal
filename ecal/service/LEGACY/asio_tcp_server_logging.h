///* ========================= eCAL LICENSE =================================
// *
// * Copyright (C) 2016 - 2019 Continental Corporation
// *
// * Licensed under the Apache License, Version 2.0 (the "License");
// * you may not use this file except in compliance with the License.
// * You may obtain a copy of the License at
// * 
// *      http://www.apache.org/licenses/LICENSE-2.0
// * 
// * Unless required by applicable law or agreed to in writing, software
// * distributed under the License is distributed on an "AS IS" BASIS,
// * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// * See the License for the specific language governing permissions and
// * limitations under the License.
// *
// * ========================= eCAL LICENSE =================================
//*/
//
//#pragma once
//
//// TODO: It may be better to use the eCAL logging instead of cout / cerr
//
////
//// If DEBUG_VERBOSE is not set by the user, we enable it when compiling in
//// Debug mode.
////
//#ifndef ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED
//  #ifdef NDEBUG
//    #define ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED 0
//  #else
//    #define ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED 1
//  #endif // NDEBUG
//#endif // !ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED
//
////
//// If the DEBUG_VERBOSE is enabled, we also enable the normal DEBUG logging
////
//#if (ECAL_ASIO_TCP_SERVER_LOG_DEBUG_VERBOSE_ENABLED)
//  #define ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED 1
//#endif
//
////
//// If we haven't decided yet whether DEBUG logging shall be enabled, we enable
//// it when compiling in Debug mode and disable it otherwise.
////
//#ifndef ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED
//  #ifdef NDEBUG
//    #define ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED 0
//  #else
//    #define ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED 1
//  #endif // NDEBUG
//#endif // !ECAL_ASIO_TCP_SERVER_LOG_DEBUG_ENABLED