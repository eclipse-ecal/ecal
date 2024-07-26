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
 * @brief  eCAL registration provider
 *
 * All process internal publisher/subscriber, server/clients register here with all their attributes.
 *
 * These information will be send cyclic (registration refresh) via UDP to external eCAL processes.
 *
**/

#include "registration/shm/ecal_registration_sender_shm.h"
#include "serialization/ecal_serialize_sample_registration.h"


eCAL::CRegistrationSenderSHM::CRegistrationSenderSHM()
{
  std::cout << "Shared memory monitoring is enabled (domain: " << Config::Experimental::GetShmMonitoringDomain() << " - queue size: " << Config::Experimental::GetShmMonitoringQueueSize() << ")" << '\n';
  m_memfile_broadcast.Create(Config::Experimental::GetShmMonitoringDomain(), Config::Experimental::GetShmMonitoringQueueSize());
  m_memfile_broadcast_writer.Bind(&m_memfile_broadcast);
}

eCAL::CRegistrationSenderSHM::~CRegistrationSenderSHM()
{
  m_memfile_broadcast_writer.Unbind();
  m_memfile_broadcast.Destroy();
}

/*
bool eCAL::CRegistrationSenderSHM::SendSample(const Registration::Sample& sample_)
{
  return false;
}
*/

bool eCAL::CRegistrationSenderSHM::SendSampleList(const Registration::SampleList& sample_list)
{
  bool return_value{true};
  // serialize whole sample list
  std::vector<char> sample_list_buffer;
  if (SerializeToBuffer(sample_list, sample_list_buffer))
  {
    if (!sample_list_buffer.empty())
    {
      // broadcast sample list over shm
      return_value &= m_memfile_broadcast_writer.Write(sample_list_buffer.data(), sample_list_buffer.size());
    }
  }
  return return_value;
}
