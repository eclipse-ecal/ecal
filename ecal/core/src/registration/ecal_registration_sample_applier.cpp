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

#include "registration/ecal_registration_sample_applier.h"

namespace eCAL
{
  namespace Registration
  {
    //////////////////////////////////////////////////////////////////
    // CSampleApplier
    //////////////////////////////////////////////////////////////////
    CSampleApplier::CSampleApplier(const SampleApplier::SAttributes& attr_)
      : m_attributes(attr_)
    {
    }

    bool CSampleApplier::ApplySample(const Registration::Sample& sample_)
    {
      if (!AcceptRegistrationSample(sample_))
      {
        Logging::Log(Logging::log_level_debug1, "CSampleApplier::ApplySample : Incoming sample discarded");
        return false;
      }

      // forward all registration samples to outside "customer" (e.g. monitoring, descgate, pub/subgate/client/service gates)
      {
        const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
        for (const auto& iter : m_callback_custom_apply_sample_map)
        {
          iter.second(sample_);
        }
      }
      return true;
    }

    bool CSampleApplier::IsShmTransportDomainMember(const Registration::Sample& sample_) const
    {
      // When are we in the same domain?
      // Either we are on the same host
      // Or the shm_transport_domain attribute of the sample are identical

      if (IsSameHost(sample_))
        return true;

      if (IsSameShmTransportDomain(sample_))
        return true;

      return false;
    }

    bool CSampleApplier::IsSameProcess(const Registration::Sample& sample_) const
    {
      // is this actually sufficient? should we also check host_name?
      const int32_t process_id = sample_.identifier.process_id;
      return process_id == m_attributes.process_id;
    }

    bool CSampleApplier::IsSameHost(const Registration::Sample& sample_) const
    {
      const std::string& sample_host_name = sample_.identifier.host_name;
      return (sample_host_name == m_attributes.host_name);
    }

    bool CSampleApplier::IsSameShmTransportDomain(const Registration::Sample& sample_) const
    {
      std::string sample_shm_transport_domain;
      switch (sample_.cmd_type)
      {
      case bct_reg_publisher:
      case bct_unreg_publisher:
      case bct_reg_subscriber:
      case bct_unreg_subscriber:
        sample_shm_transport_domain = sample_.topic.shm_transport_domain;
        break;
      case bct_reg_service:
      case bct_unreg_service:
        //shm_transport_domain = sample_.service.shm_transport_domain;  // TODO: we need to add shm_transport_domain attribute to services
        break;
      case bct_reg_client:
      case bct_unreg_client:
        //shm_transport_domain = sample_.client.shm_transport_domain;  // TODO: we need to add shm_transport_domain attribute to clients
        break;
      default:
        break;
      }

      return (sample_shm_transport_domain == m_attributes.shm_transport_domain);
    }

    bool CSampleApplier::AcceptRegistrationSample(const Registration::Sample& sample_)
    {
      // Under wich circumstances do we apply samples, so we can filter ahead of time
      // otherwise we could apply them to

      // check if the sample is from the same shm transport domain
      if (IsShmTransportDomainMember(sample_))
      {
        // register if the sample is from another process
        // or if loopback mode is enabled
        return !IsSameProcess(sample_) || m_attributes.loopback;
      }
      else
      {
        // if the sample is from an external host, register only if network mode is enabled
        return m_attributes.network_enabled;
      }
    }

    void CSampleApplier::SetCustomApplySampleCallback(const std::string& customer_, const ApplySampleCallbackT& callback_)
    {
      const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
      m_callback_custom_apply_sample_map[customer_] = callback_;
    }

    void CSampleApplier::RemCustomApplySampleCallback(const std::string& customer_)
    {
      const std::lock_guard<std::mutex> lock(m_callback_custom_apply_sample_map_mtx);
      auto iter = m_callback_custom_apply_sample_map.find(customer_);
      if (iter != m_callback_custom_apply_sample_map.end())
      {
        m_callback_custom_apply_sample_map.erase(iter);
      }
    }
  }
}