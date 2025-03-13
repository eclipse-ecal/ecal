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

#ifndef ecal_c_log_level_h_included
#define ecal_c_log_level_h_included


enum eCAL_Logging_eLogLevel
{
  eCAL_Logging_log_level_none = 0,
  eCAL_Logging_log_level_all = 255,
  eCAL_Logging_log_level_info = 1,
  eCAL_Logging_log_level_warning = 2,
  eCAL_Logging_log_level_error = 4,
  eCAL_Logging_log_level_fatal = 8,
  eCAL_Logging_log_level_debug1 = 16,
  eCAL_Logging_log_level_debug2 = 32,
  eCAL_Logging_log_level_debug3 = 64,
  eCAL_Logging_log_level_debug4 = 128,
};

typedef unsigned char eCAL_Logging_Filter;

#endif /*ecal_c_log_level_h_included*/
