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
#include <thread>
#include <set>
#include <algorithm>

#include <gtest/gtest.h>

#include <ecalhdf5/../../src/escape.h> // This header file is usually not available as public include!

using namespace eCAL::experimental::measurement::base;

namespace eCAL 
{
  namespace experimental
  {
    namespace measurement
    {
      namespace base
      {
        void PrintTo(const DataTypeInformation& info, std::ostream* os) {
          *os << "(" << info.name << "," << info.encoding << "," << info.descriptor << ")";
        }
      }
    }
  }
}


namespace
{
struct TestingMeasEntry
{
  std::string channel_name = "topic";
  std::string data = "Hello World";
  long long   snd_timestamp = 1001LL;
  long long   rcv_timestamp = 2001LL;
  long long   id = 1LL;
  long long   clock = 11LL;
};

bool MeasEntryEqualsEntryInfo(const TestingMeasEntry& meas_entry, const EntryInfo entry_info)
{
  return meas_entry.id == entry_info.SndID
    && meas_entry.snd_timestamp == entry_info.SndTimestamp
    && meas_entry.rcv_timestamp == entry_info.RcvTimestamp
    && meas_entry.clock == entry_info.SndClock;
}

bool WriteToHDF(eCAL::eh5::HDF5Meas& writer, const TestingMeasEntry& entry)
{
  return writer.AddEntryToFile(
    entry.data.data(),   // data
    entry.data.size(),   // data size
    entry.snd_timestamp, // snd_timestamp
    entry.rcv_timestamp, // rcv_timestamp
    entry.channel_name,        // channel name
    entry.id,            // id
    entry.clock          // clock
  );
}


// Validates, that if all entries contained in TestingMeasEntry were added to a measurement, they are later contained in the read measurement.
void ValidateChannelsInMeasurement(eCAL::eh5::HDF5Meas& hdf5_reader, const std::vector<TestingMeasEntry>& entries_in_measurement)
{
  std::set<std::string> expected_channel_names;
  std::transform(entries_in_measurement.begin(), entries_in_measurement.end(), std::inserter(expected_channel_names, expected_channel_names.begin()),
    [](const TestingMeasEntry& entry) { return entry.channel_name; });

  EXPECT_EQ(hdf5_reader.GetChannelNames(), expected_channel_names);
}

// Finds an EntryInfo corresponding to the TestingMeasEntry in the EntryInfoSet
// Return default EntryInfo if it cannot be Found
EntryInfo FindInSet(const eCAL::eh5::EntryInfoSet& info_set, const TestingMeasEntry& to_find)
{
  auto it = std::find_if(info_set.begin(), info_set.end(), [&to_find](const EntryInfo& entry) { return entry.SndClock == to_find.clock; });

  if (it != info_set.end()) {
    return *it;
  }
  else {
    return EntryInfo{};
  }
}

// Validate, that the TestingMeasEntry can be read from the measurement
void ValidateDataInMeasurement(eCAL::eh5::HDF5Meas& hdf5_reader, const TestingMeasEntry& entry)
{
  eCAL::eh5::EntryInfoSet entries_info_set;
  EXPECT_TRUE(hdf5_reader.GetEntriesInfo(entry.channel_name, entries_info_set));

  // Get Entry info from measurement and see that it checks out
  EntryInfo info = FindInSet(entries_info_set, entry);
  // entry was not in the dataset
  if (info == EntryInfo{})
  {
    FAIL() << "Entry info not found in set / measurement";
    return;
  }

  EXPECT_TRUE(MeasEntryEqualsEntryInfo(entry, info));

  size_t data_size;
  EXPECT_TRUE(hdf5_reader.GetEntryDataSize(info.ID, data_size));

  std::string data_read(data_size, ' ');
  EXPECT_TRUE(hdf5_reader.GetEntryData(info.ID, const_cast<char*>(data_read.data())));

  EXPECT_EQ(data_read, entry.data);
}

TestingMeasEntry m1{
 "topic / with / slash",
 "Hello World",
 1001LL,
 2001LL,
 1LL,
 11LL,
};

TestingMeasEntry m2{
  "another,topic",
  "",
  1002LL,
  2002LL,
  2LL,
  12LL
};

TestingMeasEntry m3{
  " ASCII and beyond!\a\b\t\n\v\f\r\"#$%&\'()*+,-./0123456789:,<=>\?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~üöäÜÖÄâÂôÔûÛáàÁÀúÙ",
  "o.O",
  1003LL,
  2003LL,
  3LL,
  13LL
};

TestingMeasEntry topic_1
{
  "topic_1",
  "topic1: test data",
  1001LL,
  2001LL,
  1LL,
  11LL
};

TestingMeasEntry topic_2
{
  "topic_2",
  "topic2: test data",
  1002LL,
  2002LL,
  2LL,
  12LL,
};


