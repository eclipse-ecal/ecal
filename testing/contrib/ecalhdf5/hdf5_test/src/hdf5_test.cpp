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

#include <ecalhdf5/eh5_meas.h>

#include <chrono>
#include <fstream>
#include <thread>

#include <gtest/gtest.h>

#include <ecalhdf5/../../src/escape.h> // This header file is usually not available as public include!

#define TEST_RAW_PERF      1
#define TEST_HDF5_PERF     1

#define USE_OFSTREAM       0
#define TEST_POST_SLEEP    2               // seconds

#define TEST_SIZE_1        1
#define DATA_SET_SIZE_1    (1024)          // 1 kB
#define DATA_SET_NUMBER_1  (64*1024)

#define TEST_SIZE_2        1
#define DATA_SET_SIZE_2    (128*1024)      // 128 kB
#define DATA_SET_NUMBER_2  (8*1024)

#define TEST_SIZE_3        1
#define DATA_SET_SIZE_3    (1024*1024)     // 1 MB
#define DATA_SET_NUMBER_3  (1024)

#define TEST_SIZE_4        1
#define DATA_SET_SIZE_4    (4096*1024)     // 4 MB
#define DATA_SET_NUMBER_4  (256)

namespace
{
  // properties
  std::string output_dir         = "measurement_dir";
  const size_t max_size_per_file = 500;  // MB
  std::vector<char> data;
}

TEST(HDF5, EscapeUnescape)
{
  std::string test_string             = "This string contains commata,slashes/ spaces, a percent sign (%), a bell\a, nasty line endings\r\n and a german letter oe from the Latin-1 codepage: \xF8";
  std::string expected_escaped_string = "This string contains commata%2Cslashes%2F spaces%2C a percent sign (%25)%2C a bell%07%2C nasty line endings%0D%0A and a german letter oe from the Latin-1 codepage: %F8";

  std::string escaped_string = eCAL::eh5::GetEscapedString(test_string);

  EXPECT_EQ(escaped_string, expected_escaped_string);
  EXPECT_EQ(eCAL::eh5::GetUnescapedString(escaped_string), test_string);

  std::string faulty_escaped_string     = "This%20String%20has some escapes, a % that is not escaped / one at the end %";
  std::string expected_unescaped_string = "This String has some escapes, a % that is not escaped / one at the end %";
  EXPECT_EQ(eCAL::eh5::GetUnescapedString(faulty_escaped_string), expected_unescaped_string);
}

