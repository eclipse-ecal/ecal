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
 * @brief  eCAL core functions
**/

#include "ecal_def.h"
#include "ecal_event.h"
#include "ecal_globals.h"
#include <string>
#include <vector>

#if ECAL_CORE_COMMAND_LINE
#include "util/advanced_tclap_output.h"
#endif

#include <algorithm>

namespace
{
  const eCAL::EventHandleT& ShutdownProcEvent()
  {
    static eCAL::EventHandleT evt;
    static const std::string event_name(EVENT_SHUTDOWN_PROC + std::string("_") + std::to_string(eCAL::Process::GetProcessID()));
    if (!gEventIsValid(evt))
    {
      gOpenNamedEvent(&evt, event_name, true);
    }
    return(evt);
  }
}

namespace eCAL
{
 /**
   * @brief  Get eCAL version string. 
   *
   * @return  Full eCAL version string. 
  **/
  std::string GetVersionString()
  {
    return ECAL_VERSION;
  }

  /**
   * @brief  Get eCAL version date. 
   *
   * @return  Full eCAL version date string. 
  **/
  std::string GetVersionDateString()
  {
    return ECAL_DATE;
  }

  /**
   * @brief  Get eCAL version as separated integer values. 
   *
   * @return struct SVersion that contains major, minor and patch value.
  **/
  SVersion GetVersion()
  {
    return SVersion{ ECAL_VERSION_MAJOR, ECAL_VERSION_MINOR, ECAL_VERSION_PATCH };
  }

  /**
   * @brief Initialize eCAL API.
   *
   * @param unit_name_   Defines the name of the eCAL unit.
   * @param components_  Defines which component to initialize.
   *
   * @return True if succeeded.
  **/
  bool Initialize(const std::string& unit_name_ /*= ""*/, unsigned int components_ /*= Init::Default*/)
  {
    eCAL::Configuration config;
    config.InitFromConfig();

    return Initialize(config, unit_name_, components_);
  }

  /**
   * @brief Initialize eCAL API.
   *
   * @param config_      User defined configuration object.
   * @param unit_name_   Defines the name of the eCAL unit.
   * @param components_  Defines which component to initialize.     
   * 
   * @return True if succeeded
  **/
  bool Initialize(eCAL::Configuration& config_, const std::string& unit_name_ /*= nullptr*/, unsigned int components_ /*= Init::Default*/)
  {
    InitGlobals();
    
    g_ecal_configuration = config_;

    SetGlobalUnitName(unit_name_.c_str());

    g_globals_ctx_ref_cnt++;

     // (post)initialize single components
    const auto success = g_globals()->Initialize(components_);
    
    return success;
  }

  /**
   * @brief Check eCAL initialize state.
   *
   * @param component_  Check specific component or 0 for general state of eCAL core.
   *
   * @return True if eCAL is initialized.
  **/
  bool IsInitialized()
  {
    if (g_globals_ctx == nullptr)
      return false;

    return g_globals()->IsInitialized();
  }

  /**
   * @brief Check eCAL initialize state.
   *
   * @param component_  Check specific component or 0 for general state of eCAL core.
   *
   * @return True if component is initialized.
  **/
  bool IsInitialized(unsigned int component_)
  {
    if (g_globals_ctx == nullptr)
      return false;

    return g_globals()->IsInitialized(component_);
  }

  /**
   * @brief Finalize eCAL API.
   *
   * @return True if succeeded.
  **/
  bool Finalize()
  {
    if (g_globals_ctx == nullptr)
      return false;
    g_globals_ctx_ref_cnt--;
    if (g_globals_ctx_ref_cnt > 0)
      return true;
    const auto ret = g_globals()->Finalize();
    delete g_globals_ctx;
    g_globals_ctx = nullptr;
    return ret;
  }

  /**
   * @brief Return the eCAL process state.
   *
   * @return  True if eCAL is in proper state.
  **/
  bool Ok()
  {
    const bool ecal_is_ok = (g_globals_ctx != nullptr) && !gWaitForEvent(ShutdownProcEvent(), 0);
    return(ecal_is_ok);
  }
}
