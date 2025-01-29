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

/**
 * @file   ecal_c/types.h
 * @brief  File including shared types for eCAL C API
**/

#ifndef ecal_c_types_h_included
#define ecal_c_types_h_included

/**
 * @brief Flag to indicate eCAL to allocate/deallocate memory.
**/
#define ECAL_ALLOCATE_4ME 0

/**
 * @brief Common handle for eCAL C API function calls.
**/
typedef void* ECAL_HANDLE;

/**
 * @brief eCAL version struct (C variant)
 **/
typedef struct
{
  int major; //!< major version number
  int minor; //!< minor version number
  int patch; //!< patch version number
} eCAL_SVersion;

#endif /* ecal_c_types_h_included */
