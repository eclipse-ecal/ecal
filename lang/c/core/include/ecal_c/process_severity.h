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
 * @file   process_severity.h
 * @brief  eCAL process severity
**/

#ifndef ecal_c_process_severity_h_included
#define ecal_c_process_severity_h_included

/**
 * @brief  Process severity
**/
enum eCAL_Process_eSeverity
{
  eCAL_Process_eSeverity_unknown = 0,  /*!<  0 == condition unknown     */
  eCAL_Process_eSeverity_healthy = 1,  /*!<  1 == process healthy       */
  eCAL_Process_eSeverity_warning = 2,  /*!<  2 == process warning level */
  eCAL_Process_eSeverity_critical = 3,  /*!<  3 == process critical      */
  eCAL_Process_eSeverity_failed = 4,  /*!<  4 == process failed        */
};


/**
 * @brief Process Severity Level
 *
 * enumerations for ECAL_API::SetState functionality
 * where the lowest process severity is generally proc_sev_level1
**/
enum eCAL_Process_eSeverityLevel
{
  eCAL_Process_eSeverityLevel_level1 = 1,  /*!<  default severity level 1 */
  eCAL_Process_eSeverityLevel_level2 = 2,  /*!<  severity level 2         */
  eCAL_Process_eSeverityLevel_level3 = 3,  /*!<  severity level 3         */
  eCAL_Process_eSeverityLevel_level4 = 4,  /*!<  severity level 4         */
  eCAL_Process_eSeverityLevel_level5 = 5,  /*!<  severity level 5         */
};

#endif /*ecal_c_process_severity_h_included*/