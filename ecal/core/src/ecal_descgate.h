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
 * @brief  eCAL description gateway class
**/

#pragma once

#include "ecal_global_accessors.h"
#include "ecal_def.h"

#include <shared_mutex>
#include <string>
#include <map>
#include <memory>
#include <tuple>


namespace eCAL
{
  class CDescGate
  {
  public:
    // Enumeration of qulity bits used for detecting how good a topic information is.
    enum class QualityFlags : int
    {
      NO_QUALITY                    = 0,         //!< Special value for initialization

      DESCRIPTION_AVAILABLE         = 0x1 << 4,  //!< Having a descriptor at all is the most important thing
      INFO_COMES_FROM_CORRECT_TOPIC = 0x1 << 3,  //!< The information comes from the current topic (and has not been borrowed from another topic)
      INFO_COMES_FROM_PUBLISHER     = 0x1 << 2,  //!< A descriptor coming from the publisher is better than one from a subsriber, as we assume that the publisher knows best what he is publishing
      INFO_COMES_FROM_THIS_PROCESS  = 0x1 << 1,  //!< We prefer descriptors from the current process
      TYPE_AVAILABLE                = 0x1 << 0,  //!< Having information about the type's name available is nice but not that important to us.
    };

  public:
    CDescGate();
    ~CDescGate();

    void Create();
    void Destroy();

    void ApplyTopicDescription(const std::string& topic_name_, 
                               const std::string& topic_type_,
                               const std::string& topic_desc_,
                               const QualityFlags description_quality_);

    bool GetTopicTypeName(const std::string& topic_name_, std::string& topic_type_);
    bool GetTopicDescription(const std::string& topic_name_, std::string& topic_desc_);

    void ApplyServiceDescription(const std::string& service_name_, 
                                 const std::string& method_name_, 
                                 const std::string& req_type_name_, 
                                 const std::string& req_type_desc_, 
                                 const std::string& resp_type_name_,
                                 const std::string& resp_type_desc_,
                                 const QualityFlags info_quality_);

    bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_name_, std::string& resp_type_name_);
    bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_type_desc_, std::string& resp_type_desc_);


  protected:
    struct STopicInfo
    {
      std::string type_name;                                         //!< Type name of the current topic
      std::string type_description;                                  //!< Descriptor String of the current topic. Used e.g. for dynamic deserialization

      QualityFlags description_quality   = QualityFlags::NO_QUALITY; //!< QualityFlags to determine whether we may overwrite the current data with better one. E.g. we prefer the description sent by a publisher over one sent by a subscriber. 
      bool        type_missmatch_logged = false;                     //!< Whether we have already logged a type-missmatch
    };

    struct SServiceMethodInfo
    {
      std::string request_type_name;                                 //!< Type name of the request message
      std::string request_type_description;                          //!< Descriptor String of the request description
      std::string response_type_name;                                //!< Type name of the response message
      std::string response_type_description;                         //!< Descriptor String of the response message

      QualityFlags info_quality = QualityFlags::NO_QUALITY;          //!< The Quality of the Info
    };

    // key: topic name | value: topic(type/desc)
    using TopicInfoMap = std::map<std::string, STopicInfo>;                 //!< Map containing { TopicName -> (Type, Description) } mapping of all topics that are currently known
    mutable std::shared_timed_mutex  m_topic_info_map_mutex;                //!< Mutex protecting the m_topic_info_map
    TopicInfoMap                     m_topic_info_map;                      //!< Map containing information about each known topic

    // key: tup<service name, method name> | value: tup<request (type/desc), response (type/desc)>
    using ServiceMethodInfoMap 
      = std::map<std::tuple<std::string, std::string>, SServiceMethodInfo>; //! Map { (ServiceName, MethodName) -> ( (ReqType, ReqDescription), (RespType, RespDescription) ) } mapping of all currently known services
    mutable std::shared_timed_mutex  m_service_info_map_mutex;              //!< Mutex protecting the m_service_info_map
    ServiceMethodInfoMap             m_service_info_map;                    //!< Map containing information about each known service method
  };

  constexpr inline CDescGate::QualityFlags  operator~  (CDescGate::QualityFlags  a)                            { return static_cast<CDescGate::QualityFlags>( ~static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(a) ); }
  constexpr inline CDescGate::QualityFlags  operator|  (CDescGate::QualityFlags  a, CDescGate::QualityFlags b) { return static_cast<CDescGate::QualityFlags>( static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(a) | static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
  constexpr inline CDescGate::QualityFlags  operator&  (CDescGate::QualityFlags  a, CDescGate::QualityFlags b) { return static_cast<CDescGate::QualityFlags>( static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(a) & static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
  constexpr inline CDescGate::QualityFlags  operator^  (CDescGate::QualityFlags  a, CDescGate::QualityFlags b) { return static_cast<CDescGate::QualityFlags>( static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(a) ^ static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
  inline           CDescGate::QualityFlags& operator|= (CDescGate::QualityFlags& a, CDescGate::QualityFlags b) { return reinterpret_cast<CDescGate::QualityFlags&>( reinterpret_cast<std::underlying_type<CDescGate::QualityFlags>::type&>(a) |= static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
  inline           CDescGate::QualityFlags& operator&= (CDescGate::QualityFlags& a, CDescGate::QualityFlags b) { return reinterpret_cast<CDescGate::QualityFlags&>( reinterpret_cast<std::underlying_type<CDescGate::QualityFlags>::type&>(a) &= static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
  inline           CDescGate::QualityFlags& operator^= (CDescGate::QualityFlags& a, CDescGate::QualityFlags b) { return reinterpret_cast<CDescGate::QualityFlags&>( reinterpret_cast<std::underlying_type<CDescGate::QualityFlags>::type&>(a) ^= static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
};
