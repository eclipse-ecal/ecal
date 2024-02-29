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
 * @file   ecal_proto_dyn_json_sub.h
 * @brief  dynamic protobuf message to json decoder
**/

#pragma once

#include <ecal/ecal.h>
#include <ecal/ecal_os.h>
#include <ecal/msg/protobuf/ecal_proto_dyn.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <cstdio>

#ifdef _MSC_VER
#pragma warning(push, 0) // disable proto warnings
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver_util.h>
#include <google/protobuf/util/type_resolver.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace eCAL
{
  namespace protobuf
  {
    /**
     * @brief eCAL dynamic protobuf to json subscriber.
    **/
    class CDynamicJSONSubscriber
    {
    public:
      /**
       * @brief Constructor.
      **/
      CDynamicJSONSubscriber();

      /**
       * @brief Constructor.
       *
       * @param topic_name_  Unique topic name.
      **/
      explicit CDynamicJSONSubscriber(const std::string& topic_name_);

      /**
       * @brief Destructor.
      **/
      ~CDynamicJSONSubscriber();

      CDynamicJSONSubscriber(const CDynamicJSONSubscriber&) = delete;
      CDynamicJSONSubscriber& operator=(const CDynamicJSONSubscriber&) = delete;
      CDynamicJSONSubscriber(CDynamicJSONSubscriber&& rhs) = delete;
      CDynamicJSONSubscriber& operator=(CDynamicJSONSubscriber&& rhs) = delete;

      /**
       * @brief Creates this object.
       *
       * @param topic_name_   Unique topic name.
       *
       * @return  true if it succeeds, false if it fails.
      **/
      void Create(const std::string& topic_name_);

      /**
       * @brief Destroys this object.
       *
       * @return  true if it succeeds, false if it fails.
      **/
      void Destroy();

      /**
       * @brief Query if this object is created.
       *
       * @return  true if created, false if not.
      **/
      bool IsCreated() const { return(m_created); }

      /**
       * @brief Add callback function for incoming receives.
       *
       * @param callback_  The callback function to add.
       *
       * @return  True if succeeded, false if not.
      **/
      bool AddReceiveCallback(ReceiveCallbackT callback_);

      /**
       * @brief Remove callback function for incoming receives.
       *
       * @return  True if succeeded, false if not.
      **/
      bool RemReceiveCallback();

    protected:
      void OnReceive(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_);

      bool                                                  m_created;
      std::unique_ptr<eCAL::protobuf::CProtoDynDecoder>     m_msg_decoder;
      std::string                                           m_msg_string;
      eCAL::CSubscriber                                     m_msg_sub;
      ReceiveCallbackT                                      m_msg_callback;

      std::string                                           m_topic_type;
      std::string                                           m_topic_type_full;

      std::string m_topic_desc;
      std::shared_ptr<google::protobuf::util::TypeResolver> m_resolver;
      google::protobuf::DescriptorPool                      m_descriptor_pool;
    };
    /** @example proto_dyn_json.cpp
    * This is an example how to use CDynamicJSONSubscriber to receive dynamic google::protobuf data as a JSON string with eCAL.
    */

    inline CDynamicJSONSubscriber::CDynamicJSONSubscriber() :
      m_created(false),
      m_msg_decoder(nullptr)
    {}

    inline CDynamicJSONSubscriber::CDynamicJSONSubscriber(const std::string& topic_name_) :
      m_created(false),
      m_msg_decoder(nullptr)
    {
      Create(topic_name_);
    }

    inline CDynamicJSONSubscriber::~CDynamicJSONSubscriber()
    {
      Destroy();
    }

    inline void CDynamicJSONSubscriber::Create(const std::string& topic_name_)
    {
      if (m_created) return;

      // create message decoder
      m_msg_decoder = std::make_unique<eCAL::protobuf::CProtoDynDecoder>();

      // create subscriber
      m_msg_sub.Create(topic_name_);

      // add callback
      m_msg_sub.AddReceiveCallback(std::bind(&CDynamicJSONSubscriber::OnReceive, this, std::placeholders::_1, std::placeholders::_2));

      m_created = true;
    }

    inline void CDynamicJSONSubscriber::Destroy()
    {
      if (!m_created) return;

      // remove callback
      m_msg_sub.RemReceiveCallback();

      // destroy subscriber
      m_msg_sub.Destroy();

      // delete message decoder
      m_msg_decoder.reset();

      m_created = false;
    }

    inline bool CDynamicJSONSubscriber::AddReceiveCallback(ReceiveCallbackT callback_)
    {
      if (!m_created) return false;
      m_msg_callback = callback_;
      return true;
    }

    inline bool CDynamicJSONSubscriber::RemReceiveCallback()
    {
      if (!m_created) return false;
      m_msg_callback = nullptr;
      return true;
    }

    inline void CDynamicJSONSubscriber::OnReceive(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
    {
      if (m_msg_string.empty())
      {
        // get topic type
        SDataTypeInformation topic_info;
        //nodiscard???
        eCAL::Util::GetTopicDataTypeInformation(topic_name_, topic_info);
        m_topic_type_full = topic_info.name;
        m_topic_type      = m_topic_type_full.substr(m_topic_type_full.find_last_of('.') + 1, m_topic_type_full.size());
        m_topic_type_full = "/" + m_topic_type_full;

        if (m_topic_type.empty())
        {
          std::cout << "could not get type for topic " << topic_name_ << std::endl;
          return;
        }

        // get topic description
        m_topic_desc = topic_info.descriptor;
        if (m_topic_desc.empty())
        {
          std::cout << "could not get description for topic " << topic_name_ << std::endl;
          return;
        }

        std::string error_s;
        google::protobuf::FileDescriptorSet proto_desc;
        proto_desc.ParseFromString(m_topic_desc);
        const std::shared_ptr<google::protobuf::Message> msg(m_msg_decoder->GetProtoMessageFromDescriptorSet(proto_desc, m_topic_type, error_s));
        m_resolver.reset(google::protobuf::util::NewTypeResolverForDescriptorPool("", m_msg_decoder->GetDescriptorPool()));
      }

      // decode message and execute callback
      //if(msg_callback && msg_ptr && msg_ptr->ParseFromArray(data_->buf, data_->size))
      if (m_msg_callback)
      {

        google::protobuf::util::JsonOptions options;
        options.always_print_primitive_fields = true;

        std::string binary_input;
        binary_input.assign(static_cast<char*>(data_->buf), static_cast<size_t>(data_->size));
        m_msg_string.clear();
        auto status = google::protobuf::util::BinaryToJsonString(m_resolver.get(), m_topic_type_full, binary_input, &m_msg_string, options);
        if (status.ok())
        {
          SReceiveCallbackData cb_data;
          cb_data.buf  = (void*)m_msg_string.c_str();
          cb_data.size = (long)m_msg_string.size();
          cb_data.time = data_->time;
          m_msg_callback(topic_name_, &cb_data);
        }
      }
    }
  }
}