  // properties
  std::string output_dir         = "measurement_dir";
  const size_t max_size_per_file = 500;  // MB
  std::vector<char> data;
}

TEST(HDF5, EscapeUnescape)
{
  std::string test_string             = "This string contains commata,slashes/ spaces, a percent sign (%), a bell\a, nasty line endings\r\n and a german letter oe from the Latin-1 codepage: \xF8";
  std::string expected_escaped_string = "This string contains commata%2Cslashes%2F spaces%2C a percent sign (%25)%2C a bell%07%2C nasty line endings%0D%0A and a german letter oe from the Latin-1 codepage: %F8";

  std::string escaped_string = eCAL::eh5::GetEscapedTopicname(test_string);

  EXPECT_EQ(escaped_string, expected_escaped_string);
  EXPECT_EQ(eCAL::eh5::GetUnescapedString(escaped_string), test_string);

  std::string faulty_escaped_string     = "This%20String%20has some escapes, a % that is not escaped / one at the end %";
  std::string expected_unescaped_string = "This String has some escapes, a % that is not escaped / one at the end %";
  EXPECT_EQ(eCAL::eh5::GetUnescapedString(faulty_escaped_string), expected_unescaped_string);
}



TEST(HDF5, WriteReadIntegrity)
{
  // Define data that will be written to the file

  std::string base_name = "integrity_meas";
  std::string meas_root_dir = output_dir + "/" + base_name;

  std::vector<TestingMeasEntry> meas_entries{ m1, m2, m3 };

  // Write HDF5 file
  {
    eCAL::eh5::HDF5Meas hdf5_writer;

    if (hdf5_writer.Open(meas_root_dir, eCAL::eh5::eAccessType::CREATE))
    {
      hdf5_writer.SetFileBaseName(base_name);
      hdf5_writer.SetMaxSizePerFile(max_size_per_file);
    }
    else
    {
      FAIL() << "Failed to open HDF5 Writer";
    }

    for (const auto& entry : meas_entries)
    {
      EXPECT_TRUE(WriteToHDF(hdf5_writer, entry));
    }

    EXPECT_TRUE(hdf5_writer.Close());
  }
    
  // Read HDF5 file
  {
    eCAL::eh5::HDF5Meas hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir + "/" + base_name + ".hdf5"));

    ValidateChannelsInMeasurement(hdf5_reader, meas_entries);
    for (const auto& entry : meas_entries)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
   }

  // Read entries with HDF5 dir API
  {
    eCAL::eh5::HDF5Meas hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));

    ValidateChannelsInMeasurement(hdf5_reader, meas_entries);
    for (const auto& entry : meas_entries)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
  }
}

TEST(HDF5, ReadWrite)
{
  std::string file_name = "meas_readwrite";

  // Create writer instance
  eCAL::eh5::HDF5Meas writer(output_dir, eCAL::eh5::CREATE);
  writer.SetFileBaseName(file_name);
  writer.SetMaxSizePerFile(max_size_per_file);

  data.resize(1024);
  EXPECT_TRUE(writer.AddEntryToFile(static_cast<void*>(data.data()), data.size(), 0, 0, "myChannel", 0, 0));

  EXPECT_TRUE(writer.Close());
}


