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
 * @file   ecal_eventhandle.h
 * @brief  eCAL event handle
**/

#pragma once

#ifdef _MSC_VER
#pragma message("WARNING: This header file is deprecated. It will be removed in future eCAL versions.")
#endif /*_MSC_VER*/
#ifdef __GNUC__
#pragma message "WARNING: This header file is deprecated. It will be removed in future eCAL versions."
#endif /* __GNUC__ */

#include <ecal/ecal_deprecate.h>

#include <string>
#include <vector>

namespace eCAL
{
  /**
   * @brief eCAL event callback handle.
   * @deprecated Will be removed in future eCAL versions.
  **/
  struct SEventHandle
  {
    /**
     * @brief Event callback handle constructor.
    **/
    SEventHandle() : name(), handle(nullptr)
    {
    };
    std::string name;    //!< event name
    void*       handle;  //!< event handle

    /* @cond */
    bool operator==(const SEventHandle& rhs)
    {
      return(rhs.name == name && rhs.handle == handle);
    }
    /* @endcond */
  };

  /* @cond */
  typedef SEventHandle              EventHandleT;
  typedef std::vector<EventHandleT> EventHandleVecT;
  /* @endcond */
}
