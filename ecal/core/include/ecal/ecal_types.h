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
 * @file   ecal_types.h
 * @brief  This file contains type definitions for information associated with a given topic
**/

#pragma once
#include <string>

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
      //!< @endcond
    };

    /**
     * @brief Optional compile time information associated with a given topic
     *        (necessary for reflection / runtime type checking)
    **/
    struct STopicInformation
    {
      SDataTypeInformation topic_type; //!< Data type description of the topic

      //!< @cond
      bool operator==(const STopicInformation& other) const
      {
        return topic_type == other.topic_type;
      }

      bool operator!=(const STopicInformation& other) const
      {
        return !(*this == other);
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
      //!< @endcond
    };

}
