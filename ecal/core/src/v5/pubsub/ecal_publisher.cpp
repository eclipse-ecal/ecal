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
 * @brief  eCAL publisher interface (deprecated eCAL5 version)
**/

#include <ecal/ecal.h>
#include <ecal/v5/ecal_publisher.h>

#include "ecal_globals.h"
#include <pubsub/ecal_publisher_impl.h>
#include "ecal_config_internal.h"

#include <pubsub/config/builder/writer_attribute_builder.h>
#include "readwrite/ecal_writer_buffer_payload.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>


namespace eCAL
{
  ECAL_CORE_NAMESPACE_V5
  {
    // Class to bridge legacy callbacks to v6 Callbacks
    class CEventCallbackAdapater
    {
    public:
      CEventCallbackAdapater(std::shared_ptr<CPublisherImpl> publisher_impl_)
        : m_publisher_impl(publisher_impl_)
        , m_is_registered(false)
      {
      }

      bool AddEventCallback(ePublisherEvent type_, const PubEventCallbackT& callback_)
      {
        const std::lock_guard<std::mutex> guard(m_event_callback_map_mutex);
        m_event_callback_map[type_] = callback_;
      
        if (!m_is_registered)
        {
          m_is_registered = true;
          return RegisterCallbackWithPublisher();
        }
      
        return true;
      }
      
      bool RemoveEventCallback(ePublisherEvent type_)
      {
        const std::lock_guard<std::mutex> guard(m_event_callback_map_mutex);
        m_event_callback_map[type_] = nullptr;
      
        return true;
      }

    private:
      bool RegisterCallbackWithPublisher()
      {
        auto publisher = m_publisher_impl.lock();
        if (publisher == nullptr)
          return false;
      
        auto internal_callback = [this](const STopicId& topic_id_, const eCAL::SPubEventCallbackData& callback_data_)
          {
            const std::lock_guard<std::mutex> guard(m_event_callback_map_mutex);

            const auto& v5_callback = m_event_callback_map.find(callback_data_.event_type);
            if (v5_callback != m_event_callback_map.end() && v5_callback->second != nullptr)
            {
              auto v5_callback_data = v6tov5CallbackData(topic_id_, callback_data_);
              v5_callback->second(topic_id_.topic_name.c_str(), &v5_callback_data);
            }
          };
      
        return publisher->SetEventCallback(internal_callback);
      }

      v5::SPubEventCallbackData v6tov5CallbackData(const STopicId& topic_id_, const eCAL::SPubEventCallbackData& v6_callback_data_)
      {
        SPubEventCallbackData v5_callback_data;
        v5_callback_data.type = v6_callback_data_.event_type;
        v5_callback_data.time = v6_callback_data_.event_time;
        v5_callback_data.clock = 0;
        v5_callback_data.tid = std::to_string(topic_id_.topic_id.entity_id);
        v5_callback_data.tdatatype = v6_callback_data_.subscriber_datatype;
        return v5_callback_data;
      }

      using EventCallbackMapT = std::map<ePublisherEvent, v5::PubEventCallbackT>;
      std::mutex                             m_event_callback_map_mutex;
      EventCallbackMapT                      m_event_callback_map;
      std::weak_ptr<CPublisherImpl>          m_publisher_impl;
      bool                                   m_is_registered;
    };

    CPublisher::CPublisher() :
      m_publisher_impl(nullptr),
      m_filter_id(0)
    {
    }

    CPublisher::CPublisher(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_)
      : CPublisher()
    {
      CPublisher::Create(topic_name_, data_type_info_, config_);
    }

    CPublisher::CPublisher(const std::string& topic_name_, const Publisher::Configuration& config_)
      : CPublisher(topic_name_, SDataTypeInformation{}, config_)
    {}

    CPublisher::~CPublisher()
    {
      CPublisher::Destroy();
    }

    /**
     * @brief CPublisher are move-enabled
    **/
    CPublisher::CPublisher(CPublisher&& rhs) noexcept :
                  m_publisher_impl(std::move(rhs.m_publisher_impl)),
                  m_callback_adapter(std::move(rhs.m_callback_adapter)),
                  m_filter_id(rhs.m_filter_id)
    {
      rhs.m_publisher_impl = nullptr;
      rhs.m_callback_adapter = nullptr;
    }

    /**
     * @brief CPublisher are move-enabled
    **/
    CPublisher& CPublisher::operator=(CPublisher&& rhs) noexcept
    {
      // Call destroy, to clean up the current state, then afterwards move all elements
      Destroy();

      m_publisher_impl = std::move(rhs.m_publisher_impl);
      m_callback_adapter = std::move(rhs.m_callback_adapter);
      m_filter_id  = rhs.m_filter_id;
      
      rhs.m_publisher_impl = nullptr;

      return *this;
    }

    bool CPublisher::Create(const std::string& topic_name_, const SDataTypeInformation& data_type_info_, const Publisher::Configuration& config_)
    {
      if (m_publisher_impl != nullptr) return(false);
      if (topic_name_.empty())     return(false);

      auto config = eCAL::GetConfiguration();
      config.publisher = config_;

      // create publisher
      m_publisher_impl = std::make_shared<CPublisherImpl>(data_type_info_, BuildWriterAttributes(topic_name_, config));
      m_callback_adapter = std::make_shared<CEventCallbackAdapater>(m_publisher_impl);

      // register publisher
      g_pubgate()->Register(topic_name_, m_publisher_impl);

      // we made it :-)
      return(true);
    }

    bool CPublisher::Create(const std::string& topic_name_)
    {
      return Create(topic_name_, SDataTypeInformation());
    }

    bool CPublisher::Destroy()
    {
      if (m_publisher_impl == nullptr) return(false);

      // unregister publisher
      if(g_pubgate() != nullptr) g_pubgate()->Unregister(m_publisher_impl->GetTopicName(), m_publisher_impl);
  #ifndef NDEBUG
      // log it
      eCAL::Logging::Log(Logging::log_level_debug1, std::string(m_publisher_impl->GetTopicName() + "::CPublisher::Destroy"));
  #endif

      // destroy publisher
      m_publisher_impl.reset();

      // we made it :-)
      return(true);
    }

    bool CPublisher::SetDataTypeInformation(const SDataTypeInformation& data_type_info_)
    {
      if (m_publisher_impl == nullptr) return false;
      return m_publisher_impl->SetDataTypeInformation(data_type_info_);
    }

    bool CPublisher::SetAttribute(const std::string& attr_name_, const std::string& attr_value_)
    {
      if(m_publisher_impl == nullptr) return false;
      return m_publisher_impl->SetAttribute(attr_name_, attr_value_);
    }

    bool CPublisher::ClearAttribute(const std::string& attr_name_)
    {
      if(m_publisher_impl == nullptr) return false;
      return m_publisher_impl->ClearAttribute(attr_name_);
    }

    bool CPublisher::SetID(long long filter_id_)
    {
      m_filter_id = filter_id_;
      return true;
    }

    size_t CPublisher::Send(const void* const buf_, const size_t len_, const long long time_ /* = DEFAULT_TIME_ARGUMENT */)
    {
      CBufferPayloadWriter payload{ buf_, len_ };
      return Send(payload, time_);
    }
    
    size_t CPublisher::Send(CPayloadWriter& payload_, long long time_)
    {
      if (m_publisher_impl == nullptr) return 0;

       // in an optimization case the
       // publisher can send an empty package
       // or we do not have any subscription at all
       // then the data writer will only do some statistics
       // for the monitoring layer and return
       if (!IsSubscribed())
       {
         m_publisher_impl->RefreshSendCounter();
         return(payload_.GetSize());
       }

       // send content via data writer layer
       const long long write_time = (time_ == DEFAULT_TIME_ARGUMENT) ? eCAL::Time::GetMicroSeconds() : time_;
       const size_t written_bytes = m_publisher_impl->Write(payload_, write_time, m_filter_id);

       // return number of bytes written
       return written_bytes;
    }

    size_t CPublisher::Send(const std::string& s_, long long time_)
    {
      return(Send(s_.data(), s_.size(), time_));
    }

    bool CPublisher::AddEventCallback(ePublisherEvent type_, PubEventCallbackT callback_)
    {
      if (m_publisher_impl == nullptr) return(false);
      RemEventCallback(type_);

      return(m_callback_adapter->AddEventCallback(type_, callback_));
    }

    bool CPublisher::RemEventCallback(ePublisherEvent type_)
    {
      if (m_publisher_impl == nullptr) return(false);
      return(m_callback_adapter->RemoveEventCallback(type_));
    }

    bool CPublisher::IsSubscribed() const
    {
  #if ECAL_CORE_REGISTRATION
      if(m_publisher_impl == nullptr) return(false);
      return(m_publisher_impl->IsSubscribed());
  #else  // ECAL_CORE_REGISTRATION
      return(true);
  #endif // ECAL_CORE_REGISTRATION
    }

    size_t CPublisher::GetSubscriberCount() const
    {
      if (m_publisher_impl == nullptr) return(0);
      return(m_publisher_impl->GetSubscriberCount());
    }

    std::string CPublisher::GetTopicName() const
    {
      if(m_publisher_impl == nullptr) return("");
      return(m_publisher_impl->GetTopicName());
    }

    STopicId CPublisher::GetId() const
    {
      if (m_publisher_impl == nullptr) return{};
      return(m_publisher_impl->GetTopicId());
    }

    SDataTypeInformation CPublisher::GetDataTypeInformation() const
    {
      if (m_publisher_impl == nullptr) return(SDataTypeInformation{});
      return(m_publisher_impl->GetDataTypeInformation());
    }

    std::string CPublisher::Dump(const std::string& indent_ /* = "" */) const
    {
      std::stringstream out;

      out << indent_ << "----------------------" << '\n';
      out << indent_ << " class CPublisher"      << '\n';
      out << indent_ << "----------------------" << '\n';
      out << indent_ << "DUMP NOT SUPPORTED ANYMORE" << '\n';
      out << indent_ << "----------------------" << '\n';
      out << '\n';

      return(out.str());
    }
  }
}