TEST(HDF5, WriteReadIntegrity)
{
  // Define data that will be written to the file

  const std::string t1_name          = "topic / with / slash";
  const std::string t1_data          = "Hello World";
  const long long   t1_snd_timestamp = 1001LL;
  const long long   t1_rcv_timestamp = 2001LL;
  const long long   t1_id            = 1LL;
  const long long   t1_clock         = 11LL;

  const std::string t2_name          = "another,topic";
  const std::string t2_data          = "Data of topic 2";
  const long long   t2_snd_timestamp = 1002LL;
  const long long   t2_rcv_timestamp = 2002LL;
  const long long   t2_id            = 2LL;
  const long long   t2_clock         = 12LL;

  const std::string t3_name          = " ASCII and beyond!\a\b\t\n\v\f\r\"#$%&\'()*+,-./0123456789:;<=>\?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~¸ˆ‰‹÷ƒ‚¬Ù‘˚€·‡¡¿˙Ÿ";
  const std::string t3_data          = "o.O";
  const long long   t3_snd_timestamp = 1003LL;
  const long long   t3_rcv_timestamp = 2003LL;
  const long long   t3_id            = 3LL;
  const long long   t3_clock         = 13LL;

  std::string base_name = "integrity_meas";

  // Write HDF5 file
  {
    eCAL::eh5::HDF5Meas hdf5_writer;


    if (hdf5_writer.Open(output_dir, eCAL::eh5::eAccessType::CREATE))
    {
      hdf5_writer.SetFileBaseName(base_name);
      hdf5_writer.SetMaxSizePerFile(max_size_per_file);
    }
    else
    {
      FAIL() << "Failed to open HDF5 Writer";
    }

    EXPECT_TRUE(hdf5_writer.AddEntryToFile(
      t1_data.data(),           // data
      t1_data.size(),           // data size
      t1_snd_timestamp,         // snd_timestamp
      t1_rcv_timestamp,         // rcv_timestamp
      t1_name,                  // channel name
      t1_id,                    // id
      t1_clock                  // clock
    ));

    EXPECT_TRUE(hdf5_writer.AddEntryToFile(
      t2_data.data(),           // data
      t2_data.size(),           // data size
      t2_snd_timestamp,         // snd_timestamp
      t2_rcv_timestamp,         // rcv_timestamp
      t2_name,                  // channel name
      t2_id,                    // id
      t2_clock                  // clock
    ));

    EXPECT_TRUE(hdf5_writer.AddEntryToFile(
      t3_data.data(),           // data
      t3_data.size(),           // data size
      t3_snd_timestamp,         // snd_timestamp
      t3_rcv_timestamp,         // rcv_timestamp
      t3_name,                  // channel name
      t3_id,                    // id
      t3_clock                  // clock
    ));

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read HDF4 file
  {
    eCAL::eh5::HDF5Meas hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(output_dir + "/" + base_name + ".hdf5"));

    std::set<std::string> expected_channel_names { t1_name, t2_name, t3_name };
    EXPECT_EQ(hdf5_reader.GetChannelNames(), expected_channel_names);

    eCAL::eh5::EntryInfoSet entries_info_set_t1;
    eCAL::eh5::EntryInfoSet entries_info_set_t2;
    eCAL::eh5::EntryInfoSet entries_info_set_t3;

    EXPECT_TRUE(hdf5_reader.GetEntriesInfo(t1_name, entries_info_set_t1));
    EXPECT_TRUE(hdf5_reader.GetEntriesInfo(t2_name, entries_info_set_t2));
    EXPECT_TRUE(hdf5_reader.GetEntriesInfo(t3_name, entries_info_set_t3));

    EXPECT_EQ(entries_info_set_t1.size(), 1);
    EXPECT_EQ(entries_info_set_t2.size(), 1);
    EXPECT_EQ(entries_info_set_t3.size(), 1);

    EXPECT_EQ(entries_info_set_t1.begin()->SndTimestamp, t1_snd_timestamp);
    EXPECT_EQ(entries_info_set_t1.begin()->RcvTimestamp, t1_rcv_timestamp);
    EXPECT_EQ(entries_info_set_t1.begin()->SndID,        t1_id);
    EXPECT_EQ(entries_info_set_t1.begin()->SndClock,     t1_clock);

    EXPECT_EQ(entries_info_set_t2.begin()->SndTimestamp, t2_snd_timestamp);
    EXPECT_EQ(entries_info_set_t2.begin()->RcvTimestamp, t2_rcv_timestamp);
    EXPECT_EQ(entries_info_set_t2.begin()->SndID,        t2_id);
    EXPECT_EQ(entries_info_set_t2.begin()->SndClock,     t2_clock);

    EXPECT_EQ(entries_info_set_t3.begin()->SndTimestamp, t3_snd_timestamp);
    EXPECT_EQ(entries_info_set_t3.begin()->RcvTimestamp, t3_rcv_timestamp);
    EXPECT_EQ(entries_info_set_t3.begin()->SndID,        t3_id);
    EXPECT_EQ(entries_info_set_t3.begin()->SndClock,     t3_clock);

    size_t t1_data_size;
    size_t t2_data_size;
    size_t t3_data_size;

    EXPECT_TRUE(hdf5_reader.GetEntryDataSize(entries_info_set_t1.begin()->ID, t1_data_size));
    EXPECT_TRUE(hdf5_reader.GetEntryDataSize(entries_info_set_t2.begin()->ID, t2_data_size));
    EXPECT_TRUE(hdf5_reader.GetEntryDataSize(entries_info_set_t3.begin()->ID, t3_data_size));

    std::string t1_data_read(t1_data_size, ' ');
    std::string t2_data_read(t2_data_size, ' ');
    std::string t3_data_read(t3_data_size, ' ');

    EXPECT_TRUE(hdf5_reader.GetEntryData(entries_info_set_t1.begin()->ID, const_cast<char*>(t1_data_read.data())));
    EXPECT_TRUE(hdf5_reader.GetEntryData(entries_info_set_t2.begin()->ID, const_cast<char*>(t2_data_read.data())));
    EXPECT_TRUE(hdf5_reader.GetEntryData(entries_info_set_t3.begin()->ID, const_cast<char*>(t3_data_read.data())));

    EXPECT_EQ(t1_data_read, t1_data);
    EXPECT_EQ(t2_data_read, t2_data);
    EXPECT_EQ(t3_data_read, t3_data);
  }
}

