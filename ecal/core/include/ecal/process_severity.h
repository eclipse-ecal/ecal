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

#pragma once

namespace eCAL
{
  namespace Process
  {

    /**
     * @brief  Process severity
    **/
    enum class eSeverity
    {
      unknown  = 0,  /*!<  0 == condition unknown     */
      healthy  = 1,  /*!<  1 == process healthy       */
      warning  = 2,  /*!<  2 == process warning level */
      critical = 3,  /*!<  3 == process critical      */
      failed   = 4,  /*!<  4 == process failed        */
    };


    /**
     * @brief Process Severity Level
     *
     * enumerations for ECAL_API::SetState functionality
     * where the lowest process severity is generally proc_sev_level1
    **/
    enum class eSeverityLevel
    {
      level1 = 1,  /*!<  default severity level 1 */
      level2 = 2,  /*!<  severity level 2         */
      level3 = 3,  /*!<  severity level 3         */
      level4 = 4,  /*!<  severity level 4         */
      level5 = 5,  /*!<  severity level 5         */
    };
  }
}