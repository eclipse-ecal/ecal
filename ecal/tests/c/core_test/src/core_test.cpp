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

#include <ecal/cimpl/ecal_core_cimpl.h>
#include <ecal/cimpl/ecal_process_cimpl.h>
#include <ecal/ecal_defs.h>

#include <cstring>

#include <gtest/gtest.h>

TEST(core_c_core, GetVersion)
{
  // get eCAL version string
  EXPECT_STREQ(ECAL_VERSION, eCAL_GetVersionString());

  // get eCAL version date
  EXPECT_STREQ(ECAL_DATE, eCAL_GetVersionDateString());

  // get eCAL version as separated integer values
  int major = -1;
  int minor = -1;
  int patch = -1;
  eCAL_GetVersion(&major, &minor, &patch);
  EXPECT_EQ(ECAL_VERSION_MAJOR, major);
  EXPECT_EQ(ECAL_VERSION_MINOR, minor);
  EXPECT_EQ(ECAL_VERSION_PATCH, patch);
}

TEST(core_c_core, InitializeFinalize)
{
  // Is eCAL API initialized ?
  EXPECT_EQ(0, eCAL_IsInitialized(0));

  // initialize eCAL API
  EXPECT_EQ(0, eCAL_Initialize(0, nullptr, "initialize_test", 0));

  // Is eCAL API initialized ?
  EXPECT_EQ(1, eCAL_IsInitialized(0));

  // initialize eCAL API again we expect return value 1 for yet initialized
  EXPECT_EQ(1, eCAL_Initialize(0, nullptr, "initialize_test", 0));

  // finalize eCAL API we expect return value 0 even it will not be really finalized because it's 2 times initialzed and 1 time finalized
  EXPECT_EQ(0, eCAL_Finalize(0));

  // Is eCAL API initialized ? yes it' still initialized
  EXPECT_EQ(1, eCAL_IsInitialized(0));

  // finalize eCAL API we expect return value 0 because now it will be finalized
  EXPECT_EQ(0, eCAL_Finalize(0));

  // Is eCAL API initialized ? no
  EXPECT_EQ(0, eCAL_IsInitialized(0));

  // finalize eCAL API we expect return value 1 because it was finalized before
  EXPECT_EQ(1, eCAL_Finalize(0));
}

TEST(core_c_core, MultipleInitializeFinalize)
{
  // try to initialize / finalize multiple times
  for (auto i = 0; i < 4; ++i)
  {
    // initialize eCAL API
    EXPECT_EQ(0, eCAL_Initialize(0, nullptr, "multiple_initialize_finalize_test", 0));

    // finalize eCAL API
    EXPECT_EQ(0, eCAL_Finalize(0));
  }
}

namespace
{
  std::string extractProcessName(const std::string& full_path_)
  {
    // initialize process name with full path
    std::string processName = full_path_;

    // extract the substring after the last separator
    size_t lastSeparatorPos = full_path_.find_last_of("\\/");
    if (lastSeparatorPos != std::string::npos)
    {
      processName = full_path_.substr(lastSeparatorPos + 1);
    }

    // remove the file extension if found
    size_t lastDotPos = processName.find_last_of('.');
    if (lastDotPos != std::string::npos)
    {
      processName = processName.substr(0, lastDotPos);
    }

    return processName;
  }
}

TEST(core_c_core, SetGetUnitName)
{
  // initialize eCAL API with empty unit name (eCAL will use process name as unit name)
  EXPECT_EQ(0, eCAL_Initialize(0, nullptr, "", 0));

  // Is eCAL API initialized ?
  EXPECT_EQ(1, eCAL_IsInitialized(0));

  // if we call eCAL_Initialize with empty unit name, eCAL will use the process name as unit name
  char process_name[1024] = { 0 };
  eCAL_Process_GetProcessName(process_name, sizeof(process_name));
  std::string process_name_s = extractProcessName(process_name);
  char unit_name[1024] = { 0 };
  eCAL_Process_GetUnitName(unit_name, sizeof(unit_name));
  EXPECT_STREQ(process_name_s.c_str(), unit_name);

  // set unit name (should change the name to 'unit name')
  EXPECT_EQ(0, eCAL_SetUnitName("unit name"));
  memset(unit_name, 0, sizeof(unit_name));
  eCAL_Process_GetUnitName(unit_name, sizeof(unit_name));
  EXPECT_STREQ("unit name", unit_name);

  // set nullptr unit name (should not change the unit name)
  EXPECT_EQ(-1, eCAL_SetUnitName(nullptr));
  memset(unit_name, 0, sizeof(unit_name));
  eCAL_Process_GetUnitName(unit_name, sizeof(unit_name));
  EXPECT_STREQ("unit name", unit_name);

  // set empty unit name (should not change the unit name)
  EXPECT_EQ(-1, eCAL_SetUnitName(""));
  memset(unit_name, 0, sizeof(unit_name));
  eCAL_Process_GetUnitName(unit_name, sizeof(unit_name));
  EXPECT_STREQ("unit name", unit_name);

  // finalize eCAL API we expect return value 0 because it will be finalized
  EXPECT_EQ(0, eCAL_Finalize(0));
}

TEST(core_c_core, eCAL_Ok)
{
  // check uninitialized eCAL, should not be okay
  EXPECT_EQ(0, eCAL_Ok());

  // initialize eCAL API
  EXPECT_EQ(0, eCAL_Initialize(0, nullptr, "okay_test", 0));

  // check initialized eCAL, should be okay
  EXPECT_EQ(1, eCAL_Ok());

  // finalize eCAL API we expect return value 0 because it will be finalized
  EXPECT_EQ(0, eCAL_Finalize(0));

  // check finalized eCAL, should not be okay
  EXPECT_EQ(0, eCAL_Ok());
}
