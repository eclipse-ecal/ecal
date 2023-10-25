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

#include <gtest/gtest.h>

TEST(Event, EventSetGet)
{ 
  // global parameter
  const std::string event_name = "my_event";

  // create named event
  eCAL::EventHandleT event_handle;
  EXPECT_EQ(true, eCAL::gOpenEvent(&event_handle, event_name));

  // get none set event
  EXPECT_EQ(false, gWaitForEvent(event_handle, 10));

  // set event
  EXPECT_EQ(true, gSetEvent(event_handle));

  // get set event
  EXPECT_EQ(true, gWaitForEvent(event_handle, 100));
}