TEST(HDF5, IsOneFilePerChannelEnabled)
{
  eCAL::eh5::HDF5Meas hdf5_writer;
  std::string base_name = "output";

  if (hdf5_writer.Open(output_dir, eCAL::eh5::eAccessType::CREATE))
  {
    hdf5_writer.SetFileBaseName(base_name);
    hdf5_writer.SetMaxSizePerFile(max_size_per_file);
    hdf5_writer.SetOneFilePerChannelEnabled(true);
  }
  else
  {
    FAIL() << "Failed to open HDF5 Writer";
  }

  EXPECT_TRUE(hdf5_writer.IsOneFilePerChannelEnabled());

  hdf5_writer.SetOneFilePerChannelEnabled(false);
  EXPECT_TRUE(!hdf5_writer.IsOneFilePerChannelEnabled());
}

TEST(HDF5, SetOneFilePerChannelEnabled)
{
  std::string base_name = "output";
  
  // Write HDF5 file
  {
    eCAL::eh5::HDF5Meas hdf5_writer;

    if (hdf5_writer.Open(output_dir, eCAL::eh5::eAccessType::CREATE))
    {
      hdf5_writer.SetFileBaseName(base_name);
      hdf5_writer.SetMaxSizePerFile(max_size_per_file);
      hdf5_writer.SetOneFilePerChannelEnabled(true);
    }
    else
    {
      FAIL() << "Failed to open HDF5 Writer";
    }

    EXPECT_TRUE(WriteToHDF(hdf5_writer, topic_1));
    EXPECT_TRUE(WriteToHDF(hdf5_writer, topic_2));

    EXPECT_TRUE(hdf5_writer.IsOneFilePerChannelEnabled());

    EXPECT_TRUE(hdf5_writer.Close());
  }

  //  Read HDF5 topic 1 file
  {
    eCAL::eh5::HDF5Meas hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(output_dir + "/" + base_name + "_" + topic_1.channel_name + ".hdf5"));
    ValidateChannelsInMeasurement(hdf5_reader, { topic_1 });
    ValidateDataInMeasurement(hdf5_reader, topic_1);
  }

  //  Read HDF5 topic 2 file
  {
    eCAL::eh5::HDF5Meas hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(output_dir + "/" + base_name + "_" + topic_2.channel_name + ".hdf5"));

    ValidateChannelsInMeasurement(hdf5_reader, { topic_2 });
    ValidateDataInMeasurement(hdf5_reader, topic_2);
  }

}

