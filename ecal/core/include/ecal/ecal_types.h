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
 * @file   ecal_types.h
 * @brief  This file contains type definitions for information associated with a given topic
**/

#pragma once
#include <string>
#include <tuple>
#include <iostream>

namespace eCAL
{
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

  /**
   * @brief Optional compile time information associated with a given service method
   *        (necessary for reflection / runtime type checking)
  **/
  struct SServiceMethodInformation
  {
    SDataTypeInformation request_type;   //!< Data type description of the request
    SDataTypeInformation response_type;  //!< Data type description of the response

    //!< @cond
    bool operator==(const SServiceMethodInformation& other) const
    {
      return request_type == other.request_type && response_type == other.response_type;
    }

    bool operator!=(const SServiceMethodInformation& other) const
    {
      return !(*this == other);
    }

    bool operator<(const SServiceMethodInformation& rhs) const
    {
      return std::tie(request_type, response_type) < std::tie(rhs.request_type, rhs.response_type);
    }
    //!< @endcond
  };

  namespace Registration
  {
    struct SEntityId
    {
      std::string  entity_id;         // unique id within that process (it should already be unique within the whole system)
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

    struct STopicId
    {
      SEntityId    topic_id;
      std::string  topic_name;

      bool operator==(const STopicId& other) const
      {
        return topic_id == other.topic_id && topic_name == other.topic_name;
      }

      bool operator<(const STopicId& other) const
      {
        return std::tie(topic_id, topic_name) < std::tie(other.topic_id, other.topic_name);
      }
    };

    inline std::ostream& operator<<(std::ostream& os, const STopicId& id)
    {
      os << "STopicId(topic_id: " << id.topic_id
        << ", topic_name: " << id.topic_name << ")";
      return os;
    }

    struct SServiceId
    {
      SEntityId    service_id;
      std::string  service_name;
      std::string  method_name;

      bool operator==(const SServiceId& other) const
      {
        return service_id == other.service_id && service_name == other.service_name && method_name == other.method_name;
      }

      bool operator<(const SServiceId& other) const
      {
        return std::tie(service_id, service_name, method_name) < std::tie(other.service_id, other.service_name, other.method_name);
      }
    };
  }
}
