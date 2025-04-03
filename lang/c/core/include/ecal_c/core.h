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
 * @file   ecal_c/core.h
 * @brief  eCAL core function c interface
**/

#ifndef ecal_c_core_h_included
#define ecal_c_core_h_included

#include <ecal_c/export.h>
#include <ecal_c/types.h>
#include <ecal_c/init.h>
#include <ecal_c/config/configuration.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
  /**
   * @brief  Get eCAL version string. 
   *
   * @return  Full eCAL version string. 
  **/
  ECALC_API const char* eCAL_GetVersionString();

  /**
   * @brief  Get eCAL version date. 
   *
   * @return  Full eCAL version date string. 
  **/
  ECALC_API const char* eCAL_GetVersionDateString();

  /**
   * @brief  Get eCAL version as separated integer values. 
   *
   * @return eCAL version struct with seperate version values
   *
  **/
  ECALC_API struct eCAL_SVersion eCAL_GetVersion();

  /**
   * @brief Initialize eCAL API.
   *
   * @param unit_name_   Defines the name of the eCAL unit. Optional, can be NULL.
   * @param components_  Defines which component to initialize. Optional, can be NULL.
   * @param config_      Configuration with which eCal is to be initialized. Optional, can be NULL.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_Initialize(const char *unit_name_, const unsigned int* components_, const eCAL_Configuration* config_);


  /**
   * @brief Finalize eCAL API.
   *
   * @return Zero if succeeded, non-zero otherwise.
  **/
  ECALC_API int eCAL_Finalize();

  /**
   * @brief Check eCAL initialize state.
   *
   * @return Non-zero if eCAL is initialized, zero otherwise.
  **/
  ECALC_API int eCAL_IsInitialized();

  /**
 * @brief Check initialize state of components.
 *
 * @param components_ Components to be checked.
 * 
 * @return Non-zero if eCAL is initialized, zero otherwise.
**/
  ECALC_API int eCAL_IsComponentInitialized(unsigned int components_);

  /**
   * @brief Return the eCAL process state.
   *
   * @return Non-zero if eCAL is in proper state, zero otherwise. 
  **/
  ECALC_API int eCAL_Ok();
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*ecal_c_core_h_included*/