TEST(HDF5, ReadWrite)
{
  std::string file_name = "meas_readwrite";

  // Create writer instance
  eCAL::eh5::HDF5Meas writer(output_dir, eCAL::eh5::CREATE);
  writer.SetFileBaseName(file_name);
  writer.SetMaxSizePerFile(max_size_per_file);

  data.resize(DATA_SET_SIZE_1);
  EXPECT_TRUE(writer.AddEntryToFile(static_cast<void*>(data.data()), data.size(), 0, 0, "myChannel", 0, 0));

  EXPECT_TRUE(writer.Close());
}

void MeasPerf(const std::string& file_name, const size_t pkg_size, const size_t pkg_num)
{
  data.resize(pkg_size);
  const size_t write_loops(pkg_num);

#if TEST_RAW_PERF
  // Test raw performance
  {
    // start time
    auto start = std::chrono::high_resolution_clock::now();

#if USE_OFSTREAM

    std::ofstream rfile(file_name + "_raw", std::ios::trunc | std::ios::out | std::ios::binary);
    if (rfile)
    {
      for (size_t loops = 0; loops < write_loops; ++loops)
      {
        rfile.write(data.data(), data.size());
      }
    }
    rfile.close();

#else // USE_OFSTREAM

    FILE* pFile(nullptr);
    std::string fname = file_name + "_raw";
    pFile = fopen(fname.c_str(), "wb");
    if (pFile)
    {
      for (size_t loops = 0; loops < write_loops; ++loops)
      {
        fwrite(data.data(), 1, data.size(), pFile);
      }
      fclose(pFile);
    }

#endif // USE_OFSTREAM

    // end time
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    size_t sum_data = data.size() * write_loops;
    std::cout << std::endl;
    std::cout << "Packages number : " << write_loops << std::endl;
    std::cout << "Packages size   : " << data.size()/1024 << " kB" << std::endl;
    std::cout << "Sum payload     : " << sum_data / (1024*1024) << " MB" << std::endl;
    std::cout << "Throughput RAW  : " << int((sum_data / (1024.0 * 1024.0)) / elapsed.count()) << " MB/s " << std::endl;
    std::cout << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(TEST_POST_SLEEP));
  }
#endif

#if TEST_HDF5_PERF
  // Test HDF5 performance
  {
    // start time
    auto start = std::chrono::high_resolution_clock::now();

    eCAL::eh5::HDF5Meas writer(output_dir, eCAL::eh5::CREATE);
    writer.SetFileBaseName(file_name + "_hdf5");
    writer.SetMaxSizePerFile(max_size_per_file);
    for (size_t loop = 0; loop < write_loops; ++loop)
    {
      EXPECT_TRUE(writer.AddEntryToFile(static_cast<void*>(data.data()), data.size(), 0, 0, "myChannel", 0, loop));
    }
    EXPECT_TRUE(writer.Close());

    // end time
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    size_t sum_data = data.size() * write_loops;
    std::cout << std::endl;
    std::cout << "Packages number : " << write_loops << std::endl;
    std::cout << "Packages size   : " << data.size() / 1024 << " kB" << std::endl;
    std::cout << "Sum payload     : " << sum_data / (1024 * 1024) << " MB" << std::endl;
    std::cout << "Throughput HDF5 : " << int((sum_data / (1024.0 * 1024.0)) / elapsed.count()) << " MB/s " << std::endl;
    std::cout << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(TEST_POST_SLEEP));
  }
#endif
}

#if TEST_SIZE_1
TEST(HDF5, Performance_1kb)
{
  MeasPerf("meas_1_pkg",    DATA_SET_SIZE_1, DATA_SET_NUMBER_1);
}
#endif // TEST_SIZE_1

#if TEST_SIZE_2
TEST(HDF5, Performance_128kb)
{
  MeasPerf("meas_128_pkg",  DATA_SET_SIZE_2, DATA_SET_NUMBER_2);
}
#endif // TEST_SIZE_2

#if TEST_SIZE_3
TEST(HDF5, Performance_1024kb)
{
  MeasPerf("meas_1024_pkg", DATA_SET_SIZE_3, DATA_SET_NUMBER_3);
}
#endif // TEST_SIZE_3

#if TEST_SIZE_4
TEST(HDF5, Performance_4096kb)
{
  MeasPerf("meas_4096_pkg", DATA_SET_SIZE_4, DATA_SET_NUMBER_4);
}
#endif // TEST_SIZE_4
