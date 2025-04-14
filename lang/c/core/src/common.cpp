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
 * @file   common.cpp
 * @brief  C API common helper functions
**/

#include "common.h"


/*
 *
 * Helper functions
 *
 */
inline std::size_t aligned_size(std::size_t size)
{
  return (size + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
}

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

/*
 * Calculation fuctions for extended size
 *
 */

size_t ExtSize_String(const std::string& string_)
{
  return aligned_size(string_.size() + 1);
}
size_t ExtSize_Array(const std::string& array_)
{
  return aligned_size(array_.size());
}

size_t ExtSize_SEntityId(const eCAL::SEntityId& entity_id_)
{
  return ExtSize_String(entity_id_.host_name);
}

size_t ExtSize_STopicId(const eCAL::STopicId& topic_id_)
{
  return ExtSize_String(topic_id_.topic_name) +
    ExtSize_SEntityId(topic_id_.topic_id);
}

size_t ExtSize_SDataTypeInformation(const eCAL::SDataTypeInformation& data_type_information_)
{
  return ExtSize_String(data_type_information_.name) +
    ExtSize_String(data_type_information_.encoding) +
    ExtSize_Array(data_type_information_.descriptor);
}

size_t ExtSize_SServiceId(const eCAL::SServiceId& service_id_)
{
  return ExtSize_SEntityId(service_id_.service_id) +
    ExtSize_String(service_id_.service_name);
}

size_t ExtSize_SServiceMethodInformation(const eCAL::SServiceMethodInformation& method_info_)
{
  return ExtSize_String(method_info_.method_name) +
    ExtSize_SDataTypeInformation(method_info_.request_type) +
    ExtSize_SDataTypeInformation(method_info_.response_type);
}

size_t ExtSize_SServiceResponse(const eCAL::SServiceResponse& service_response_)
{
  return ExtSize_String(service_response_.error_msg) +
    ExtSize_Array(service_response_.response) +
    ExtSize_SServiceId(service_response_.server_id) +
    ExtSize_SServiceMethodInformation(service_response_.service_method_information);
}

/*
 * Conversion functions
 *
 */
char* Convert_String(const std::string& string_, char** offset_)
{
  auto* destination_ptr = *offset_;
  std::strcpy(destination_ptr, string_.c_str());
  *offset_ += ExtSize_String(string_);
  return destination_ptr;
}

void* Convert_Array(const std::string& array_, char** offset_)
{
  auto* destination_ptr = *offset_;
  std::memcpy(destination_ptr, array_.data(), array_.size());
  *offset_ += ExtSize_Array(array_);
  return destination_ptr;
}

/////////////

void Convert_StringSet(char*** string_set_c_, std::size_t* string_set_c_length_, const std::set<std::string>& string_set_)
{
  std::size_t extended_size{ 0 };
  for (const auto& string : string_set_)
    extended_size += ExtSize_String(string);
  const auto base_size{ string_set_.size() * sizeof(char*) };

  *string_set_c_ = reinterpret_cast<char**>(std::malloc(extended_size + base_size));
  if (*string_set_c_ != NULL)
  {
    *string_set_c_length_ = string_set_.size();
    std::size_t i{ 0 };
    auto* string_offset = reinterpret_cast<char*>(*string_set_c_) + base_size;
    for (const auto& string : string_set_)
    {
      (*string_set_c_)[i++] = string_offset;
      const auto aligned_string_size{ ExtSize_String(string) };
      std::strncpy(string_offset, string.c_str(), aligned_string_size);
      string_offset += aligned_string_size;
    }
  }
}

/*
 * Assignment functions
 *
 */

void Assign_SDataTypeInformation(eCAL::SDataTypeInformation& data_type_information_, const struct eCAL_SDataTypeInformation* data_type_information_c_)
{
  data_type_information_.name = data_type_information_c_->name != NULL ? data_type_information_c_->name : "";
  data_type_information_.encoding = data_type_information_c_->encoding != NULL ? data_type_information_c_->encoding : "";
  if (data_type_information_c_->descriptor != NULL)
    data_type_information_.descriptor.assign(reinterpret_cast<const char*>(data_type_information_c_->descriptor), data_type_information_c_->descriptor_length);
  else
    data_type_information_.descriptor.clear();
}

void Assign_SDataTypeInformation(struct eCAL_SDataTypeInformation* data_type_information_c_, const eCAL::SDataTypeInformation& data_type_information_)
{
  data_type_information_c_->name = data_type_information_.name.c_str();
  data_type_information_c_->encoding = data_type_information_.encoding.c_str();
  data_type_information_c_->descriptor = data_type_information_.descriptor.data();
  data_type_information_c_->descriptor_length = data_type_information_.descriptor.length();
}

void Assign_SDataTypeInformation(struct eCAL_SDataTypeInformation* data_type_information_c_, const eCAL::SDataTypeInformation& data_type_information_, char** offset_)
{
  data_type_information_c_->name = Convert_String(data_type_information_.name, offset_);
  data_type_information_c_->encoding = Convert_String(data_type_information_.encoding, offset_);
  data_type_information_c_->descriptor = Convert_Array(data_type_information_.descriptor, offset_);
  data_type_information_c_->descriptor_length = data_type_information_.descriptor.length();
}

/////////////////////

void Assign_SEntityId(struct eCAL_SEntityId* entity_id_c_, const eCAL::SEntityId& entity_id_)
{
  entity_id_c_->entity_id = entity_id_.entity_id;
  entity_id_c_->process_id = entity_id_.process_id;
  entity_id_c_->host_name = entity_id_.host_name.c_str();
}

void Assign_SEntityId(eCAL::SEntityId& entity_id_, const struct eCAL_SEntityId* entity_id_c_)
{
  entity_id_.entity_id = entity_id_c_->entity_id;
  entity_id_.process_id = entity_id_c_->process_id;
  entity_id_.host_name = entity_id_c_->host_name != NULL ? entity_id_c_->host_name : "";
}

void Assign_SEntityId(struct eCAL_SEntityId* entity_id_c_, const eCAL::SEntityId& entity_id_, char** ptr_)
{
  entity_id_c_->entity_id = entity_id_.entity_id;
  entity_id_c_->process_id = entity_id_.process_id;
  entity_id_c_->host_name = Convert_String(entity_id_.host_name.c_str(), ptr_);
}

////////////////////////////////////////

void Assign_STopicId(eCAL::STopicId& topic_id_, const struct eCAL_STopicId* topic_id_c_)
{
  Assign_SEntityId(topic_id_.topic_id, &topic_id_c_->topic_id);
  topic_id_.topic_name = topic_id_c_->topic_name != NULL ? topic_id_c_->topic_name : "";
}

void Assign_STopicId(struct eCAL_STopicId* topic_id_c_, const eCAL::STopicId& topic_id_, char** ptr_)
{
  topic_id_c_->topic_name = Convert_String(topic_id_.topic_name, ptr_);
  Assign_SEntityId(&topic_id_c_->topic_id, topic_id_.topic_id, ptr_);
}

void Assign_STopicId(struct eCAL_STopicId* topic_id_c_, const eCAL::STopicId& topic_id_)
{
  Assign_SEntityId(&topic_id_c_->topic_id, topic_id_.topic_id);
  topic_id_c_->topic_name = topic_id_.topic_name.c_str();
}

/////////////

void Assign_SServiceId(struct eCAL_SServiceId* service_id_c_, const eCAL::SServiceId& service_id_)
{
  Assign_SEntityId(&service_id_c_->service_id, service_id_.service_id);
  service_id_c_->service_name = service_id_.service_name.c_str();
}

void Assign_SServiceId(struct eCAL_SServiceId* service_id_c_, const eCAL::SServiceId& service_id_, char** offset_)
{
  Assign_SEntityId(&service_id_c_->service_id, service_id_.service_id, offset_);
  service_id_c_->service_name = Convert_String(service_id_.service_name, offset_);
}

void Assign_SServiceId(eCAL::SServiceId& service_id_, const struct eCAL_SServiceId* service_id_c_)
{
  Assign_SEntityId(service_id_.service_id, &service_id_c_->service_id);
  service_id_.service_name = service_id_c_->service_name;
}

/////////////

void Assign_SServiceMethodInformation(struct eCAL_SServiceMethodInformation* method_info_c_, const eCAL::SServiceMethodInformation& method_info_, char** offset_)
{
  method_info_c_->method_name = Convert_String(method_info_.method_name, offset_);
  Assign_SDataTypeInformation(&method_info_c_->request_type, method_info_.request_type, offset_);
  Assign_SDataTypeInformation(&method_info_c_->response_type, method_info_.response_type, offset_);
}

void Assign_SServiceMethodInformation(struct eCAL_SServiceMethodInformation* method_info_c_, const eCAL::SServiceMethodInformation& method_info_)
{
  method_info_c_->method_name = method_info_.method_name.c_str();
  Assign_SDataTypeInformation(&method_info_c_->request_type, method_info_.request_type);
  Assign_SDataTypeInformation(&method_info_c_->response_type, method_info_.response_type);
}

void Assign_SServiceMethodInformation(eCAL::SServiceMethodInformation& method_info_, const struct eCAL_SServiceMethodInformation* method_info_c_)
{
  method_info_.method_name = method_info_c_->method_name != NULL ? method_info_c_->method_name : "";
  Assign_SDataTypeInformation(method_info_.request_type, &method_info_c_->request_type);
  Assign_SDataTypeInformation(method_info_.response_type, &method_info_c_->response_type);
}

////////////////////

void Assign_SServiceResponse(struct eCAL_SServiceResponse* service_response_c_, const eCAL::SServiceResponse& service_response_)
{
  static const std::map<eCAL::eCallState, eCAL_eCallState> call_state_map
  {
    {eCAL::eCallState::none, eCAL_eCallState_none},
    {eCAL::eCallState::executed, eCAL_eCallState_executed},
    {eCAL::eCallState::timeouted, eCAL_eCallState_timeouted},
    {eCAL::eCallState::failed, eCAL_eCallState_failed}
  };

  service_response_c_->call_state = call_state_map.at(service_response_.call_state);
  Assign_SServiceId(&service_response_c_->server_id, service_response_.server_id);
  Assign_SServiceMethodInformation(&service_response_c_->service_method_information, service_response_.service_method_information);
  service_response_c_->ret_state = service_response_.ret_state;
  service_response_c_->response = service_response_.response.data();
  service_response_c_->response_length = service_response_.response.size();
  service_response_c_->error_msg = service_response_.error_msg.c_str();
}

void Assign_SServiceResponse(struct eCAL_SServiceResponse* service_response_c_, const eCAL::SServiceResponse& service_response_, char** offset_)
{
  static const std::map<eCAL::eCallState, eCAL_eCallState> call_state_map
  {
    {eCAL::eCallState::none, eCAL_eCallState_none},
    {eCAL::eCallState::executed, eCAL_eCallState_executed},
    {eCAL::eCallState::timeouted, eCAL_eCallState_timeouted},
    {eCAL::eCallState::failed, eCAL_eCallState_failed}
  };

  service_response_c_->call_state = call_state_map.at(service_response_.call_state);
  Assign_SServiceId(&service_response_c_->server_id, service_response_.server_id, offset_);
  Assign_SServiceMethodInformation(&service_response_c_->service_method_information, service_response_.service_method_information, offset_);
  service_response_c_->ret_state = service_response_.ret_state;
  service_response_c_->response = Convert_Array(service_response_.response, offset_);
  service_response_c_->error_msg = Convert_String(service_response_.error_msg, offset_);
}