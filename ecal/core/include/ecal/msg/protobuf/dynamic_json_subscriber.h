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
      CDynamicJSONSubscriber(const CDynamicJSONSubscriber&);
      CDynamicJSONSubscriber& operator=(const CDynamicJSONSubscriber&);
    };
    /** @example proto_dyn_json.cpp
    * This is an example how to use CDynamicJSONSubscriber to receive dynamic google::protobuf data as a JSON string with eCAL.
    */
  }
}