TEST(HDF5, EscapeFilenamesForOneFilePerChannel)
{
  // Define data that will be written to the file
  TestingMeasEntry normal_ascii = topic_1;
  normal_ascii.channel_name = "NormalAscii";
  const std::string normal_ascii_escaped_filename = "NormalAscii";

  TestingMeasEntry escape_ascii = topic_1;
  escape_ascii.channel_name = "Escape these: \\/*\"<>|%";
  const std::string escape_ascii_escaped_filename = "Escape these%3A %5C%2F%2A%22%3C%3E%7C%25";

  TestingMeasEntry escape_ascii_2 = topic_1;
  escape_ascii_2.channel_name = "Don't escape these +~#,;.²³🍺";
  const std::string escape_ascii_2_escaped_filename = "Don't escape these +~#,;.²³\xF0\x9F\x8D\xBA";

  std::string base_name = "escape_filename_meas";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    eCAL::eh5::HDF5Meas hdf5_writer;

    if (hdf5_writer.Open(meas_root_dir, eCAL::eh5::eAccessType::CREATE))
    {
      hdf5_writer.SetFileBaseName(base_name);
      hdf5_writer.SetMaxSizePerFile(max_size_per_file);
      hdf5_writer.SetOneFilePerChannelEnabled(true);
    }
    else
    {
      FAIL() << "Failed to open HDF5 Writer";
    }

    EXPECT_TRUE(WriteToHDF(hdf5_writer, normal_ascii));
    EXPECT_TRUE(WriteToHDF(hdf5_writer, escape_ascii));
    EXPECT_TRUE(WriteToHDF(hdf5_writer, escape_ascii_2));

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read File 1
  {
    eCAL::eh5::HDF5Meas hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir + "/" + base_name + "_" + normal_ascii_escaped_filename + ".hdf5"));
    ValidateDataInMeasurement(hdf5_reader, normal_ascii);
  }

  // Read File 2
  {
    eCAL::eh5::HDF5Meas hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir + "/" + base_name + "_" + escape_ascii_escaped_filename + ".hdf5"));
    ValidateDataInMeasurement(hdf5_reader, escape_ascii);
  }

  // Read File 3
  {
    eCAL::eh5::HDF5Meas hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir + "/" + base_name + "_" + escape_ascii_2_escaped_filename + ".hdf5"));
    ValidateDataInMeasurement(hdf5_reader, escape_ascii_2);
  }

  // Read entries with HDF5 dir API
  {
    eCAL::eh5::HDF5Meas hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));
    ValidateDataInMeasurement(hdf5_reader, normal_ascii);
    ValidateDataInMeasurement(hdf5_reader, escape_ascii);
    ValidateDataInMeasurement(hdf5_reader, escape_ascii_2);
  }
}

// This test validates that Datatypinformation is stored to / can be retrieved from the measurement correctly.
TEST(HDF5, WriteReadTopicTypeInformation)
{
  // Define data that will be written to the file
  TestingMeasEntry entry;
  DataTypeInformation info{ "mytype", "myencoding", "mydescriptor" };

  std::string base_name = "datatypeinformation_meas";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    eCAL::eh5::HDF5Meas hdf5_writer;

    if (hdf5_writer.Open(meas_root_dir, eCAL::eh5::eAccessType::CREATE))
    {
      hdf5_writer.SetFileBaseName(base_name);
      hdf5_writer.SetMaxSizePerFile(max_size_per_file);
    }
    else
    {
      FAIL() << "Failed to open HDF5 Writer";
    }

    hdf5_writer.SetChannelDataTypeInformation(entry.channel_name, info);
    EXPECT_TRUE(WriteToHDF(hdf5_writer, entry));

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read entries with HDF5 dir API
  {
    eCAL::eh5::HDF5Meas hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));
    EXPECT_EQ(hdf5_reader.GetChannelDataTypeInformation(entry.channel_name), info);
  }
}

TEST(HDF5, WriteReadTopicTypeInformationDeprecated)
{
  // Define data that will be written to the file
  TestingMeasEntry entry;
  std::string type = "myencoding:mytype";
  std::string descriptor =  "mydescriptor";
  DataTypeInformation info{ "mytype", "myencoding", "mydescriptor" };

  std::string base_name = "datatypeinformation_meas_deprecated";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    eCAL::eh5::HDF5Meas hdf5_writer;

    if (hdf5_writer.Open(meas_root_dir, eCAL::eh5::eAccessType::CREATE))
    {
      hdf5_writer.SetFileBaseName(base_name);
      hdf5_writer.SetMaxSizePerFile(max_size_per_file);
    }
    else
    {
      FAIL() << "Failed to open HDF5 Writer";
    }

    hdf5_writer.SetChannelType(entry.channel_name, type);
    hdf5_writer.SetChannelDescription(entry.channel_name, descriptor);

    EXPECT_TRUE(WriteToHDF(hdf5_writer, entry));

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read entries with HDF5 dir API
  {
    eCAL::eh5::HDF5Meas hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));
    EXPECT_EQ(hdf5_reader.GetChannelType(entry.channel_name), type);
    EXPECT_EQ(hdf5_reader.GetChannelDescription(entry.channel_name), descriptor);
    EXPECT_EQ(hdf5_reader.GetChannelDataTypeInformation(entry.channel_name), info);
  }
}