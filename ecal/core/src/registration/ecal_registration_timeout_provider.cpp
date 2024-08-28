/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2024 Continental Corporation
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
        auto& unregister_sample_process = unregister_sample.process;
        unregister_sample_process.pname = sample_process.pname;
        unregister_sample_process.uname = sample_process.uname;
      }

      if (IsTopicRegistration(unregister_sample))
      {
        const auto& sample_topic = sample_.topic;
        auto& unregister_sample_topic = unregister_sample.topic;
        unregister_sample_topic.hgname = sample_topic.hgname;
        unregister_sample_topic.pname = sample_topic.pname;
        unregister_sample_topic.tname = sample_topic.tname;
        unregister_sample_topic.uname = sample_topic.uname;
      }

      if (unregister_sample.cmd_type == bct_unreg_service)
      {
        const auto& sample_service = sample_.service;
        auto& unregister_sample_service = unregister_sample.service;
        unregister_sample_service.pname = sample_service.pname;
        unregister_sample_service.sname = sample_service.sname;
        unregister_sample_service.uname = sample_service.uname;
        unregister_sample_service.version = sample_service.version;
      }

      if (unregister_sample.cmd_type == bct_unreg_client)
      {
        const auto& sample_client = sample_.client;
        auto& unregister_sample_client = unregister_sample.client;

        unregister_sample_client.pname = sample_client.pname;
        unregister_sample_client.sname = sample_client.sname;
        unregister_sample_client.uname = sample_client.uname;
        unregister_sample_client.version = sample_client.version;
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
