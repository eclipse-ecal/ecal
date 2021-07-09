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
#include <iostream>
#include <thread>

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
  EXPECT_EQ(false, mem_file.IsOpened());

  // write content to memory file before open
  EXPECT_EQ(0, mem_file.Write((void*)send_s.c_str(), slen, 0));

  // open memory file with full access (timeout 100 ms)
  EXPECT_EQ(true, mem_file.GetFullAccess(100));

  // check open state
  EXPECT_EQ(true, mem_file.IsOpened());

  // check full access state
  EXPECT_EQ(true, mem_file.HasFullAccess());

  // check read only access state
  EXPECT_EQ(false, mem_file.HasReadOnlyAccess());

  // write content to memory file
  EXPECT_EQ(slen, mem_file.Write((void*)send_s.c_str(), slen, 0));

  // write double sized content to memory file
  std::string double_send_s = send_s + send_s;
  EXPECT_EQ(0, mem_file.Write((void*)double_send_s.c_str(), double_send_s.size(), 0));

  // close memory file
  EXPECT_EQ(true, mem_file.ReleaseFullAccess());

  // check open state
  EXPECT_EQ(false, mem_file.IsOpened());

  // check full access state
  EXPECT_EQ(false, mem_file.HasFullAccess());

  // check read only access state
  EXPECT_EQ(false, mem_file.HasReadOnlyAccess());

  // write content to closed memory file
  EXPECT_EQ(0, mem_file.Write((void*)send_s.c_str(), slen, 0));

  // open memory file with timeout 100 ms
  EXPECT_EQ(true, mem_file.GetReadAccess(100));

  // check read only access state
  EXPECT_EQ(true, mem_file.HasReadOnlyAccess());

  // check full access state
  EXPECT_EQ(false, mem_file.HasFullAccess());

  // read content from memory file
  std::vector<char> read_buf;
  read_buf.resize(slen);
  EXPECT_EQ(slen, mem_file.Read((void*)read_buf.data(), read_buf.size(), 0));

  // close memory file
  EXPECT_EQ(true, mem_file.ReleaseReadAccess());

  // check read only access state
  EXPECT_EQ(false, mem_file.HasReadOnlyAccess());

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

TEST(IO, MemfileConcurrency)
{
  eCAL::CMemoryFile mem_file;

  // global parameter
  const std::string memfile_name = "my_memory_file";

  // buffer length && runs
  const size_t buflen(1024);
  const size_t runs(10);

  // create memory file
  EXPECT_EQ(true, mem_file.Create(memfile_name.c_str(), true, buflen));

  // producer thread
  auto num_writes(0);
  std::thread producer([&]()
    {
      std::vector<int> write_buf;
      write_buf.resize(buflen);

      for (int i = 0; i != runs; ++i)
      {
        EXPECT_EQ(true, mem_file.GetFullAccess(100));
        if (mem_file.HasFullAccess())
        {
          write_buf[0] = num_writes;
          auto written = mem_file.Write((void*)write_buf.data(), write_buf.size(), 0);
          EXPECT_EQ(buflen, written);
          std::cout << std::endl;
          std::cout << "producer write access  : " << num_writes << std::endl;
          EXPECT_EQ(true, mem_file.ReleaseFullAccess());
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          num_writes++;
        }
      }
    });
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  // consumer 1 thread
  auto num_reads1(0);
  std::thread consumer1([&]()
    {
      std::vector<int> read_buf;
      read_buf.resize(buflen);

      for (int i = 0; i != runs; ++i)
      {
        EXPECT_EQ(true, mem_file.GetReadAccess(100));
        if (mem_file.HasReadOnlyAccess())
        {
          auto read = mem_file.Read((void*)read_buf.data(), read_buf.size(), 0);
          EXPECT_EQ(buflen, read);
          std::cout << "consumer 1 read access : " << num_reads1 << " with " << read_buf[0] << std::endl;
          EXPECT_EQ(true, mem_file.ReleaseReadAccess());
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          //EXPECT_EQ(read_buf[0], num_reads1);
          num_reads1++;
        }
      }
    });
  std::this_thread::sleep_for(std::chrono::milliseconds(1));

  // consumer 2 thread
  auto num_reads2(0);
  std::thread consumer2([&]()
    {
      std::vector<int> read_buf;
      read_buf.resize(buflen);

      for (int i = 0; i != runs; ++i)
      {
        EXPECT_EQ(true, mem_file.GetReadAccess(100));
        if (mem_file.HasReadOnlyAccess())
        {
          auto read = mem_file.Read((void*)read_buf.data(), read_buf.size(), 0);
          EXPECT_EQ(buflen, read);
          std::cout << "consumer 2 read access : " << num_reads2 << " with " << read_buf[0] << std::endl;
          EXPECT_EQ(true, mem_file.ReleaseReadAccess());
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          //EXPECT_EQ(read_buf[0], num_reads2);
          num_reads2++;
        }
      }
    });

  // join threads
  producer.join();
  consumer1.join();
  consumer2.join();

  EXPECT_EQ(num_writes, num_reads1);
  EXPECT_EQ(num_writes, num_reads2);
}
