// Copyright (c) Continental. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for details.

#pragma once

#include <tcpub/tcpub_logger.h>

//
// If DEBUG_VERBOSE is not set by the user, we enable it when compiling in
// Debug mode. The user can still omit those verbose messages by setting a
// logger function that drops the verbose messages.
//
#ifndef TCPUB_LOG_DEBUG_VERBOSE_ENABLED
  #ifdef NDEBUG
    #define TCPUB_LOG_DEBUG_VERBOSE_ENABLED 0
  #else
    #define TCPUB_LOG_DEBUG_VERBOSE_ENABLED 1
  #endif // NDEBUG
#endif // !TCPUB_LOG_DEBUG_VERBOSE_ENABLED

//
// If the DEBUG_VERBOSE is enabled, we also enable the normal DEBUG logging
//
#if (TCPUB_LOG_DEBUG_VERBOSE_ENABLED)
  #define TCPUB_LOG_DEBUG_ENABLED 1
#endif

//
// If we haven't decided yet whether DEBUG logging shall be enabled, we enable
// it when compiling in Debug mode and disable it otherwise.
//
#ifndef TCPUB_LOG_DEBUG_ENABLED
  #ifdef NDEBUG
    #define TCPUB_LOG_DEBUG_ENABLED 0
  #else
    #define TCPUB_LOG_DEBUG_ENABLED 1
  #endif // NDEBUG
#endif // !TCPUB_LOG_DEBUG_ENABLED
