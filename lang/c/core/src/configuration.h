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

#pragma once

#include <ecal/ecal.h>

#include <ecal_c/config/configuration.h>

#include <map>

/*
 *  Assigment and conversion functions
 *  from C++ to C
 * 
 */

enum eCAL_TransportLayer_eType Convert_TransportLayer_eType(eCAL::TransportLayer::eType type_);
enum eCAL_Registration_Local_eTransportType Convert_Registration_Local_eTransportType(eCAL::Registration::Local::eTransportType transport_type_);
enum eCAL_Registration_Network_eTransportType Convert_Registration_Network_eTransportType(eCAL::Registration::Network::eTransportType transport_type_);
enum eCAL_Types_UdpConfigVersion Convert_Types_UdpConfigVersion(eCAL::Types::UdpConfigVersion udp_config_version_);
enum eCAL_eCommunicationMode Convert_eCommunicationMode(eCAL::eCommunicationMode communication_mode_);
eCAL_Logging_Filter Convert_Logging_Filter(eCAL::Logging::Filter filter_);

void Assign_Application_Configuration(struct eCAL_Application_Configuration* configuration_c_, const eCAL::Application::Configuration& configuration_);
void Assign_Logging_Configuration(struct eCAL_Logging_Configuration* configuration_c_, const eCAL::Logging::Configuration& configuration_);
void Assign_Publisher_Configuration(struct eCAL_Publisher_Configuration* configuration_c_, const eCAL::Publisher::Configuration& configuration_);
void Assign_Registration_Configuration(struct eCAL_Registration_Configuration* configuration_c_, const eCAL::Registration::Configuration& configuration_);
void Assign_Subscriber_Configuration(struct eCAL_Subscriber_Configuration* configuration_c_, const eCAL::Subscriber::Configuration& configuration_);
void Assign_Time_Configuration(struct eCAL_Time_Configuration* configuration_c_, const eCAL::Time::Configuration& configuration_);
void Assign_TransportLayer_Configuration(struct eCAL_TransportLayer_Configuration* configuration_c_, const eCAL::TransportLayer::Configuration& configuration_);
void Assign_Configuration(eCAL_Configuration* configuration_c_, const eCAL::Configuration& configuration_);

/*
 *  Assigment and conversion functions
 *  from C to C++
 *
 */

eCAL::TransportLayer::eType Convert_TransportLayer_eType(enum eCAL_TransportLayer_eType type_);
eCAL::Registration::Local::eTransportType Convert_Registration_Local_eTransportType(enum eCAL_Registration_Local_eTransportType transport_type_);
eCAL::Registration::Network::eTransportType Convert_Registration_Network_eTransportType(enum eCAL_Registration_Network_eTransportType transport_type_);
eCAL::Types::UdpConfigVersion Convert_Types_UdpConfigVersion(enum eCAL_Types_UdpConfigVersion udp_config_version_);
eCAL::eCommunicationMode Convert_eCommunicationMode(enum eCAL_eCommunicationMode communication_mode_);
eCAL::Logging::Filter Convert_Logging_FilterC(eCAL_Logging_Filter filter_c_);

void Assign_Application_Configuration(eCAL::Application::Configuration& configuration_, const struct eCAL_Application_Configuration* configuration_c_);
void Assign_Logging_Configuration(eCAL::Logging::Configuration& configuration_, const struct eCAL_Logging_Configuration* configuration_c_);
void Assign_Publisher_Configuration(eCAL::Publisher::Configuration& configuration_, const struct eCAL_Publisher_Configuration* configuration_c_);
void Assign_Registration_Configuration(eCAL::Registration::Configuration& configuration_, const struct eCAL_Registration_Configuration* configuration_c_);
void Assign_Subscriber_Configuration(eCAL::Subscriber::Configuration& configuration_, const struct eCAL_Subscriber_Configuration* configuration_c_);
void Assign_Time_Configuration(eCAL::Time::Configuration& configuration_, const struct eCAL_Time_Configuration* configuration_c_);
void Assign_TransportLayer_Configuration(eCAL::TransportLayer::Configuration& configuration_, const struct eCAL_TransportLayer_Configuration* configuration_c_);
void Assign_Configuration(eCAL::Configuration& configuration_, const eCAL_Configuration* configuration_c_);
