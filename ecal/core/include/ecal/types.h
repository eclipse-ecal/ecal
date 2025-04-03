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
 * @file   types.h
 * @brief  This file contains type definitions for information associated with a given topic
**/

#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>

namespace eCAL
{
  /**
   * @brief eCAL version struct
  **/
  struct SVersion
  {
    const int major; //!< major version number
    const int minor; //!< minor version number
    const int patch; //!< patch version number
  };

  /**
   * @brief Optional compile time information associated with a given topic
   *        (necessary for reflection / runtime type checking)
  **/
  struct SDataTypeInformation
  {
    std::string name;          //!< name of the datatype
    std::string encoding;      //!< encoding of the datatype (e.g. protobuf, flatbuffers, capnproto)
    std::string descriptor;    //!< descriptor information of the datatype (necessary for reflection)

    //!< @cond
    bool operator==(const SDataTypeInformation& other) const
    {
      return name == other.name && encoding == other.encoding && descriptor == other.descriptor;
    }

    bool operator!=(const SDataTypeInformation& other) const
    {
      return !(*this == other);
    }

    bool operator<(const SDataTypeInformation& rhs) const
    {
        return std::tie(name, encoding, descriptor) < std::tie(rhs.name, rhs.encoding, rhs.descriptor);
    }

    void clear()
    {
      name.clear();
      encoding.clear();
      descriptor.clear();
    }
    //!< @endcond
  };

  using EntityIdT = uint64_t;

  struct SEntityId
  {
    EntityIdT    entity_id  = 0;    // unique id within that process (it should already be unique within the whole system)
    int32_t      process_id = 0;    // process id which produced the sample
    std::string  host_name;         // host which produced the sample

    bool operator==(const SEntityId& other) const {
      return entity_id == other.entity_id;
    }

    bool operator<(const SEntityId& other) const
    {
      return entity_id < other.entity_id;
    }
  };

  // Overload the << operator for SEntityId
  inline std::ostream& operator<<(std::ostream& os, const SEntityId& id)
  {
    os << "SEntityId(entity_id: " << id.entity_id
      << ", process_id: " << id.process_id
      << ", host_name: " << id.host_name << ")";
    return os;
  }
}
