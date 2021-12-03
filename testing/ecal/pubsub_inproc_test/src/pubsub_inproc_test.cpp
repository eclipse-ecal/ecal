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
#include <ecal/msg/string/publisher.h>
#include <ecal/msg/string/subscriber.h>

#include <sstream>
#include <gtest/gtest.h>

#define CMN_REGISTRATION_REFRESH 1000

// callback function 1
size_t sub_clock1(0);
void OnReceive1(const std::string& msg_)
{
  std::cout << "PubClock    : " << msg_       << std::endl;
  std::cout << "SubClock  1 : " << sub_clock1 << std::endl;
  sub_clock1++;
}

// callback function 2
size_t sub_clock2(0);
void OnReceive2()
{
  std::cout << "SubClock  2 : " << sub_clock2 << std::endl;
  sub_clock2++;
}

// callback function 3
size_t sub_clock3(0);
void OnReceive3()
{
  std::cout << "SubClock  3 : " << sub_clock3 << std::endl;
  std::cout                                   << std::endl;
  sub_clock3++;
}

TEST(INPROC, CLOCKS)
{ 
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize(0, nullptr, "inproc_clock_test"));

  // publish / subscribe match in the same process
  eCAL::Util::EnableLoopback(true);

  // create simple string publisher
  eCAL::string::CPublisher<std::string> pub("CLOCK");

  // and force him to send on 3 layers in parallel
  pub.SetLayerMode(eCAL::TLayer::tlayer_all, eCAL::TLayer::smode_off);
  pub.SetLayerMode(eCAL::TLayer::tlayer_udp_mc, eCAL::TLayer::smode_on);
  pub.SetLayerMode(eCAL::TLayer::tlayer_shm, eCAL::TLayer::smode_on);
  pub.SetLayerMode(eCAL::TLayer::tlayer_inproc, eCAL::TLayer::smode_on);

  // create subscriber number 1
  eCAL::string::CSubscriber<std::string> sub1("CLOCK");
  EXPECT_EQ(true, sub1.AddReceiveCallback(std::bind(OnReceive1, std::placeholders::_2)));

  // create subscriber number 2
  eCAL::string::CSubscriber<std::string> sub2("CLOCK");
  EXPECT_EQ(true, sub2.AddReceiveCallback(std::bind(OnReceive2)));

  // create subscriber number 3
  eCAL::string::CSubscriber<std::string> sub3("CLOCK");
  EXPECT_EQ(true, sub3.AddReceiveCallback(std::bind(OnReceive3)));

  // let's match them
  eCAL::Process::SleepMS(2 * CMN_REGISTRATION_REFRESH);

  std::cout << std::endl;
  for (size_t count = 0; count <= 10; ++count)
  {
    // check all clocks
    // they should always count with
    // the publishing one
    EXPECT_EQ(count, sub_clock1);
    EXPECT_EQ(count, sub_clock2);
    EXPECT_EQ(count, sub_clock3);

    // create a clock string
    std::stringstream ss; ss << count;
    // and fire on the three layers
    pub.Send(ss.str());
  }

  // finalize eCAL API
  EXPECT_EQ(0, eCAL::Finalize());
}
