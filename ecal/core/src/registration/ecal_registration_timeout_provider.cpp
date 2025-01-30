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
 * @brief  eCAL registration receiver
 *
 * Receives registration information from external eCAL processes and forwards them to
 * the internal publisher/subscriber, server/clients.
 *
**/

#include "registration/ecal_registration_timeout_provider.h"


namespace eCAL
{
  namespace Registration
  {
    bool IsUnregistrationSample(const Registration::Sample& sample_)
    {
      return sample_.cmd_type == bct_unreg_client ||
        sample_.cmd_type == bct_unreg_process ||
        sample_.cmd_type == bct_unreg_publisher ||
        sample_.cmd_type == bct_unreg_service ||
        sample_.cmd_type == bct_unreg_subscriber;
    }

    Sample CreateUnregisterSample(const Sample& sample_)
    {
      Sample unregister_sample;

      unregister_sample.cmd_type = GetUnregistrationType(sample_);
      unregister_sample.identifier = sample_.identifier;

      if (IsProcessRegistration(unregister_sample))
      {
        const auto& sample_process = sample_.process;
        auto& unregister_sample_process        = unregister_sample.process;
        unregister_sample_process.process_name = sample_process.process_name;
        unregister_sample_process.unit_name    = sample_process.unit_name;
      }

      if (IsTopicRegistration(unregister_sample))
      {
        const auto& sample_topic                     = sample_.topic;
        auto& unregister_sample_topic                = unregister_sample.topic;
        unregister_sample_topic.shm_transport_domain = sample_topic.shm_transport_domain;
        unregister_sample_topic.process_name         = sample_topic.process_name;
        unregister_sample_topic.topic_name           = sample_topic.topic_name;
        unregister_sample_topic.unit_name            = sample_topic.unit_name;
      }

      if (unregister_sample.cmd_type == bct_unreg_service)
      {
        const auto& sample_service = sample_.service;
        auto& unregister_sample_service = unregister_sample.service;
        unregister_sample_service.process_name = sample_service.process_name;
        unregister_sample_service.service_name = sample_service.service_name;
        unregister_sample_service.unit_name    = sample_service.unit_name;
        unregister_sample_service.version      = sample_service.version;
      }

      if (unregister_sample.cmd_type == bct_unreg_client)
      {
        const auto& sample_client = sample_.client;
        auto& unregister_sample_client = unregister_sample.client;

        unregister_sample_client.process_name = sample_client.process_name;
        unregister_sample_client.service_name = sample_client.service_name;
        unregister_sample_client.unit_name    = sample_client.unit_name;
        unregister_sample_client.version      = sample_client.version;
      }
      return unregister_sample;
    }

    eCmdType GetUnregistrationType(const Registration::Sample& sample_)
    {
      if (sample_.cmd_type == bct_reg_client)
        return bct_unreg_client;
      if (sample_.cmd_type == bct_reg_process)
        return bct_unreg_process;
      if (sample_.cmd_type == bct_reg_publisher)
        return bct_unreg_publisher;
      if (sample_.cmd_type == bct_reg_service)
        return bct_unreg_service;
      if (sample_.cmd_type == bct_reg_subscriber)
        return bct_unreg_subscriber;
      return bct_none;
    }

    bool IsProcessRegistration(const Registration::Sample& sample_)
    {
      return sample_.cmd_type == bct_reg_process ||
        sample_.cmd_type == bct_unreg_process;
    }

    bool IsTopicRegistration(const Registration::Sample& sample_)
    {
      return sample_.cmd_type == bct_reg_publisher ||
        sample_.cmd_type == bct_reg_subscriber ||
        sample_.cmd_type == bct_unreg_publisher ||
        sample_.cmd_type == bct_unreg_subscriber;
    }
  }
}
