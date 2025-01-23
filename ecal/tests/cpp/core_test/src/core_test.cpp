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

#include <ecal/core.h>
#include <ecal/os.h>
#include <ecal/process.h>
#include <ecal/defs.h>

#include <ecal_utils/filesystem.h>

#include <gtest/gtest.h>
#include <string>

TEST(core_cpp_core, GetVersion)
{
  // get eCAL version string
  EXPECT_STREQ(ECAL_VERSION, eCAL::GetVersionString().c_str());

  // get eCAL version date
  EXPECT_STREQ(ECAL_DATE, eCAL::GetVersionDateString().c_str());

  // get eCAL version as separated integer values
  EXPECT_EQ(ECAL_VERSION_MAJOR, eCAL::GetVersion().major);
  EXPECT_EQ(ECAL_VERSION_MINOR, eCAL::GetVersion().minor);
  EXPECT_EQ(ECAL_VERSION_PATCH, eCAL::GetVersion().patch);
}

TEST(core_cpp_core, InitializeFinalize)
{
  // Is eCAL API initialized ?
  EXPECT_EQ(false, eCAL::IsInitialized());

  // initialize eCAL API
  EXPECT_EQ(true, eCAL::Initialize("initialize_test"));

  // Is eCAL API initialized ?
  EXPECT_EQ(true, eCAL::IsInitialized());

  // initialize eCAL API again we expect return value 1 for yet initialized
  EXPECT_EQ(false, eCAL::Initialize("initialize_test"));

  // finalize eCAL API we expect return value 0 even it will not be really finalized because it's 2 times initialzed and 1 time finalized
  EXPECT_EQ(true, eCAL::Finalize());

  // Is eCAL API initialized ? yes it' still initialized
  EXPECT_EQ(true, eCAL::IsInitialized());

  // finalize eCAL API we expect return value 0 because now it will be finalized
  EXPECT_EQ(true, eCAL::Finalize());

  // Is eCAL API initialized ? no
  EXPECT_EQ(false, eCAL::IsInitialized());

  // finalize eCAL API we expect return value 1 because it was finalized before
  EXPECT_EQ(false, eCAL::Finalize());
}

TEST(core_cpp_core, MultipleInitializeFinalize)
{
  // try to initialize / finalize multiple times
  for (auto i = 0; i < 4; ++i)
  {
    // initialize eCAL API
    EXPECT_EQ(true, eCAL::Initialize("multiple_initialize_finalize_test"));

    // finalize eCAL API
    EXPECT_EQ(true, eCAL::Finalize());
  }
}

TEST(core_cpp_core, GetUnitName)
{
  // initialize eCAL API with empty unit name (eCAL will use process name as unit name)
  EXPECT_EQ(true, eCAL::Initialize(""));

  // Is eCAL API initialized ?
  EXPECT_EQ(true, eCAL::IsInitialized());

  // if we call eCAL_Initialize with empty unit name, eCAL will use the process name as unit name
  std::string process_name = EcalUtils::Filesystem::BaseName(eCAL::Process::GetProcessName());
  EXPECT_STREQ(process_name.c_str(), eCAL::Process::GetUnitName().c_str());

  // finalize eCAL API we expect return value 0 because it will be finalized
  EXPECT_EQ(true, eCAL::Finalize());
}

TEST(core_cpp_core, eCAL_Ok)
{
  // check uninitialized eCAL, should not be okay
  EXPECT_EQ(false, eCAL::Ok());

  // initialize eCAL API
  EXPECT_EQ(true, eCAL::Initialize("okay_test"));

  // check initialized eCAL, should be okay
  EXPECT_EQ(true, eCAL::Ok());

  // finalize eCAL API we expect return value 0 because it will be finalized
  EXPECT_EQ(true, eCAL::Finalize());

  // check finalized eCAL, should not be okay
  EXPECT_EQ(false, eCAL::Ok());
}

/* excluded for now, system timer jitter too high */
#if 0
namespace
{
  // timer callback function
  std::atomic_size_t     g_callback_received{ 0 };
  std::vector<long long> g_timer_vec(100);
  void OnTimer()
  {
    if (g_callback_received < g_timer_vec.size()) g_timer_vec[g_callback_received] = eCAL::Time::GetMicroSeconds();
    g_callback_received += 1;
  }
}

TEST(Core, TimerCallback)
{ 
  // initialize eCAL API
  EXPECT_EQ(0, eCAL::Initialize("timer callback"));

  // Is eCAL API initialized ?
  EXPECT_EQ(1, eCAL::IsInitialized());

  std::this_thread::sleep_for(std::chrono::seconds(1));

  eCAL::CTimer timer;
  const int tloop(10);
  timer.Start(tloop, &OnTimer);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Expect that the timer callback was called a hundred times +/-1
  EXPECT_TRUE((g_callback_received >= 99) && (g_callback_received <= 101));
  std::cout << std::endl;
  std::cout << "Timer called " << g_callback_received << " times." << std::endl;
  std::cout << std::endl;

  // Calculate loop jitter
  std::vector<long long> timer_vec_diff;
  long long last_time(0);
  for (auto time : g_timer_vec)
  {
    if (last_time) timer_vec_diff.push_back(time - last_time);
    last_time = time;
  }

  // Print out loop times
  //for (auto time : timer_vec_diff) std::cout << static_cast<double>(time) / 1000.0 << " ms" << std::endl;

  // Print out min / max loop times
  auto tmin = *min_element(std::begin(timer_vec_diff), std::end(timer_vec_diff));
  auto tmax = *max_element(std::begin(timer_vec_diff), std::end(timer_vec_diff));
  std::cout << "Min loop time [ms]: " << tmin / 1000.0 << std::endl;
  std::cout << "Max loop time [ms]: " << tmax / 1000.0 << std::endl;
  std::cout << std::endl;

  // check precision loop time +/- 2 ms
  const int max_jitter_ms(2);
  EXPECT_TRUE(tmin > (tloop - max_jitter_ms) * 1000);
  EXPECT_TRUE(tmax < (tloop + max_jitter_ms) * 1000);

  // this is an issue !
  // we need to stop timer before we finalize eCAL API
  // otherwise it will hang forever
  timer.Stop();

  // finalize second time eCAL API we still expect 0 because 
  // but now reference counter is 0 and destruction should be succeeded
  EXPECT_EQ(0, eCAL::Finalize());

  // finalize eCAL API again we expect 1 because yet finalized
  EXPECT_EQ(1, eCAL::Finalize());
}
#endif
