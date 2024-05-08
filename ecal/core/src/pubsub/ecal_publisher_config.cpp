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
 * @brief  eCAL publisher configuration
**/

#include <ecal/ecal_config.h>
#include <ecal/ecal_publisher_config.h>

namespace eCAL
{
  namespace Publisher
  {
    Configuration::Configuration() :
      share_topic_type(eCAL::Config::IsTopicTypeSharingEnabled()),
      share_topic_description(eCAL::Config::IsTopicDescriptionSharingEnabled())
    {
      // shm config
      shm.activate                = eCAL::Config::GetPublisherShmMode() != TLayer::eSendMode::smode_off;
      shm.zero_copy_mode          = eCAL::Config::IsMemfileZerocopyEnabled();
      shm.acknowledge_timeout_ms  = eCAL::Config::GetMemfileAckTimeoutMs();

      shm.memfile_min_size_bytes  = eCAL::Config::GetMemfileMinsizeBytes();
      shm.memfile_reserve_percent = eCAL::Config::GetMemfileOverprovisioningPercentage();
      shm.memfile_buffer_count    = eCAL::Config::GetMemfileBufferCount();

      // udp config
      udp.activate                = eCAL::Config::GetPublisherUdpMulticastMode() != TLayer::eSendMode::smode_off;
      udp.sndbuf_size_bytes       = eCAL::Config::GetUdpMulticastSndBufSizeBytes();

      // tcp config
      tcp.activate                = eCAL::Config::GetPublisherTcpMode() != TLayer::eSendMode::smode_off;
    }
  }
}
