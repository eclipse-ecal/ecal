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

#include <stdint.h>

/**
 * @brief Optional compile time information associated with a given topic
 *        (necessary for reflection / runtime type checking)
**/
struct eCAL_SDataTypeInformation
{
  const char* name;         //!< name of the datatype
  const char* encoding;     //!< encoding of the datatype (e.g. protobuf, flatbuffers, capnproto)
  const void* descriptor;   //!< descriptor information of the datatype (necessary for reflection)
  size_t descriptor_length; //!< length of descriptor information
};

typedef uint64_t eCAL_EntityIdT;

/**
 * @brief Combined meta infomration for an eCAL entity such as publisher, subscriber, etc.
**/
struct eCAL_SEntityId
{
  eCAL_EntityIdT entity_id; //!< unique id within that process (it should already be unique within the whole system)
  int32_t process_id;       //!< process id which produced the sample
  const char* host_name;    //!< host which produced the sample
};

/**
 * @brief eCAL version struct (C variant)
 **/
struct eCAL_SVersion
{
  int major; //!< major version number
  int minor; //!< minor version number
  int patch; //!< patch version number
};

#endif /* ecal_c_types_h_included */
