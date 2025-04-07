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
 * @file   auxiliaries.h
 * @brief  eCAL auxiliary functions
**/

#ifndef ecal_c_auxiliaries_h_included
#define ecal_c_auxiliaries_h_included

#include <ecal_c/export.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief Allocate a memory block for passing it to eCAL
   * 
   * This function is suppose to be used when the user needs to pass memory within the eCAL context, e.g. within a callback.
   *
   * @param size_ Size of the allocated memory block.
   * 
   * @return Pointer to the allocated memory block if succeeded, NULL otherwise.
  **/
  ECALC_API void* eCAL_Malloc(size_t size_);

  /**
   * @brief Free a memory block which has been passed from eCAL
   *
   * This function basically needs to be called when eCAL returns a non-constant pointer by value or by reference to the user.
   *
   * @param ptr_ Non constant pointer to be freed.
  **/
  ECALC_API void eCAL_Free(void* ptr_);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_auxiliaries_h_included*/