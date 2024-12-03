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
 * @file   eh5_types.h
 * @brief  eCALHDF5 types defines
**/

#pragma once

#include <set>
#include <string>
#include <vector>

#include <ecal/measurement/base/types.h>

namespace eCAL
{
  namespace eh5
  {
    //!< @cond
    const std::string kChnNameAttribTitle ("Channel Name");
    const std::string kChnDescAttrTitle   ("Channel Description");
    const std::string kChnTypeAttrTitle   ("Channel Type");
    const std::string kChnIdTypename      ("TypeName");
    const std::string kChnIdEncoding      ("TypeEncoding");
    const std::string kChnIdDescriptor    ("TypeDescriptor");
    const std::string kChnIdData          ("DataTable");
    const std::string kFileVerAttrTitle   ("Version");
    const std::string kTimestampAttrTitle ("Timestamps");
    const std::string kChnAttrTitle       ("Channels");

    // Remove @eCAL6 -> backwards compatibility with old interface!
    using SChannel = eCAL::experimental::measurement::base::Channel;
    using SEntryInfo = eCAL::experimental::measurement::base::EntryInfo;
    using EntryInfoSet = eCAL::experimental::measurement::base::EntryInfoSet;
    using EntryInfoVect = eCAL::experimental::measurement::base::EntryInfoVect;
    using SWriteEntry = eCAL::experimental::measurement::base::WriteEntry;

    namespace v2
    {
      enum eAccessType
      {
        RDONLY,    //!< ReadOnly - the measurement can only be read
        CREATE,    //!< Create   - a new measurement will be created
      };
    }

    inline namespace v3
    {
      enum class eAccessType
      {
        RDONLY,    //!< ReadOnly - the measurement can only be read
        CREATE,    //!< Create   - a new measurement will be created
        CREATE_V5  //!< Create a legacy V5 hdf5 measurement (For testing purpose only!)
      };
    }
  
    using eCAL::experimental::measurement::base::DataTypeInformation;
    //!< @endcond
  }  // namespace eh5
}  // namespace eCAL
