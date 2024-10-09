/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
#include "config/ecal_cmd_parser.h"
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
  const char* GetVersionString()
  {
    return(ECAL_VERSION);
  }

  /**
   * @brief  Get eCAL version date. 
   *
   * @return  Full eCAL version date string. 
  **/
  const char* GetVersionDateString()
  {
    return(ECAL_DATE);
  }

  /**
   * @brief  Get eCAL version as separated integer values. 
   *
   * @param [out] major_  The eCAL major version number.
   * @param [out] minor_  The eCAL minor version number.
   * @param [out] patch_  The eCAL patch version number.
   *
   * @return  Zero if succeeded.
  **/
  int GetVersion(int* major_, int* minor_, int* patch_)
  {
    if((major_ == nullptr) || (minor_ == nullptr) || (patch_ == nullptr)) return(-1);
    *major_ = ECAL_VERSION_MAJOR;
    *minor_ = ECAL_VERSION_MINOR;
    *patch_ = ECAL_VERSION_PATCH;
    return(0);
  }

  /**
   * @brief Initialize eCAL API.
   *
   * @param argc_        Number of command line arguments. 
   * @param argv_        Array of command line arguments. 
   * @param unit_name_   Defines the name of the eCAL unit. 
   * @param components_  Defines which component to initialize.
   *
   * @return Zero if succeeded, 1 if already initialized, -1 if failed.
  **/
  int Initialize(int argc_ , char **argv_, const char *unit_name_, unsigned int components_)
  {
    eCAL::Configuration config(argc_, argv_);

    return Initialize(config, unit_name_, components_);
  }

  /**
   * @brief Initialize eCAL API.
   *
   * @param args_        Vector of config arguments to overwrite (["arg1", "value1", "arg2", "arg3", "value3" ..]).
   * @param unit_name_   Defines the name of the eCAL unit.
   * @param components_  Defines which component to initialize.
   *
   * @return Zero if succeeded, 1 if already initialized, -1 if failed.
  **/
  int Initialize(std::vector<std::string> args_, const char *unit_name_, unsigned int components_) //-V826
  {
    eCAL::Configuration config(args_);

    return Initialize(config, unit_name_, components_);
  }

  /**
   * @brief Initialize eCAL API.
   *
   * @param config_      User defined configuration object.
   * @param unit_name_   Defines the name of the eCAL unit.
   * @param components_  Defines which component to initialize.     
   * 
   * @return Zero if succeeded, 1 if already initialized, -1 if failed.
  **/
  int Initialize(eCAL::Configuration& config_, const char *unit_name_ /*= nullptr*/, unsigned int components_ /*= Init::Default*/)
  {
    g_ecal_configuration = config_;

    if (unit_name_ != nullptr)
    {
      SetGlobalUnitName(unit_name_);
    }

    g_globals_ctx_ref_cnt++;

     // (post)initialize single components
    const int success = g_globals()->Initialize(components_);

    if (config_.command_line_arguments.dump_config)
    {
      Process::DumpConfig();
    }
    
    return success;
  }

  /**
   * @brief Check eCAL initialize state.
   *
   * @param component_  Check specific component or 0 for general state of eCAL core.
   *
   * @return 1 if eCAL is initialized.
  **/
  int IsInitialized(unsigned int component_)
  {
    if (g_globals_ctx == nullptr) return(0);
    if(g_globals()->IsInitialized(component_)) return(1);
    return(0);
  }

  /**
   * @brief  Set/change the unit name of current module.
   *
   * @param unit_name_  Defines the name of the eCAL unit. 
   *
   * @return  Zero if succeeded, -1 if failed.
  **/
  int SetUnitName(const char *unit_name_)
  {
    if (unit_name_ == nullptr) return -1;
    
    const std::string uname = unit_name_;
    if (uname.empty()) return -1;
 
    g_unit_name = uname;
    return 0;
  }

  /**
   * @brief Finalize eCAL API.
   *
   * @return Zero if succeeded, 1 if already finalized, -1 if failed.
  **/
  int Finalize()
  {
    if (g_globals_ctx == nullptr) return 1;
    g_globals_ctx_ref_cnt--;
    if (g_globals_ctx_ref_cnt > 0) return 0;
    int const ret = g_globals()->Finalize();
    delete g_globals_ctx;
    g_globals_ctx = nullptr;
    return(ret);
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
