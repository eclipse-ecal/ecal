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
 * @file   ecal_subscriber.h
 * @brief  eCAL subscriber interface
**/

#pragma once

#include <ecal/ecal_deprecate.h>
#include <ecal/ecal_os.h>

#include <ecal/ecal_callback.h>
#include <ecal/ecal_config.h>
#include <ecal/ecal_types.h>

#include <memory>
#include <string>

namespace eCAL
{
  class CSubscriberImpl;

  inline namespace v6
  {
    /**
     * @brief eCAL subscriber class.
    **/
    class ECAL_API_CLASS CSubscriber
    {
    public:
      class Arguments
      {
      public:

        SDataTypeInformation data_type_info;
        SubEventIDCallbackT event_callback = nullptr;
        Subscriber::Configuration config = GetSubscriberConfiguration();

        template <typename... Args>
        explicit Arguments(std::string topic_name_, Args&&... args)
          : topic_name_(std::move(topic_name_))
        {
          // Parse the variadic arguments
          parseArgs(std::forward<Args>(args)...);
        }

      private:
        void setArg(const SDataTypeInformation& data_type_info_)
        {
          data_type_info = data_type_info_;
        }

        void setArg(const SubEventIDCallbackT& event_callback_)
        {
          event_callback = event_callback_;
        }

        void setArg(const Subscriber::Configuration& config_)
        {
          config = config_;
        }

        // Base case: no args
        void parseArgs() { }

        // Recursive case: parse one argument at a time
        template <typename T, typename... Rest>
        void parseArgs(T&& arg, Rest&&... rest)
        {
          setArg(std::forward<T>(arg));
          parseArgs(std::forward<Rest>(rest)...);
        }
      };


      /**
       * @brief Constructor.
       *
       * @param topic_name_      Unique topic name.
       * @param data_type_info_  Topic data type information (encoding, type, descriptor).
       * @param config_          Optional configuration parameters.
      **/
      ECAL_API_EXPORTED_MEMBER
        CSubscriber(const Arguments& arguments);

      /**
       * @brief Destructor.
      **/
      ECAL_API_EXPORTED_MEMBER
        virtual ~CSubscriber();

      /**
       * @brief CSubscribers are non-copyable.
      **/
      CSubscriber(const CSubscriber&) = delete;

      /**
       * @brief CSubscribers are non-copyable.
      **/
      CSubscriber& operator=(const CSubscriber&) = delete;

      /**
       * @brief CSubscribers are move-enabled.
      **/
      ECAL_API_EXPORTED_MEMBER
        CSubscriber(CSubscriber&& rhs) noexcept;

      /**
       * @brief CSubscribers are move-enabled.
      **/
      ECAL_API_EXPORTED_MEMBER
        CSubscriber& operator=(CSubscriber&& rhs) noexcept;

      /**
       * @brief Set/overwrite callback function for incoming receives.
       *
       * @param callback_  The callback function to set.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool SetReceiveCallback(ReceiveIDCallbackT callback_);

      /**
       * @brief Remove callback function for incoming receives.
       *
       * @return  True if succeeded, false if not.
      **/
      ECAL_API_EXPORTED_MEMBER
        bool RemoveReceiveCallback();

      /**
       * @brief Query the number of connected publishers.
       *
       * @return  Number of publishers.
      **/
      ECAL_API_EXPORTED_MEMBER
        size_t GetPublisherCount() const;

      /**
       * @brief Retrieve the topic name.
       *
       * @return  The topic name.
      **/
      ECAL_API_EXPORTED_MEMBER
        std::string GetTopicName() const;

      /**
       * @brief Retrieve the topic id.
       *
       * @return  The topic id.
      **/
      ECAL_API_EXPORTED_MEMBER
        Registration::STopicId GetTopicId() const;

      /**
       * @brief Retrieve the topic information.
       *
       * @return  The topic information.
      **/
      ECAL_API_EXPORTED_MEMBER
        SDataTypeInformation GetDataTypeInformation() const;

    private:
      std::shared_ptr<CSubscriberImpl> m_subscriber_impl;
    };
  }
}
