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

#include <cstddef>
#include <ecal/ecal.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <thread>

enum {
  CMN_MONITORING_TIMEOUT_MS = (5000 + 100),
  CMN_REGISTRATION_REFRESH_MS = (1000)
};

void AssertMonitoringHasOwnProcess(const eCAL::Monitoring::SMonitoring& monitoring)
{
  int process_id = eCAL::Process::GetProcessID();

  for (const auto& proc : monitoring.processes)
  {
    if (proc.process_id == process_id)
    {
      return;
    }
  }
  FAIL() << "Could not find publisher in monitoring";
}

void AssertMonitoringHasPublisher(const eCAL::Monitoring::SMonitoring& monitoring, const eCAL::STopicId& publisher_id, int send_counter)
{
  for (const auto& pub : monitoring.publishers)
  {
    if (pub.topic_id == publisher_id.topic_id.entity_id)
    {
      ASSERT_TRUE(pub.topic_name == publisher_id.topic_name) << "Topic name mismatch in monitoring data for publisher";
      ASSERT_TRUE(pub.data_clock == send_counter) << "Send counter mismatch in monitoring data for publisher";
      return;
    }
  }
  FAIL() << "Could not find publisher in monitoring";
}

void AssertMonitoringHasSubscriber(const eCAL::Monitoring::SMonitoring& monitoring, const eCAL::STopicId& subscriber_id, int send_counter)
{ 
  for (const auto& sub : monitoring.subscribers)
  {
    if (sub.topic_id == subscriber_id.topic_id.entity_id)
    {
      ASSERT_TRUE(sub.topic_name == subscriber_id.topic_name) << "Topic name mismatch in monitoring data for subscriber";
      ASSERT_TRUE(sub.data_clock == send_counter) << "Send counter mismatch in monitoring data for subscriber";
      return;
    }
  }
  FAIL() << "Could not find subscriber in monitoring";
}

void AssertMonitoringHasServer(const eCAL::Monitoring::SMonitoring& monitoring, const eCAL::SServiceId server_id, int call_counter)
{
  for (const auto& server : monitoring.servers)
  {
    if (server.service_id == server_id.service_id.entity_id)
    {
      ASSERT_TRUE(server.service_name == server_id.service_name) << "Service name mismatch in monitoring data for server";
      ASSERT_TRUE(server.methods[0].call_count == call_counter) << "Call counter mismatch in monitoring data for server";
      return;
    }
  }
  FAIL() << "Could not find server in monitoring";
}

void AssertMonitoringHasClient(const eCAL::Monitoring::SMonitoring& monitoring, const eCAL::SServiceId client_id, int call_counter)
{
  for (const auto& client : monitoring.clients)
  {
    if (client.service_id == client_id.service_id.entity_id)
    {
      ASSERT_TRUE(client.service_name == client_id.service_name) << "Service name mismatch in monitoring data for client";
      ASSERT_TRUE(client.methods[0].call_count == call_counter) << "Call counter mismatch in monitoring data for client";
      return;
    }
  }
  FAIL() << "Could not find client in monitoring";
}


/*
* eCAL Monitoring capabilities are not activated by default to save resources.
*/
TEST(core_cpp_monitoring, MonitoringNotActivatedByDefault)
{
  eCAL::Initialize("core_cpp_monitoring_monitoring_not_activated");

  eCAL::Monitoring::SMonitoring mon;
  bool got_monitoring = eCAL::Monitoring::GetMonitoring(mon);

  ASSERT_FALSE(got_monitoring) << "GetMonitoring should fail when monitoring is not activated by default";

  eCAL::Finalize();
}

/*
* eCAL Monitoring capabilities are not activated by default to save resources.
*/
TEST(core_cpp_monitoring, OwnProcessInMonitoring)
{
  eCAL::Initialize("core_cpp_monitoring_own_process_in_monitoring", eCAL::Init::All);

  // Wait some time to ensure monitoring data is populated
  std::this_thread::sleep_for(std::chrono::milliseconds(2 * CMN_REGISTRATION_REFRESH_MS));

  eCAL::Monitoring::SMonitoring mon;
  bool got_monitoring = eCAL::Monitoring::GetMonitoring(mon);
  ASSERT_TRUE(got_monitoring) << "GetMonitoring failed";
  AssertMonitoringHasOwnProcess(mon);

  eCAL::Finalize();
}

TEST(core_cpp_monitoring, PublisherSubscriberInMonitoring)
{
  eCAL::Initialize("core_cpp_monitoring_own_process_in_monitoring_after_delay", eCAL::Init::All);

  eCAL::CPublisher pub("test_topic");
  eCAL::CSubscriber sub("test_topic");

  std::this_thread::sleep_for(std::chrono::milliseconds(2 * CMN_REGISTRATION_REFRESH_MS));
  pub.Send("");
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  pub.Send("");
  std::this_thread::sleep_for(std::chrono::milliseconds(2 * CMN_REGISTRATION_REFRESH_MS));

  eCAL::Monitoring::SMonitoring mon;
  bool got_monitoring = eCAL::Monitoring::GetMonitoring(mon);
  ASSERT_TRUE(got_monitoring) << "GetMonitoring failed after delay";
  AssertMonitoringHasPublisher(mon, pub.GetTopicId(), 2);
  AssertMonitoringHasSubscriber(mon, sub.GetTopicId(), 2);

  eCAL::Finalize();
}

TEST(core_cpp_monitoring, ClientServerInMonitoring)
{
  eCAL::Initialize("core_cpp_monitoring_own_process_in_monitoring_after_delay", eCAL::Init::All);

  auto method_callback = [&](const eCAL::SServiceMethodInformation& method_info_, const std::string& request_, std::string& response_) -> int
    {
      response_ = request_;
      return 0;
    };

  eCAL::CServiceServer server("foo");
  eCAL::SServiceMethodInformation method1_info{ "foo::method1", {"foo::req_type1", "", ""}, {"foo::resp_type1", "", ""} };
  server.SetMethodCallback(method1_info, method_callback);

  eCAL::CServiceClient client("foo", {method1_info});

  std::this_thread::sleep_for(std::chrono::milliseconds(2 * CMN_REGISTRATION_REFRESH_MS));
  client.GetClientInstances()[0].CallWithResponse("foo::method1", "request", 1000);
  client.GetClientInstances()[0].CallWithResponse("foo::method1", "request", 1000);
  std::this_thread::sleep_for(std::chrono::milliseconds(2 * CMN_REGISTRATION_REFRESH_MS));

  eCAL::Monitoring::SMonitoring mon;
  bool got_monitoring = eCAL::Monitoring::GetMonitoring(mon);
  ASSERT_TRUE(got_monitoring) << "GetMonitoring failed after delay";
  AssertMonitoringHasServer(mon, server.GetServiceId(), 2);
  AssertMonitoringHasClient(mon, client.GetServiceId(), 2);

  eCAL::Finalize();
}