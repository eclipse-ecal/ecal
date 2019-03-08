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

#ifndef ECAL_C_DLL

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/dynamic_subscriber.h>

namespace eCAL
{
  namespace protobuf
  {

    class CDynamicJSONSubscriberImpl;

    /**
     * @brief eCAL dynamic protobuf to json subscriber.
    **/
    class ECAL_API CDynamicJSONSubscriber
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
      CDynamicJSONSubscriber(const std::string& topic_name_);

      /**
       * @brief Destructor.
      **/
      ~CDynamicJSONSubscriber();

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
      bool IsCreated() { return(created); }

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
      bool                          created;
      CDynamicJSONSubscriberImpl*   proto_dyn_sub_impl;

    private:
      // this object must not be copied.
      CDynamicJSONSubscriber(const CDynamicSubscriber&);
      CDynamicJSONSubscriber& operator=(const CDynamicSubscriber&);
    };
    /** @example proto_dyn_json.cpp
    * This is an example how to use CDynamicJSONSubscriber to receive dynamic google::protobuf data as a JSON string with eCAL.
    */
  }
}

#else /*ECAL_C_DLL*/

#include <ecal/ecalc.h>

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
      CDynamicJSONSubscriber() :
        m_subscriber(nullptr),
        m_created(false)
      {}

      /**
       * @brief Constructor.
       *
       * @param topic_name_  Unique topic name.
      **/
      CDynamicJSONSubscriber(const std::string& topic_name_) :
        m_subscriber(nullptr),
        m_created(false)
      {
        Create(topic_name_);
      }

      /**
       * @brief Destructor.
      **/
      ~CDynamicJSONSubscriber()
      {
        Destroy();
      }


      /**
       * @brief Creates this object.
       *
       * @param topic_name_   Unique topic name.
       *
       * @return  true if it succeeds, false if it fails.
      **/
      void Create(const std::string& topic_name_)
      {
        if (m_created) return;
        m_subscriber = eCAL_Proto_Dyn_JSON_Sub_Create(topic_name_.c_str());
        if (!m_subscriber) return;

        m_topic_name = topic_name_;
        m_created = true;
      }

      /**
       * @brief Destroys this object.
       *
       * @return  true if it succeeds, false if it fails.
      **/
      void Destroy()
      {
        if (!m_created) return;
        if (m_subscriber) eCAL_Proto_Dyn_JSON_Sub_Destroy(m_subscriber);

        m_subscriber = nullptr;
        m_topic_name = "";
        m_topic_type = "";
        m_created = false;
      }

      /**
       * @brief Query if this object is created.
       *
       * @return  true if created, false if not.
      **/
      bool IsCreated() { return(m_created); }

      /**
       * @brief Add callback function for incoming receives.
       *
       * @param callback_  The callback function to add.
       *
       * @return  True if succeeded, false if not.
      **/
      static void ReceiveCallback(const char* topic_name_, const struct SReceiveCallbackDataC* data_, void* par_)
      {
        if (par_ == nullptr) return;
        CDynamicJSONSubscriber* sub = static_cast<CDynamicJSONSubscriber*>(par_);
        SReceiveCallbackData data;
        data.buf = data_->buf;
        data.size = data_->size;
        data.id = data_->id;
        data.time = data_->time;
        data.clock = data_->clock;
        sub->m_receive_callback(topic_name_, &data);
      }
      bool AddReceiveCallback(ReceiveCallbackT callback_)
      {
        if (!m_subscriber) return(false);
        m_receive_callback = callback_;
        return(eCAL_Proto_Dyn_JSON_Sub_AddReceiveCallbackC(m_subscriber, &CDynamicJSONSubscriber::ReceiveCallback, this) != 0);
      }

      /**
       * @brief Remove callback function for incoming receives.
       *
       * @return  True if succeeded, false if not.
      **/
      bool RemReceiveCallback()
      {
        if (!m_subscriber) return(false);
        return(eCAL_Proto_Dyn_JSON_Sub_RemReceiveCallback(m_subscriber) != 0);
      }

    protected:
      ECAL_HANDLE      m_subscriber;
      std::string      m_topic_name;
      std::string      m_topic_type;
      ReceiveCallbackT m_receive_callback;
      //EventCallbackT   m_event_callback; (Talk about later)
      bool             m_created;
    };
  }

}

#endif /*ECAL_C_DLL*/
