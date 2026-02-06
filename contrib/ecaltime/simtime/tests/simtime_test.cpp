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

#include <gtest/gtest.h>

#include <thread>

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4146 4505 4800 4189 4592) // disable proto warnings
#endif
#include <ecal/ecaltime/pb/sim_time.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
constexpr int CMN_REGISTRATION_REFRESH_MS = 1000;

class SimTimePublisher
{
public:
  SimTimePublisher()
    : sim_time_pub("__sim_time__")
  {
    sim_time_msg.set_host_name(eCAL::Process::GetHostName());
    sim_time_msg.set_real_time_factor(1.0);
    sim_time_msg.set_simulation_state(eCAL::pb::SimTime_eState::SimTime_eState_playing);
  }

  void publishSimTime(uint64_t sim_time_ns)
  {
    sim_time_msg.set_simulation_time_nsecs(sim_time_ns);
    sim_time_msg.set_local_time_nsecs(std::chrono::steady_clock::now().time_since_epoch().count());
    sim_time_pub.Send(sim_time_msg);
  }

private:
  eCAL::pb::SimTime sim_time_msg;
  eCAL::protobuf::CPublisher<eCAL::pb::SimTime> sim_time_pub;
};

constexpr int EXPECTED_OK_STATUS = 0;
constexpr int EXPECTED_ERROR_STATUS_NO_MESSAGES_RECEIVED = 2;

void ExpectStatus(int status_code)
{
  int ecal_time_status{ 0 };
  std::string ecal_time_status_message;
  eCAL::Time::GetStatus(ecal_time_status, &ecal_time_status_message);
  EXPECT_EQ(ecal_time_status, status_code) << ecal_time_status_message;
}

/*
* eCAL Monitoring capabilities are not activated by default to save resources.
*/
TEST(contrib_cpp_simtime, TestSimtime)
{
  const std::string timesync_module_name{ "ecaltime-simtime" };
  auto config = eCAL::Init::Configuration();
  config.timesync.timesync_module_rt = timesync_module_name;
  config.publisher.layer.shm.acknowledge_timeout_ms = 100;
  eCAL::Initialize(config, "contrib_cpp_simtime_test");

  EXPECT_EQ(eCAL::Time::GetName(), timesync_module_name);
  
  // We expect that the plugin has been loaded correctly, but not necessarily yet working
  ExpectStatus(EXPECTED_ERROR_STATUS_NO_MESSAGES_RECEIVED);

  SimTimePublisher sim_time_pub;
  std::this_thread::sleep_for(std::chrono::milliseconds(CMN_REGISTRATION_REFRESH_MS * 3));

  sim_time_pub.publishSimTime(1000);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  auto ecal_time_ns = eCAL::Time::GetNanoSeconds();
  EXPECT_GE(ecal_time_ns, 1000);

  ExpectStatus(EXPECTED_OK_STATUS);

  eCAL::Finalize();
}

