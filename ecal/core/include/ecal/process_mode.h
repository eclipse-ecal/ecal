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
 * @file   process_mode.h
 * @brief  eCAL process start mode
**/

#pragma once

/**
 * @brief  Process start mode (StartProcess) 
**/
namespace eCAL
{
  namespace Process
  {
    enum class eStartMode
    {
      normal = 0,     /*!<  0 == start mode normal     */
      hidden = 1,     /*!<  1 == start mode hidden     */
      minimized = 2,  /*!<  2 == start mode minimized  */
      maximized = 3,  /*!<  3 == start mode maximized  */
    };
  }
}