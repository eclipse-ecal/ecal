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
 * @file   ecal_c/log.h
 * @brief  eCAL logging c interface
**/

#ifndef ecal_c_log_h_included
#define ecal_c_log_h_included

#include <ecal_c/export.h>
#include <ecal_c/log_level.h>
#include <ecal_c/types/logging.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Log a message.
   *
   * @param level_   The level.
   * @param message_ The log message string.
  **/
  ECALC_API void eCAL_Logging_Log(enum eCAL_Logging_eLogLevel level_, const char* message_);

  /**
   * @brief Get logging as serialized protobuf buffer.
   *
   * @param [out] logging_buffer_         Pointer to a protobuf serialized log buffer. Must point to NULL and needs to be released by eCAL_Free().
   * @param [out] logging_buffer_length_  Length of the log buffer. Must point to zero.
   * 
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_Logging_GetLoggingBuffer(void** logging_buffer_, size_t* logging_buffer_length_);

  /**
   * @brief Get logging as deserialized log messages.
   *
   * @param [out] logging_     Pointer to deserialized log messages. Must point to NULL and needs to be released by eCAL_Free().
   * @param [out] logging_     Pointer to deserialized log messages. Must point to NULL and needs to be released by eCAL_Free().
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_Logging_GetLogging(struct eCAL_Logging_SLogging** logging_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_log_h_included*/
