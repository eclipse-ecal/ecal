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

#include "ecal_descgate.h"

#include <gtest/gtest.h>

TEST(core_cpp_descgate, GetPublisher)
{
#if 0
  constexpr int desc_gate_expiration_ms(5000);
  eCAL::CDescGate desc_gate(std::chrono::milliseconds(desc_gate_expiration_ms));

  auto pub_map = desc_gate.GetPublisher();
  EXPECT_EQ(0, pub_map.size());
#else
  constexpr int desc_gate_expiration_ms(5000);
  auto desc_gate = std::make_shared<eCAL::CDescGate>(std::chrono::milliseconds(desc_gate_expiration_ms));

  auto pub_map = desc_gate->GetPublisher();
  EXPECT_EQ(0, pub_map.size());

  eCAL::Registration::Sample reg_sample;
  reg_sample.cmd_type = eCAL::bct_reg_publisher;

  auto& ecal_reg_sample_topic = reg_sample.topic;
  ecal_reg_sample_topic.tname  = "foo";
  ecal_reg_sample_topic.tid    = "12345";

  desc_gate->ApplySample(reg_sample, eCAL::tl_none);

  pub_map = desc_gate->GetPublisher();
  EXPECT_EQ(0, pub_map.size());

#endif
}
