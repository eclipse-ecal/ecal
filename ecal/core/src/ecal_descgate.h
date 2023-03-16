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

#include <ecal/ecal_util.h>

#include "ecal_global_accessors.h"
#include "ecal_def.h"
#include "ecal_expmap.h"

#include <shared_mutex>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <tuple>
#include <vector>


namespace eCAL
{
  class CDescGate
  {
  public:
    // Enumeration of quality bits used for detecting how good a topic information is.
    enum class QualityFlags : int
    {
      NO_QUALITY                     = 0,         //!< Special value for initialization

      DESCRIPTION_AVAILABLE          = 0x1 << 4,  //!< Having a descriptor at all is the most important thing
      INFO_COMES_FROM_CORRECT_ENTITY = 0x1 << 3,  //!< The information comes from the current topic/service 
                                                  //!< and has not been borrowed from another emtity, like read by a subscriber from a publisher
      INFO_COMES_FROM_PRODUCER       = 0x1 << 2,  //!< A descriptor coming from the producer (like a publisher) is better than one from a 
                                                  //!< consumer (like a subscriber), as we assume that the publisher knows best what he is publishing
      INFO_COMES_FROM_THIS_PROCESS   = 0x1 << 1,  //!< We prefer descriptors from the current process
      TYPE_AVAILABLE                 = 0x1 << 0,  //!< Having information about the type's name available is nice but not that important to us
    };

  public:
    CDescGate();
    ~CDescGate();

    void Create();
    void Destroy();

    bool ApplyTopicDescription(const std::string& topic_name_, 
                               const std::string& topic_type_,
                               const std::string& topic_desc_,
                               const QualityFlags description_quality_);

    void GetTopics(std::unordered_map<std::string, Util::STopicInfo>& topic_info_map_);
    void GetTopicNames(std::vector<std::string>& topic_names_);
    bool GetTopicTypeName(const std::string& topic_name_, std::string& topic_type_);
    bool GetTopicDescription(const std::string& topic_name_, std::string& topic_desc_);

    bool ApplyServiceDescription(const std::string& service_name_, 
                                 const std::string& method_name_, 
                                 const std::string& req_type_name_, 
                                 const std::string& req_type_desc_, 
                                 const std::string& resp_type_name_,
                                 const std::string& resp_type_desc_,
                                 const QualityFlags description_quality_);

    void GetServices(std::map<std::tuple<std::string, std::string>, Util::SServiceMethodInfo>& service_info_map_);
    void GetServiceNames(std::vector<std::tuple<std::string, std::string>>& service_method_names_);
    bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_name_, std::string& resp_type_name_);
    bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_type_desc_, std::string& resp_type_desc_);

  protected:
    struct STopicInfoQuality
    {
      Util::STopicInfo info;                                                       //!< Topic info struct with type name and descriptor.
      QualityFlags     quality               = QualityFlags::NO_QUALITY;           //!< QualityFlags to determine whether we may overwrite the current data with better one. E.g. we prefer the description sent by a publisher over one sent by a subscriber. 
      bool             type_missmatch_logged = false;                              //!< Whether we have already logged a type-missmatch
    };

    struct SServiceMethodInfoQuality
    {
      Util::SServiceMethodInfo info;                                               //!< Service info struct with type names and descriptors for request and response.
      QualityFlags             quality = QualityFlags::NO_QUALITY;                 //!< The Quality of the Info
    };

    // key: topic name | value: topic (type/desc), quality
    using TopicInfoMap = eCAL::Util::CExpMap<std::string, STopicInfoQuality>;      //!< Map containing { TopicName -> (Type, Description, Quality) } mapping of all topics that are currently known
    struct STopicInfoMap
    {
      explicit STopicInfoMap(const std::chrono::milliseconds& timeout_) :
        map(new TopicInfoMap(timeout_))
      {
      };
      mutable std::shared_timed_mutex sync;                                        //!< Mutex protecting the map
      std::unique_ptr<TopicInfoMap>   map;                                         //!< Map containing information about each known topic
    };
    STopicInfoMap m_topic_info_map;

    // key: tup<service name, method name> | value: request (type/desc), response (type/desc), quality
    using ServiceMethodInfoMap = eCAL::Util::CExpMap<std::tuple<std::string, std::string>, SServiceMethodInfoQuality>; //!< Map { (ServiceName, MethodName) -> ( (ReqType, ReqDescription), (RespType, RespDescription), Quality ) } mapping of all currently known services
    struct SServiceMethodInfoMap
    {
      explicit SServiceMethodInfoMap(const std::chrono::milliseconds& timeout_) :
        map(new ServiceMethodInfoMap(timeout_))
      {
      };
      mutable std::shared_timed_mutex       sync;                                  //!< Mutex protecting the map
      std::unique_ptr<ServiceMethodInfoMap> map;                                   //!< Map containing information about each known service
    };
    SServiceMethodInfoMap m_service_info_map;
  };

  constexpr inline CDescGate::QualityFlags  operator~  (CDescGate::QualityFlags  a)                            { return static_cast<CDescGate::QualityFlags>( ~static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(a) ); }
  constexpr inline CDescGate::QualityFlags  operator|  (CDescGate::QualityFlags  a, CDescGate::QualityFlags b) { return static_cast<CDescGate::QualityFlags>( static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(a) | static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
  constexpr inline CDescGate::QualityFlags  operator&  (CDescGate::QualityFlags  a, CDescGate::QualityFlags b) { return static_cast<CDescGate::QualityFlags>( static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(a) & static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
  constexpr inline CDescGate::QualityFlags  operator^  (CDescGate::QualityFlags  a, CDescGate::QualityFlags b) { return static_cast<CDescGate::QualityFlags>( static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(a) ^ static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
  inline           CDescGate::QualityFlags& operator|= (CDescGate::QualityFlags& a, CDescGate::QualityFlags b) { return reinterpret_cast<CDescGate::QualityFlags&>( reinterpret_cast<std::underlying_type<CDescGate::QualityFlags>::type&>(a) |= static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
  inline           CDescGate::QualityFlags& operator&= (CDescGate::QualityFlags& a, CDescGate::QualityFlags b) { return reinterpret_cast<CDescGate::QualityFlags&>( reinterpret_cast<std::underlying_type<CDescGate::QualityFlags>::type&>(a) &= static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
  inline           CDescGate::QualityFlags& operator^= (CDescGate::QualityFlags& a, CDescGate::QualityFlags b) { return reinterpret_cast<CDescGate::QualityFlags&>( reinterpret_cast<std::underlying_type<CDescGate::QualityFlags>::type&>(a) ^= static_cast<std::underlying_type<CDescGate::QualityFlags>::type>(b) ); }
};
