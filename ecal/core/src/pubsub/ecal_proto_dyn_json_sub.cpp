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
 * dynamic protobuf message decoder
**/

#include <ecal/ecal_os.h>
#include <ecal/msg/protobuf/dynamic_json_subscriber.h>
#include <ecal/msg/protobuf/dynamic_subscriber.h>

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4800) // disable proto warnings
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
    class CDynamicJSONSubscriberImpl
    {
    public:
      CDynamicJSONSubscriberImpl() :
        created(false),
        msg_decoder(nullptr),
        msg_ptr()
      {}

      CDynamicJSONSubscriberImpl(const std::string& topic_name_) :
        created(false),
        msg_decoder(nullptr),
        msg_ptr()
      {
        Create(topic_name_);
      }

      ~CDynamicJSONSubscriberImpl()
      {
        Destroy();
      }

      void Create(const std::string& topic_name_)
      {
        if (created) return;

        // create message decoder
        msg_decoder = new eCAL::protobuf::CProtoDynDecoder();

        // create subscriber
        msg_sub.Create(topic_name_);

        // add callback
        msg_sub.AddReceiveCallback(std::bind(&CDynamicJSONSubscriberImpl::OnReceive, this, std::placeholders::_1, std::placeholders::_2));

        created = true;
      }

      void Destroy()
      {
        if (!created) return;

        // remove callback
        msg_sub.RemReceiveCallback();

        // destroy subscriber
        msg_sub.Destroy();

        // delete message decoder
        delete msg_decoder;

        created = false;
      }

      void AddReceiveCallback(ReceiveCallbackT callback_)
      {
        msg_callback = callback_;
      }

      void RemReceiveCallback()
      {
        msg_callback = nullptr;
      }

    protected:
      void OnReceive(const char* topic_name_, const struct eCAL::SReceiveCallbackData* data_)
      {
        if (msg_ptr.empty())
        {
          // get topic type
          topic_type_full = eCAL::Util::GetTypeName(topic_name_);
          topic_type_full = topic_type_full.substr(topic_type_full.find_first_of(':') + 1, topic_type_full.size());
          topic_type = topic_type_full.substr(topic_type_full.find_last_of('.') + 1, topic_type_full.size());
          topic_type_full = "/" + topic_type_full;

          if (topic_type.empty())
          {
            std::cout << "could not get type for topic " << topic_name_ << std::endl;
            return;
          }

          // get topic description
          topic_desc = eCAL::Util::GetDescription(topic_name_);
          if (topic_desc.empty())
          {
            std::cout << "could not get description for topic " << topic_name_ << std::endl;
            return;
          }

          std::string error_s;
          google::protobuf::FileDescriptorSet proto_desc;
          proto_desc.ParseFromString(topic_desc);
          msg_decoder->GetProtoMessageFromDescriptorSet(proto_desc, topic_type, error_s);
          resolver_.reset(google::protobuf::util::NewTypeResolverForDescriptorPool("", msg_decoder->GetDescriptorPool()));
        }

        // decode message and execute callback
        //if(msg_callback && msg_ptr && msg_ptr->ParseFromArray(data_->buf, data_->size))
        if (msg_callback)
        {

          google::protobuf::util::JsonOptions options;
          options.always_print_primitive_fields = true;

          std::string binary_input;
          binary_input.assign((char*)data_->buf, static_cast<size_t>(data_->size));
          msg_ptr.clear();
          auto status = google::protobuf::util::BinaryToJsonString(resolver_.get(), topic_type_full, binary_input, &msg_ptr, options);
          if (status == google::protobuf::util::Status::OK)
          {
            SReceiveCallbackData cb_data;
            cb_data.buf = (void*)msg_ptr.c_str();
            cb_data.size = (long)msg_ptr.size();
            cb_data.time = data_->time;
            msg_callback(topic_name_, &cb_data);
          }
        }
      }

      bool                              created;
      eCAL::protobuf::CProtoDynDecoder* msg_decoder;
      std::string                       msg_ptr;
      eCAL::CSubscriber                 msg_sub;
      ReceiveCallbackT                  msg_callback;

      std::string topic_type;
      std::string topic_type_full;

      std::string topic_desc;
      std::shared_ptr<google::protobuf::util::TypeResolver> resolver_;
      google::protobuf::DescriptorPool m_descriptor_pool;

    private:
      // this object must not be copied.
      CDynamicJSONSubscriberImpl(const CDynamicSubscriber&);
      CDynamicJSONSubscriberImpl& operator=(const CDynamicSubscriber&);
    };

    CDynamicJSONSubscriber::CDynamicJSONSubscriber() :
      created(false),
      proto_dyn_sub_impl(nullptr)
    {
    }

    CDynamicJSONSubscriber::CDynamicJSONSubscriber(const std::string& topic_name_) :
      created(false)
    {
      Create(topic_name_);
    }

    CDynamicJSONSubscriber::~CDynamicJSONSubscriber()
    {
      Destroy();
    };

    void CDynamicJSONSubscriber::Create(const std::string& topic_name_)
    {
      if (created) return;
      proto_dyn_sub_impl = new CDynamicJSONSubscriberImpl(topic_name_);
      proto_dyn_sub_impl->Create(topic_name_);
      created = true;
    }

    void CDynamicJSONSubscriber::Destroy()
    {
      if (!created) return;
      proto_dyn_sub_impl->Destroy();
      delete proto_dyn_sub_impl;
      proto_dyn_sub_impl = nullptr;
      created = false;
    }

    bool CDynamicJSONSubscriber::AddReceiveCallback(ReceiveCallbackT callback_)
    {
      if (!created) return false;
      proto_dyn_sub_impl->AddReceiveCallback(callback_);
      return true;
    }

    bool CDynamicJSONSubscriber::RemReceiveCallback()
    {
      if (!created) return false;
      proto_dyn_sub_impl->RemReceiveCallback();
      return true;
    }
  }
}
