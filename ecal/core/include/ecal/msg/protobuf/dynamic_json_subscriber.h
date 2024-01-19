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
#include <memory>

namespace eCAL
{
  namespace protobuf
  {

    class CDynamicJSONSubscriberImpl;

    /**
     * @brief eCAL dynamic protobuf to json subscriber.
    **/
    class CDynamicJSONSubscriber
    {
    public:
      /**
       * @brief Constructor.
      **/
      ECAL_API CDynamicJSONSubscriber();

      /**
       * @brief Constructor.
       *
       * @param topic_name_  Unique topic name.
      **/
      ECAL_API CDynamicJSONSubscriber(const std::string& topic_name_);

      /**
       * @brief Destructor.
      **/
      ECAL_API ~CDynamicJSONSubscriber();

      CDynamicJSONSubscriber(const CDynamicJSONSubscriber&) = delete;
      CDynamicJSONSubscriber& operator=(const CDynamicJSONSubscriber&) = delete;

      /**
       * @brief Move constructor
      **/
      ECAL_API CDynamicJSONSubscriber(CDynamicJSONSubscriber&& rhs);

      /**
       * @brief Move assignment
      **/
      ECAL_API CDynamicJSONSubscriber& operator=(CDynamicJSONSubscriber&& rhs);

      /**
       * @brief Creates this object.
       *
       * @param topic_name_   Unique topic name.
       *
       * @return  true if it succeeds, false if it fails.
      **/
      ECAL_API void Create(const std::string& topic_name_);

      /**
       * @brief Destroys this object.
       *
       * @return  true if it succeeds, false if it fails.
      **/
      ECAL_API void Destroy();

      /**
       * @brief Query if this object is created.
       *
       * @return  true if created, false if not.
      **/
      ECAL_API bool IsCreated() { return(created); }

      /**
       * @brief Add callback function for incoming receives.
       *
       * @param callback_  The callback function to add.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API bool AddReceiveCallback(ReceiveCallbackT callback_);

      /**
       * @brief Remove callback function for incoming receives.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API bool RemReceiveCallback();

    protected:
      bool                                        created;
      std::unique_ptr<CDynamicJSONSubscriberImpl> proto_dyn_sub_impl;
    };
    /** @example proto_dyn_json.cpp
    * This is an example how to use CDynamicJSONSubscriber to receive dynamic google::protobuf data as a JSON string with eCAL.
    */
  }
}
