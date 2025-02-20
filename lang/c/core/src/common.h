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
 * @file   common.h
 * @brief  C API common helper functions
**/

#pragma once

#include <string>
#include <cstring>
#include <cstdlib>

int CopyBuffer(void* target_, int target_len_, const std::string& source_s_);

namespace
{
  char* Clone_CString(const char* c_string_)
  {
    char* cloned_c_string = NULL;
    if (c_string_ != NULL)
    {
      const auto c_string_len = std::strlen(c_string_);
      cloned_c_string = reinterpret_cast<char*>(std::malloc(c_string_len + 1));
      if (cloned_c_string != NULL)
        std::strcpy(cloned_c_string, c_string_);
    }
    return cloned_c_string;
  }

  void* Clone_CArray(const void* c_array_, size_t c_array_len_)
  {
    void* cloned_c_array = NULL;
    if (c_array_ != NULL)
    {
      cloned_c_array = std::malloc(c_array_len_);
      if (cloned_c_array != NULL)
        std::memcpy(cloned_c_array, c_array_, c_array_len_);
    }
    return cloned_c_array;
  }

  void Convert_SDataTypeInformation(eCAL::SDataTypeInformation& data_type_information_, const struct eCAL_SDataTypeInformation* data_type_information_c_)
  {
    data_type_information_.name = data_type_information_c_->name;
    data_type_information_.encoding = data_type_information_c_->encoding;
    data_type_information_.descriptor.assign(reinterpret_cast<const char*>(data_type_information_c_->descriptor), data_type_information_c_->descriptor_len);
  }

  void Convert_SDataTypeInformation(struct eCAL_SDataTypeInformation* data_type_information_c_, const eCAL::SDataTypeInformation& data_type_information_)
  {
    data_type_information_c_->name = Clone_CString(data_type_information_.name.c_str());
    data_type_information_c_->encoding = Clone_CString(data_type_information_.encoding.c_str());
    data_type_information_c_->descriptor = Clone_CArray(data_type_information_.descriptor.data(), data_type_information_.descriptor.length());
    data_type_information_c_->descriptor_len = data_type_information_.descriptor.length();
  }

  void Assign_SDataTypeInformation(struct eCAL_SDataTypeInformation* data_type_information_c_, const eCAL::SDataTypeInformation& data_type_information_)
  {
    data_type_information_c_->name = data_type_information_.name.c_str();
    data_type_information_c_->encoding = data_type_information_.encoding.c_str();
    data_type_information_c_->descriptor = data_type_information_.descriptor.data();
    data_type_information_c_->descriptor_len = data_type_information_.descriptor.length();
  }

  void Free_SDataTypeInformation(struct eCAL_SDataTypeInformation* data_type_information_)
  {
    std::free(const_cast<void*>(reinterpret_cast<const void*>(data_type_information_->name)));
    std::free(const_cast<void*>(reinterpret_cast<const void*>(data_type_information_->encoding)));
    std::free(const_cast<void*>(reinterpret_cast<const void*>(data_type_information_->descriptor)));
  }

  void Convert_STopicId(struct eCAL_STopicId* topic_id_c_, const eCAL::STopicId& topic_id_)
  {
    topic_id_c_->topic_id.entity_id = topic_id_.topic_id.entity_id;
    topic_id_c_->topic_id.process_id = topic_id_.topic_id.process_id;
    topic_id_c_->topic_id.host_name = Clone_CString(topic_id_.topic_id.host_name.c_str());
    topic_id_c_->topic_name = Clone_CString(topic_id_.topic_name.c_str());
  }

  void Assign_STopicId(struct eCAL_STopicId* topic_id_c_, const eCAL::STopicId& topic_id_)
  {
    topic_id_c_->topic_id.entity_id = topic_id_.topic_id.entity_id;
    topic_id_c_->topic_id.process_id = topic_id_.topic_id.process_id;
    topic_id_c_->topic_id.host_name = topic_id_.topic_id.host_name.c_str();
    topic_id_c_->topic_name = topic_id_.topic_name.c_str();
  }

  void Free_STopicId(struct eCAL_STopicId* topic_id_)
  {
    std::free(const_cast<void*>(reinterpret_cast<const void*>(topic_id_->topic_id.host_name)));
    std::free(const_cast<void*>(reinterpret_cast<const void*>(topic_id_->topic_name)));
  }
}