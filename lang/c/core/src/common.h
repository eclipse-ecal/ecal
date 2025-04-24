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
 * @file   common.h
 * @brief  C API common helper functions
**/

#pragma once

#include <string>
#include <cstring>
#include <cstdlib>

#include <ecal/ecal.h>
#include <ecal_c/types.h>
#include <ecal_c/pubsub/types.h>
#include <ecal_c/service/types.h>

/*
 *
 * Helper functions
 *
 */
std::size_t aligned_size(std::size_t size);
char* Clone_CString(const char* c_string_);

/*
 * Calculation fuctions for extended size
 *
 */

size_t ExtSize_String(const std::string& string_);
size_t ExtSize_Array(const std::string& array_);
size_t ExtSize_SEntityId(const eCAL::SEntityId& entity_id_);
size_t ExtSize_STopicId(const eCAL::STopicId& topic_id_);
size_t ExtSize_SDataTypeInformation(const eCAL::SDataTypeInformation& data_type_information_);
size_t ExtSize_SServiceId(const eCAL::SServiceId& service_id_);
size_t ExtSize_SServiceMethodInformation(const eCAL::SServiceMethodInformation& method_info_);
size_t ExtSize_SServiceResponse(const eCAL::SServiceResponse& service_response_);

/*
 * Conversion functions
 *
 */
char* Convert_String(const std::string& string_, char** offset_);
void* Convert_Array(const std::string& array_, char** offset_);

/////////////

void Convert_StringSet(char*** string_set_c_, std::size_t* string_set_c_length_, const std::set<std::string>& string_set_);

/*
 * Assignment functions
 *
 */

void Assign_SDataTypeInformation(eCAL::SDataTypeInformation& data_type_information_, const struct eCAL_SDataTypeInformation* data_type_information_c_);
void Assign_SDataTypeInformation(struct eCAL_SDataTypeInformation* data_type_information_c_, const eCAL::SDataTypeInformation& data_type_information_);
void Assign_SDataTypeInformation(struct eCAL_SDataTypeInformation* data_type_information_c_, const eCAL::SDataTypeInformation& data_type_information_, char** offset_);

/////////////////////

void Assign_SEntityId(struct eCAL_SEntityId* entity_id_c_, const eCAL::SEntityId& entity_id_);
void Assign_SEntityId(eCAL::SEntityId& entity_id_, const struct eCAL_SEntityId* entity_id_c_);
void Assign_SEntityId(struct eCAL_SEntityId* entity_id_c_, const eCAL::SEntityId& entity_id_, char** ptr_);

////////////////////////////////////////

void Assign_STopicId(eCAL::STopicId& topic_id_, const struct eCAL_STopicId* topic_id_c_);
void Assign_STopicId(struct eCAL_STopicId* topic_id_c_, const eCAL::STopicId& topic_id_, char** ptr_);
void Assign_STopicId(struct eCAL_STopicId* topic_id_c_, const eCAL::STopicId& topic_id_);

/////////////

void Assign_SServiceId(struct eCAL_SServiceId* service_id_c_, const eCAL::SServiceId& service_id_);
void Assign_SServiceId(struct eCAL_SServiceId* service_id_c_, const eCAL::SServiceId& service_id_, char** offset_);
void Assign_SServiceId(eCAL::SServiceId& service_id_, const struct eCAL_SServiceId* service_id_c_);

/////////////

void Assign_SServiceMethodInformation(struct eCAL_SServiceMethodInformation* method_info_c_, const eCAL::SServiceMethodInformation& method_info_, char** offset_);
void Assign_SServiceMethodInformation(struct eCAL_SServiceMethodInformation* method_info_c_, const eCAL::SServiceMethodInformation& method_info_);
void Assign_SServiceMethodInformation(eCAL::SServiceMethodInformation& method_info_, const struct eCAL_SServiceMethodInformation* method_info_c_);

////////////////////

void Assign_SServiceResponse(struct eCAL_SServiceResponse* service_response_c_, const eCAL::SServiceResponse& service_response_);
void Assign_SServiceResponse(struct eCAL_SServiceResponse* service_response_c_, const eCAL::SServiceResponse& service_response_, char** offset_);
