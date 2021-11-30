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

#include <ecal/ecal.h>

#include <iostream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4800) // disable proto warnings
#endif
#include "ecal/pb/ecal.pb.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

void OnProcessRegistration(const char* sample_, int sample_size_)
{
  eCAL::pb::Sample sample;
  if (sample.ParseFromArray(sample_, sample_size_))
  {
    std::cout << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << "Process Registration"               << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << sample.DebugString();
    std::cout                                         << std::endl;
  }
}

void OnServiceRegistration(const char* sample_, int sample_size_)
{
  eCAL::pb::Sample sample;
  if (sample.ParseFromArray(sample_, sample_size_))
  {
    std::cout << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << "Service Registration"               << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << sample.DebugString();
    std::cout                                         << std::endl;
  }
}

void OnClientRegistration(const char* sample_, int sample_size_)
{
  eCAL::pb::Sample sample;
  if (sample.ParseFromArray(sample_, sample_size_))
  {
    std::cout << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << "Client Registration"                << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << sample.DebugString();
    std::cout << std::endl;
  }
}

void OnSubscriberRegistration(const char* sample_, int sample_size_)
{
  eCAL::pb::Sample sample;
  if (sample.ParseFromArray(sample_, sample_size_))
  {
    std::cout << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << "Subscriber Registration"            << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << sample.DebugString();
    std::cout                                         << std::endl;
  }
}

void OnPublisherRegistration(const char* sample_, int sample_size_)
{
  eCAL::pb::Sample sample;
  if (sample.ParseFromArray(sample_, sample_size_))
  {
    std::cout << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << "Publisher Registration"             << std::endl;
    std::cout << "----------------------------------" << std::endl;
    std::cout << sample.DebugString();
    std::cout                                         << std::endl;
  }
}

int main(int argc, char **argv)
{
  // initialize eCAL API
  eCAL::Initialize(argc, argv, "monitoring registrations", eCAL::Init::None);

  // set process state
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "I feel good !");

  // add process register callback function
  eCAL::Process::AddRegistrationCallback(reg_event_process,    std::bind(OnProcessRegistration, std::placeholders::_1, std::placeholders::_2));

  // add service register callback function
  eCAL::Process::AddRegistrationCallback(reg_event_service,    std::bind(OnServiceRegistration, std::placeholders::_1, std::placeholders::_2));

  // add client register callback function
  eCAL::Process::AddRegistrationCallback(reg_event_client,     std::bind(OnClientRegistration, std::placeholders::_1, std::placeholders::_2));

  // add subscriber register callback function
  eCAL::Process::AddRegistrationCallback(reg_event_subscriber, std::bind(OnSubscriberRegistration, std::placeholders::_1, std::placeholders::_2));

  // add publisher register callback function
  eCAL::Process::AddRegistrationCallback(reg_event_publisher,  std::bind(OnPublisherRegistration, std::placeholders::_1, std::placeholders::_2));

  while(eCAL::Ok())
  {
    // sleep 100 ms
    eCAL::Process::SleepMS(100);
  }

  // finalize eCAL API
  eCAL::Finalize();

  return(0);
}
