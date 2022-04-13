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
    CDescGate();
    ~CDescGate();

    void Create();
    void Destroy();

    void ApplyTopicDescription(const std::string& topic_name_, 
                               const std::string& topic_type_,
                               const std::string& topic_desc_);

    bool GetTopicTypeName(const std::string& topic_name_, std::string& topic_type_);
    bool GetTopicDescription(const std::string& topic_name_, std::string& topic_desc_);

    void ApplyServiceDescription(const std::string& service_name_, 
                                 const std::string& method_name_, 
                                 const std::string& req_type_name_, 
                                 const std::string& req_type_desc_, 
                                 const std::string& resp_type_name_,
                                 const std::string& resp_type_desc_);

    bool GetServiceTypeNames(const std::string& service_name_, const std::string& method_name_, std::string& req_type_name_, std::string& resp_type_name_);
    bool GetServiceDescription(const std::string& service_name_, const std::string& method_name_, std::string& req_type_desc_, std::string& resp_type_desc_);

  protected:
    struct STypeDesc
    {
      STypeDesc() : match_fail(false) {};
      std::string type;
      std::string desc;
      bool        match_fail;

      void set_type(const std::string& topic_type_)
      {
        if (topic_type_.empty()) return;
        type = topic_type_;
      }

      void set_desc(const std::string& topic_desc_)
      {
        if (topic_desc_.empty()) return;
        desc = topic_desc_;
      }
    };

    // key: topic name | value: topic(type/desc)
    typedef std::map<std::string, STypeDesc> TopicNameDescMapT;
    std::shared_timed_mutex  m_topic_name_desc_sync;
    TopicNameDescMapT        m_topic_name_desc_map;

    // key: tup<service name, method name> | value: tup<request (type/desc), response (type/desc)>
    typedef std::map<std::tuple<std::string, std::string>, std::tuple<STypeDesc, STypeDesc>> ServiceMethodDescMapT;
    std::shared_timed_mutex  m_service_method_desc_sync;
    ServiceMethodDescMapT    m_service_method_desc_map;
  };
};
