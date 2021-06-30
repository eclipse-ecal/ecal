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
#include "io/ecal_memfile.h"

#include <atomic>
#include <chrono>
#include <memory>

#include <gtest/gtest.h>

namespace eCAL
{
  SMemFileMap* g_memfile_map()
  {
    static std::shared_ptr<SMemFileMap> m(new SMemFileMap);
    return(m.get());
  }
}

TEST(IO, MemfileReadWrite)
{
  eCAL::CMemoryFile mem_file;

  // global parameter
  const std::string memfile_name = "my_memory_file";

  // default send string
  std::string send_s = "Hello World ";
  for (int i = 0; i < 10; i++)
  {
    send_s += send_s;
  }
  size_t slen = send_s.size();

  // check creation state
  EXPECT_EQ(false, mem_file.IsCreated());

  // create memory file
  EXPECT_EQ(true, mem_file.Create(memfile_name.c_str(), true, slen));

  // check creation state
  EXPECT_EQ(true, mem_file.IsCreated());

  // check open state
  //EXPECT_EQ(false, mem_file.IsOpened());

  // write content to memory file before open
  EXPECT_EQ(0, mem_file.Write((void*)send_s.c_str(), slen, 0));

  // open memory file with timeout 100 ms
  EXPECT_EQ(true, mem_file.GetFullAccess(100));

  // check open state
  //EXPECT_EQ(true, mem_file.IsOpened());

  // write content to memory file
  EXPECT_EQ(slen, mem_file.Write((void*)send_s.c_str(), slen, 0));

  // write double sized content to memory file
  std::string double_send_s = send_s + send_s;
  EXPECT_EQ(0, mem_file.Write((void*)double_send_s.c_str(), double_send_s.size(), 0));

  // close memory file
  EXPECT_EQ(true, mem_file.ReleaseFullAccess());

  // check open state
  //EXPECT_EQ(false, mem_file.IsOpened());

  // write content to closed memory file
  EXPECT_EQ(0, mem_file.Write((void*)send_s.c_str(), slen, 0));

  // open memory file with timeout 100 ms
  EXPECT_EQ(true, mem_file.GetReadAccess(100));

  // read content from memory file
  std::vector<char> read_buf;
  read_buf.resize(slen);
  EXPECT_EQ(slen, mem_file.Read((void*)read_buf.data(), read_buf.size(), 0));

  // close memory file
  EXPECT_EQ(true, mem_file.ReleaseReadAccess());

  // destroy memory file
  EXPECT_EQ(true, mem_file.Destroy(true));
}

TEST(IO, MemfilePerf)
{
  eCAL::CMemoryFile mem_file;

  // global parameter
  const std::string memfile_name = "my_memory_file";

  // default send string
  std::string send_s = "Hello World ";
  for (int i = 0; i < 10; i++)
  {
    send_s += send_s;
  }
  size_t slen = send_s.size();

  // create memory file
  EXPECT_EQ(true, mem_file.Create(memfile_name.c_str(), true, slen));

  // open memory file with timeout 100 ms
  EXPECT_EQ(true, mem_file.GetFullAccess(100));

  // start time
  auto start = std::chrono::high_resolution_clock::now();

  // do work
  std::vector<char> read_buf;
  read_buf.resize(slen);
  const size_t write_loops(1000000);
  for (size_t loops = 0; loops < write_loops; ++loops)
  {
    // write content to memory file
    EXPECT_EQ(slen, mem_file.Write((void*)send_s.c_str(), slen, 0));

    // read content from memory file
    EXPECT_EQ(slen, mem_file.Read((void*)read_buf.data(), read_buf.size(), 0));
  }

  // end time
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Elapsed time : " << elapsed.count() << " s" << std::endl;

  size_t sum_snd_bytes = send_s.size() * write_loops;
  std::cout << "Sent         : " << sum_snd_bytes << " bytes";
  std::cout << " (" << sum_snd_bytes / (1024 * 1024) << " MB)" << std::endl;
  std::cout << "Throughput   : " << int((sum_snd_bytes / (1024.0 * 1024.0)) / elapsed.count()) << " MB/s " << std::endl;

  // close memory file
  EXPECT_EQ(true, mem_file.ReleaseFullAccess());

  // destroy memory file
  EXPECT_EQ(true, mem_file.Destroy(true));
}
