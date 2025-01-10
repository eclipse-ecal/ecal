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

#include <algorithm>
#include <chrono>
#include <limits>
#include <set>
#include <thread>
#include <sstream>

#include <gtest/gtest.h>

#include <ecalhdf5/eh5_meas.h>
#include <src/hdf5_helper.h> // This header file is usually not available as public include!
#include <src/escape.h> // This header file is usually not available as public include!

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
  namespace eh5
  {
    void PrintTo(const SChannel& channel, std::ostream* os) {
      *os << "(" << channel.name << "," << channel.id << ")";
    }
  }
}


using MeasAPI = eCAL::eh5::v3::HDF5Meas;
using MeasAPIAccess = eCAL::eh5::v3::eAccessType;
using LegacyAPI = eCAL::eh5::v2::HDF5Meas;
using LegacyAPIAccess = eCAL::eh5::v2::eAccessType;
using eCAL::eh5::SChannel;

namespace
{
struct TestingMeasEntry
{
  SChannel channel{ "topic", 1 };
  std::string data = "Hello World";
  long long   snd_timestamp = 1001LL;
  long long   rcv_timestamp = 2001LL;
  long long   snd_id = 0;
  long long   clock = 11LL;
};

std::string print(const TestingMeasEntry& entry) {
  std::stringstream s;
  s << "("
    << entry.channel.name << ", " 
    << entry.channel.id << ", "
    << entry.data << ", " 
    << entry.snd_timestamp << ", "
    << entry.rcv_timestamp << ", "
    << entry.snd_id << ", "
    << entry.clock << ")";
  return s.str();
}

std::string print(const EntryInfo& info) {
  std::stringstream s;
  s << "( rcv: " << info.RcvTimestamp
    << ", id: " << info.ID
    << ", clock: " << info.SndClock
    << ", snd: " << info.SndTimestamp
    << ", snd_id: " << info.SndID
    << ")";
  return s.str();
}

bool MeasEntryEqualsEntryInfo(const TestingMeasEntry& meas_entry, const EntryInfo entry_info)
{
  return meas_entry.snd_id == entry_info.SndID
    && meas_entry.snd_timestamp == entry_info.SndTimestamp
    && meas_entry.rcv_timestamp == entry_info.RcvTimestamp
    && meas_entry.clock == entry_info.SndClock;
}

eCAL::eh5::SWriteEntry Convert(const TestingMeasEntry& entry)
{
  eCAL::eh5::SWriteEntry write_entry;
  write_entry.channel = entry.channel;
  write_entry.data = entry.data.data();
  write_entry.size = entry.data.size();
  write_entry.snd_timestamp = entry.snd_timestamp;
  write_entry.rcv_timestamp = entry.rcv_timestamp;
  write_entry.sender_id = entry.snd_id;
  write_entry.clock = entry.clock;
  return write_entry;
}

bool WriteToHDF(MeasAPI& writer, const TestingMeasEntry& entry)
{
  eCAL::eh5::SWriteEntry write_entry = Convert(entry);
  return writer.AddEntryToFile(write_entry);
}

bool WriteToHDF(LegacyAPI& writer, const TestingMeasEntry& entry)
{
  return writer.AddEntryToFile(
    entry.data.data(),   // data
    entry.data.size(),   // data size
    entry.snd_timestamp, // snd_timestamp
    entry.rcv_timestamp, // rcv_timestamp
    entry.channel.name,  // channel_name
    entry.snd_id,        // channel, id (maybe we shouldn't mix them???)
    entry.clock          // clock
  );
}

// Validates, that if all entries contained in TestingMeasEntry were added to a measurement, they are later contained in the read measurement.
void ValidateChannelsInMeasurementV6(MeasAPI& hdf5_reader, const std::vector<TestingMeasEntry>& entries_in_measurement)
{
  std::set<eCAL::eh5::SChannel> expected_channels;
  std::transform(entries_in_measurement.begin(), entries_in_measurement.end(), std::inserter(expected_channels, expected_channels.begin()),
    [](const TestingMeasEntry& entry) { return entry.channel; });

  EXPECT_EQ(hdf5_reader.GetChannels(), expected_channels);
}

// Validates, that if all entries contained in TestingMeasEntry were added to a measurement, they are later contained in the read measurement.
void ValidateChannelsInMeasurementV5(MeasAPI& hdf5_reader, const std::vector<TestingMeasEntry>& entries_in_measurement)
{
  std::set<eCAL::eh5::SChannel> expected_channels;
  std::transform(entries_in_measurement.begin(), entries_in_measurement.end(), std::inserter(expected_channels, expected_channels.begin()),
    [](const TestingMeasEntry& entry) { return eCAL::eh5::SChannel(entry.channel.name, 0); });

  EXPECT_EQ(hdf5_reader.GetChannels(), expected_channels);
}

// Finds an EntryInfo corresponding to the TestingMeasEntry in the EntryInfoSet
// Return default EntryInfo if it cannot be Found
EntryInfo FindInSet(const eCAL::eh5::EntryInfoSet& info_set, const TestingMeasEntry& to_find)
{
  auto it = std::find_if(info_set.begin(), info_set.end(), [&to_find](const EntryInfo& entry) { 
    return entry.SndClock == to_find.clock && 
           entry.SndID == to_find.snd_id &&
           entry.SndTimestamp == to_find.snd_timestamp &&
           entry.RcvTimestamp == to_find.rcv_timestamp; });

  if (it != info_set.end()) {
    return *it;
  }
  else {
    return EntryInfo{};
  }
}

template <typename Reader>
void ValidateDataInMeasurementGeneric(Reader& hdf5_reader, const TestingMeasEntry& entry, std::function<bool(eCAL::eh5::EntryInfoSet& entries)> entry_info_function)
{
  eCAL::eh5::EntryInfoSet entries_info_set;

  EXPECT_TRUE(entry_info_function(entries_info_set));

  // Get Entry info1 from measurement and see that it checks out
  EntryInfo info = FindInSet(entries_info_set, entry);
  // entry1 was not in the dataset
  if (info == EntryInfo{})
  {
    FAIL() << "Entry info for not found in set / measurement\n" << print(entry);
    return;
  }

  EXPECT_TRUE(MeasEntryEqualsEntryInfo(entry, info)) << print(entry) << " != " << print(info);

  size_t data_size;
  EXPECT_TRUE(hdf5_reader.GetEntryDataSize(info.ID, data_size));

  std::string data_read(data_size, ' ');
  EXPECT_TRUE(hdf5_reader.GetEntryData(info.ID, const_cast<char*>(data_read.data())));

  EXPECT_EQ(data_read, entry.data);
}


// Validate, that the TestingMeasEntry can be read from the measurement
void ValidateDataInMeasurement(MeasAPI& hdf5_reader, const TestingMeasEntry& entry)
{
  auto get_entries_info = [&hdf5_reader, &entry](eCAL::eh5::EntryInfoSet& entries) -> bool
  {
    return hdf5_reader.GetEntriesInfo(entry.channel, entries);
  };

  ValidateDataInMeasurementGeneric(hdf5_reader, entry, get_entries_info);
}

void ValidateDataInMeasurement(LegacyAPI& hdf5_reader, const TestingMeasEntry& entry)
{
  auto get_entries_info = [&hdf5_reader, &entry](eCAL::eh5::EntryInfoSet& entries) -> bool
  {
    return hdf5_reader.GetEntriesInfo(entry.channel.name, entries);
  };

  ValidateDataInMeasurementGeneric(hdf5_reader, entry, get_entries_info);
}


TestingMeasEntry m1{
 {"topic / with / slash", 1},
 //{"t1", 1},
 "Hello World",
 1001LL,
 2001LL,
 0,
 11LL,
};

TestingMeasEntry m2{
  {"another,topic", 2},
  //{"t2", 2},
  "",
  1002LL,
  2002LL,
  0,
  12LL
};

TestingMeasEntry m3{
  {" ASCII and beyond!\a\b\t\n\v\f\r\"#$%&\'()*+,-./0123456789:,<=>\?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~üöäÜÖÄâÂôÔûÛáàÁÀúÙ", 3},
  //{"t3", 3},
  "o.O",
  1003LL,
  2003LL,
  0,
  13LL
};

TestingMeasEntry topic_1
{
  {"topic_1", 1},
  "topic1: test data",
  1001LL,
  2001LL,
  0,
  11LL
};

TestingMeasEntry topic_2
{
  {"topic_2", 2},
  "topic2: test data",
  1002LL,
  2002LL,
  1000,
  12LL,
};


  // properties
  std::string output_dir         = "measurement_dir";
  const size_t max_size_per_file = 500;  // MB
  std::vector<char> data;
}

template <typename Meas, typename AccessType>
void CreateMeasurement(Meas& hdf5_writer, std::string root_dir, std::string base_name, AccessType access_type = AccessType::CREATE)
{
  if (hdf5_writer.Open(root_dir, access_type))
  {
    hdf5_writer.SetFileBaseName(base_name);
    hdf5_writer.SetMaxSizePerFile(max_size_per_file);
  }
  else
  {
    FAIL() << "Failed to open HDF5 Writer";
  }
}

TEST(contrib, HDF5_EscapeUnescape)
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



TEST(contrib, HDF5_WriteReadIntegrity)
{
  // Define data that will be written to the file

  std::string base_name = "integrity_meas";
  std::string meas_root_dir = output_dir + "/" + base_name;

  std::vector<TestingMeasEntry> meas_entries{ m1, m2, m3 };

  // Write HDF5 file
  {
    MeasAPI hdf5_writer;
    CreateMeasurement<MeasAPI, MeasAPIAccess>(hdf5_writer, meas_root_dir, base_name);

    for (const auto& entry : meas_entries)
    {
      EXPECT_TRUE(WriteToHDF(hdf5_writer, entry));
    }

    EXPECT_TRUE(hdf5_writer.Close());
  }
    
  std::set<eCAL::eh5::SChannel> expected_channels{ m1.channel, m2.channel, m3.channel };

  // Read HDF5 file
  {
    MeasAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir + "/" + base_name + ".hdf5"));

    auto actual_channels = hdf5_reader.GetChannels();
    EXPECT_EQ(actual_channels, expected_channels);

    ValidateChannelsInMeasurementV6(hdf5_reader, meas_entries);
    for (const auto& entry : meas_entries)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
   }

  // Read entries with HDF5 dir API
  {
    MeasAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));

    auto actual_channels = hdf5_reader.GetChannels();
    EXPECT_EQ(actual_channels, expected_channels);
    
    ValidateChannelsInMeasurementV6(hdf5_reader, meas_entries);
    for (const auto& entry : meas_entries)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
  }
}

TEST(contrib, HDF5_ReadWrite)
{
  std::string file_name = "meas_readwrite";

  // Create writer instance
  MeasAPI writer(output_dir, MeasAPIAccess::CREATE);
  writer.SetFileBaseName(file_name);
  writer.SetMaxSizePerFile(max_size_per_file);

  data.resize(1024);
  eCAL::eh5::SWriteEntry entry;
  entry.channel = CreateChannel("myChannel");
  entry.data = static_cast<void*>(data.data());
  entry.size = data.size();
  EXPECT_TRUE(writer.AddEntryToFile(entry));

  EXPECT_TRUE(writer.Close());
}


TEST(contrib, HDF5_IsOneFilePerChannelEnabled)
{
  MeasAPI hdf5_writer;
  std::string base_name = "output";

  if (hdf5_writer.Open(output_dir, MeasAPIAccess::CREATE))
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

TEST(contrib, HDF5_SetOneFilePerChannelEnabled)
{
  std::string base_name = "output";
  
  // Write HDF5 file
  {
    MeasAPI hdf5_writer;

    if (hdf5_writer.Open(output_dir, MeasAPIAccess::CREATE))
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
    MeasAPI hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(output_dir + "/" + base_name + "_" + topic_1.channel.name + ".hdf5"));
    ValidateChannelsInMeasurementV6(hdf5_reader, { topic_1 });
    ValidateDataInMeasurement(hdf5_reader, topic_1);
  }

  //  Read HDF5 topic 2 file
  {
    MeasAPI hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(output_dir + "/" + base_name + "_" + topic_2.channel.name + ".hdf5"));

    ValidateChannelsInMeasurementV6(hdf5_reader, { topic_2 });
    ValidateDataInMeasurement(hdf5_reader, topic_2);
  }

}

TEST(contrib, HDF5_EscapeFilenamesForOneFilePerChannel)
{
  // Define data that will be written to the file
  TestingMeasEntry normal_ascii = topic_1;
  normal_ascii.channel.name = "NormalAscii";
  const std::string normal_ascii_escaped_filename = "NormalAscii";

  TestingMeasEntry escape_ascii = topic_1;
  escape_ascii.channel.name = "Escape these: \\/*\"<>|%";
  const std::string escape_ascii_escaped_filename = "Escape these%3A %5C%2F%2A%22%3C%3E%7C%25";

  TestingMeasEntry escape_ascii_2 = topic_1;
  escape_ascii_2.channel.name = "Don't escape these +~#,;.²³🍺";
  const std::string escape_ascii_2_escaped_filename = "Don't escape these +~#,;.²³\xF0\x9F\x8D\xBA";

  std::string base_name = "escape_filename_meas";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    MeasAPI hdf5_writer;

    if (hdf5_writer.Open(meas_root_dir, MeasAPIAccess::CREATE))
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
    MeasAPI hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir + "/" + base_name + "_" + normal_ascii_escaped_filename + ".hdf5"));
    ValidateDataInMeasurement(hdf5_reader, normal_ascii);
  }

  // Read File 2
  {
    MeasAPI hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir + "/" + base_name + "_" + escape_ascii_escaped_filename + ".hdf5"));
    ValidateDataInMeasurement(hdf5_reader, escape_ascii);
  }

  // Read File 3
  {
    MeasAPI hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir + "/" + base_name + "_" + escape_ascii_2_escaped_filename + ".hdf5"));
    ValidateDataInMeasurement(hdf5_reader, escape_ascii_2);
  }

  // Read entries with HDF5 dir API
  {
    MeasAPI hdf5_reader;

    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));
    ValidateDataInMeasurement(hdf5_reader, normal_ascii);
    ValidateDataInMeasurement(hdf5_reader, escape_ascii);
    ValidateDataInMeasurement(hdf5_reader, escape_ascii_2);
  }
}

// This test validates that Datatypinformation is stored to / can be retrieved from the measurement correctly.
TEST(contrib, HDF5_WriteReadTopicTypeInformation)
{
  // Define data that will be written to the file
  TestingMeasEntry entry;
  const auto& channel = entry.channel;
  DataTypeInformation info{ "mytype", "myencoding", "mydescriptor" };

  std::string base_name = "datatypeinformation_meas";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    MeasAPI hdf5_writer;
    CreateMeasurement<MeasAPI, MeasAPIAccess>(hdf5_writer, meas_root_dir, base_name);

    hdf5_writer.SetChannelDataTypeInformation(channel, info);
    EXPECT_TRUE(WriteToHDF(hdf5_writer, entry));

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read entries with HDF5 dir API
  {
    MeasAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));
    EXPECT_EQ(hdf5_reader.GetChannelDataTypeInformation(channel), info);
  }
}

/*
* This test verifies that you save different descriptors for the same channel
* And later read them from the measurement 
*/
TEST(HDF5, WriteReadMultipleTopicTypeInformation)
{
  TestingMeasEntry entry1;
  entry1.channel.id = 1;
  TestingMeasEntry entry2;
  entry2.channel.id = 2;
  TestingMeasEntry entry3;
  entry3.channel.id = 3;

  const auto& channel1 = entry1.channel;
  const auto& channel2 = entry2.channel;
  const auto& channel3 = entry3.channel;

  DataTypeInformation info1{ "mytype", "myencoding", "mydescriptor" };
  DataTypeInformation info2{ "mytype2", "myencoding2", "mydescriptor2" };
  // take one info whith \0 character, as protobuf descriptors may contain those
  DataTypeInformation info3{ "mytype3", "myencoding3", std::string("my\0\ndescriptor3", 15)};
  EXPECT_EQ(info3.descriptor.size(), 15);

  std::string base_name = "datatypeinformation_multiple";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    MeasAPI hdf5_writer;
    CreateMeasurement<MeasAPI, MeasAPIAccess>(hdf5_writer, meas_root_dir, base_name);

    hdf5_writer.SetChannelDataTypeInformation(channel1, info1);
    EXPECT_TRUE(WriteToHDF(hdf5_writer, entry1));

    hdf5_writer.SetChannelDataTypeInformation(channel2, info2);
    EXPECT_TRUE(WriteToHDF(hdf5_writer, entry2));

    hdf5_writer.SetChannelDataTypeInformation(channel3, info3);
    EXPECT_TRUE(WriteToHDF(hdf5_writer, entry3));

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read entries with HDF5 dir API
  {
    MeasAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));
    EXPECT_EQ(hdf5_reader.GetChannelDataTypeInformation(channel1), info1);
    EXPECT_EQ(hdf5_reader.GetChannelDataTypeInformation(channel2), info2);
    EXPECT_EQ(hdf5_reader.GetChannelDataTypeInformation(channel3), info3);
    ValidateDataInMeasurement(hdf5_reader, entry1);
    ValidateDataInMeasurement(hdf5_reader, entry2);
    ValidateDataInMeasurement(hdf5_reader, entry3);
  }
}

// This test checks two cases for the old API.
// We write values with ID 0, and with ID != 0
// Both descriptors should be read from the measurement successfully
TEST(contrib, HDF5_WriteReadTopicTypeInformationDeprecated)
{
  // Define data that will be written to the file
  TestingMeasEntry entry_zero{ {"topic_zero",0}, "topic2: test data", 1001, 1002,  1000,  0 };
  TestingMeasEntry entry_nonzero{ {"topic_nonzerozero",0xAAAA}, "topic2: test data", 1001, 1002,  2000,  0 };

  std::string type = "myencoding:mytype";
  std::string descriptor =  "mydescriptor";
  DataTypeInformation info{ "mytype", "myencoding", "mydescriptor" };

  std::string base_name = "datatypeinformation_meas_deprecated";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    LegacyAPI hdf5_writer;
    CreateMeasurement<LegacyAPI, LegacyAPIAccess>(hdf5_writer, meas_root_dir, base_name);

    hdf5_writer.SetChannelType(entry_zero.channel.name, type);
    hdf5_writer.SetChannelDescription(entry_zero.channel.name, descriptor);

    hdf5_writer.SetChannelType(entry_nonzero.channel.name, type);
    hdf5_writer.SetChannelDescription(entry_nonzero.channel.name, descriptor);

    EXPECT_TRUE(WriteToHDF(hdf5_writer, entry_zero));
    EXPECT_TRUE(WriteToHDF(hdf5_writer, entry_nonzero));

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read entries with HDF5 dir API
  {
    LegacyAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));
    EXPECT_EQ(hdf5_reader.GetChannelType(entry_zero.channel.name), type);
    EXPECT_EQ(hdf5_reader.GetChannelDescription(entry_zero.channel.name), descriptor);
    EXPECT_EQ(hdf5_reader.GetChannelType(entry_nonzero.channel.name), type);
    EXPECT_EQ(hdf5_reader.GetChannelDescription(entry_nonzero.channel.name), descriptor);

    EXPECT_EQ(hdf5_reader.GetChannelDataTypeInformation({ entry_zero.channel.name, 0 }), info);
    EXPECT_EQ(hdf5_reader.GetChannelDataTypeInformation({ entry_nonzero.channel.name, 0 }), info);
  }
}

// This tests confirms if you write with the new API, you can read the information with the old API
TEST(HDF5, WriteReadTopicTypeInformationDeprecated2)
{
  // Define data that will be written to the file
  TestingMeasEntry entry_topic_1{ {"topic_1", 1}, "topic2: test data", 1001, 1002,  0xAAAA,  0 };
  const auto& channel_topic_1 = entry_topic_1.channel;

  TestingMeasEntry entry_topic_2{ {"topic_2", 2}, "topic2: test data", 1001, 1002,  0xAAAA,  0 };
  const auto& channel_topic_2 = entry_topic_2.channel;

  std::string type_1 = "myencoding:mytype";
  std::string descriptor_1 = "mydescriptor";
  DataTypeInformation info_1{ "mytype", "myencoding", "mydescriptor" };

  std::string type_2 = "myencoding2:mytype2";
  std::string descriptor_2 = "mydescriptor2";
  DataTypeInformation info_2{ "mytype2", "myencoding2", "mydescriptor2" };

  std::string base_name = "datatypeinformation_meas_deprecated_2";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    MeasAPI hdf5_writer;
    CreateMeasurement<MeasAPI, MeasAPIAccess>(hdf5_writer, meas_root_dir, base_name);
    
    hdf5_writer.SetChannelDataTypeInformation(channel_topic_1, info_1);
    EXPECT_TRUE(WriteToHDF(hdf5_writer, entry_topic_1));

    hdf5_writer.SetChannelDataTypeInformation(channel_topic_2, info_2);
    EXPECT_TRUE(WriteToHDF(hdf5_writer, entry_topic_2));


    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read entries with HDF5 dir API
  {
    LegacyAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));
    EXPECT_EQ(hdf5_reader.GetChannelType(channel_topic_1.name), type_1);
    EXPECT_EQ(hdf5_reader.GetChannelDescription(channel_topic_1.name), descriptor_1);

    EXPECT_EQ(hdf5_reader.GetChannelType(channel_topic_2.name), type_2);
    EXPECT_EQ(hdf5_reader.GetChannelDescription(channel_topic_2.name), descriptor_2);
  }
}


// This tests confirms if you write with the new API, you can read the information with the old API
TEST(HDF5, MinMaxTimestamps)
{
  Channel::id_t id_1 = 0xAAAA;
  Channel::id_t id_2 = 0x00AA;
  std::string topic_name = "topic";

  eCAL::eh5::SChannel channel_1{ topic_name, id_1 };
  eCAL::eh5::SChannel channel_2{ topic_name, id_2 };

  // Define data that will be written to the file
  std::vector<TestingMeasEntry> meas_entries = {
    TestingMeasEntry{ {topic_name, id_1}, "topic2: test data", 1001, 1002,  0,  0 },
    TestingMeasEntry{ {topic_name, id_1}, "topic2: test data", 2001, 2002,  0,  1 },
    TestingMeasEntry{ {topic_name, id_2}, "topic2: test data", 2051, 2052,  0,  0 },
    TestingMeasEntry{ {topic_name, id_1}, "topic2: test data", 3001, 3002,  0,  2 },
    TestingMeasEntry{ {topic_name, id_2}, "topic2: test data", 3051, 3052,  0,  1 },
    TestingMeasEntry{ {topic_name, id_1}, "topic2: test data", 4001, 4002,  0,  3 },
    TestingMeasEntry{ {topic_name, id_1}, "topic2: test data", 5001, 5002,  0,  4 },
  };

  std::string base_name = "min_max_timestamps";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    MeasAPI hdf5_writer;
    CreateMeasurement<MeasAPI, MeasAPIAccess>(hdf5_writer, meas_root_dir, base_name);

    for (const auto& entry : meas_entries)
    {
      EXPECT_TRUE(WriteToHDF(hdf5_writer, entry));
    }

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read entries with HDF5 dir API
  {
    MeasAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));
    // New API
    EXPECT_EQ(hdf5_reader.GetMinTimestamp(channel_1), 1002) << "Min channel 1 should be 1002";
    EXPECT_EQ(hdf5_reader.GetMinTimestamp(channel_2), 2052) << "Min channel 2 should be 2052";
    
    // New API
    EXPECT_EQ(hdf5_reader.GetMaxTimestamp(channel_1), 5002) << "Max channel 1 should be 5002";
    EXPECT_EQ(hdf5_reader.GetMaxTimestamp(channel_2), 3052) << "Max channel 2 should be 3052";

    for (const auto& entry : meas_entries)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
  }

  // Read entries with HDF5 dir API
  {
    LegacyAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));

    // Deprecated API
    EXPECT_EQ(hdf5_reader.GetMinTimestamp(topic_name), 1002) << "Min topic should be 1002";
    // Deprecated API
    EXPECT_EQ(hdf5_reader.GetMaxTimestamp(topic_name), 5002) << "Min topic should be 5002";

    // TODO This won't work, I guess
    for (const auto& entry : meas_entries)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
  }

}

/*
* This test checks that we can correctly merge multiple measurements
* 
*/
TEST(HDF5, MergedMeasurements)
{
  Channel::id_t id_1 = 0xAAAA;
  Channel::id_t id_2 = 0x00AA;
  Channel::id_t id_3 = 0x0001;
  std::string topic_name_1_2 = "topic";
  std::string topic_name_3   = "topic_3";

  eCAL::eh5::SChannel channel_1{ topic_name_1_2, id_1 };
  eCAL::eh5::SChannel channel_2{ topic_name_1_2, id_2 };
  eCAL::eh5::SChannel channel_3{ topic_name_3, id_3 };

  DataTypeInformation info_1{ "mytype1", "myencoding1", "mydescriptor1" };
  DataTypeInformation info_2{ "mytype2", "myencoding2", "mydescriptor2" };
  DataTypeInformation info_3{ "mytype3", "myencoding3", "mydescriptor3" };

  // Define data that will be written to the file
  std::vector<TestingMeasEntry> entries_meas_1 = {
    TestingMeasEntry{ {topic_name_1_2, id_1}, "topic2: test data", 0001, 0002, 0 ,  0 },
    TestingMeasEntry{ {topic_name_1_2, id_1}, "topic2: test data", 2001, 2002, 0 ,  1 },
    TestingMeasEntry{ {topic_name_1_2, id_1}, "topic2: test data", 3001, 3002, 0 ,  2 },
    TestingMeasEntry{ {topic_name_1_2, id_1}, "topic2: test data", 4001, 4002, 0 ,  3 },
    TestingMeasEntry{ {topic_name_1_2, id_1}, "topic2: test data", 5001, 5002, 0 ,  4 },
  };

  std::vector<TestingMeasEntry> entries_meas_2 = {
    TestingMeasEntry{ {topic_name_3  , id_3 }, "topic2: test data", 1001, 1002, 0 ,  0 },
    TestingMeasEntry{ {topic_name_3  , id_3 }, "topic2: test data", 2001, 2002, 0 ,  1 },
    TestingMeasEntry{ {topic_name_1_2, id_2 }, "topic2: test data", 2051, 2052, 0 ,  0 },
    TestingMeasEntry{ {topic_name_3  , id_3 }, "topic2: test data", 3001, 3002, 0 ,  2 },
    TestingMeasEntry{ {topic_name_1_2, id_2 }, "topic2: test data", 3051, 3052, 0 ,  1 },
    TestingMeasEntry{ {topic_name_3  , id_3 }, "topic2: test data", 6001, 6002, 0 ,  3 },
    TestingMeasEntry{ {topic_name_3  , id_3 }, "topic2: test data", 7001, 7002, 0 ,  4 },
  };

  std::string base_dir   = output_dir + "/merged_measurement";
  std::string meas_dir_1 = base_dir + "/meas1";
  std::string meas_dir_2 = base_dir + "/meas2";

  // Write first HDF5 file
  {
    MeasAPI hdf5_writer;
    CreateMeasurement<MeasAPI, MeasAPIAccess>(hdf5_writer, meas_dir_1, "meas1");

    for (const auto& entry : entries_meas_1)
    {
      EXPECT_TRUE(WriteToHDF(hdf5_writer, entry));
    }

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Write second HDF5 file
  {
    MeasAPI hdf5_writer;
    CreateMeasurement<MeasAPI, MeasAPIAccess>(hdf5_writer, meas_dir_1, "meas2");

    for (const auto& entry : entries_meas_2)
    {
      EXPECT_TRUE(WriteToHDF(hdf5_writer, entry));
    }

    EXPECT_TRUE(hdf5_writer.Close());
  }

  {
    MeasAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(base_dir));

    std::set<Channel> expected_channels{ channel_1, channel_2, channel_3 };
    EXPECT_EQ(hdf5_reader.GetChannels(), expected_channels);

    // New API
    EXPECT_EQ(hdf5_reader.GetMinTimestamp(channel_1), 0002);
    EXPECT_EQ(hdf5_reader.GetMinTimestamp(channel_2), 2052);
    EXPECT_EQ(hdf5_reader.GetMinTimestamp(channel_3), 1002);

    // New API
    EXPECT_EQ(hdf5_reader.GetMaxTimestamp(channel_1), 5002);
    EXPECT_EQ(hdf5_reader.GetMaxTimestamp(channel_2), 3052);
    EXPECT_EQ(hdf5_reader.GetMaxTimestamp(channel_3), 7002);

    // Check that all entries are present in the measurement
    for (const auto& entry : entries_meas_1)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
    for (const auto& entry : entries_meas_2)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
  }

  {
    LegacyAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(base_dir));

    std::set<std::string> expected_channels{ topic_name_1_2, topic_name_3 };
    EXPECT_EQ(hdf5_reader.GetChannelNames(), expected_channels);

    // Deprecated API
    EXPECT_EQ(hdf5_reader.GetMinTimestamp(topic_name_1_2), 0002);
    EXPECT_EQ(hdf5_reader.GetMinTimestamp(topic_name_3), 1002);

    // Deprecated API
    EXPECT_EQ(hdf5_reader.GetMaxTimestamp(topic_name_1_2), 5002);
    EXPECT_EQ(hdf5_reader.GetMaxTimestamp(topic_name_3), 7002);

    for (const auto& entry : entries_meas_1)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
    for (const auto& entry : entries_meas_2)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
  }

}

// We don't write empty measurements.
// If we change the implementation, we can reactivate this test
TEST(HDF5, DISABLED_WriteReadEmptyMeasurement)
{
  eCAL::eh5::SChannel channel{ "topic", 0};
  DataTypeInformation info{ "mytype", "myencoding", "mydescriptor" };

  std::string base_name = "empty_measurement";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    MeasAPI hdf5_writer;
    CreateMeasurement<MeasAPI, MeasAPIAccess>(hdf5_writer, meas_root_dir, base_name);
    hdf5_writer.SetChannelDataTypeInformation(channel, info);

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read entries with HDF5 dir API
  {
    MeasAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));

    EXPECT_EQ(hdf5_reader.GetChannelDataTypeInformation(channel), info);

    std::set<eCAL::eh5::SChannel> channels{ channel };
    EXPECT_EQ(hdf5_reader.GetChannels(), channels);

    EXPECT_EQ(hdf5_reader.GetMinTimestamp(channel), 0);
    EXPECT_EQ(hdf5_reader.GetMaxTimestamp(channel), 0);
  }
}

// This tests confirms if you write with the old API, you can read the information both old and new api
TEST(HDF5, TestReaderWriterV5)
{
  Channel::id_t id_1 = 0xAAAA;
  Channel::id_t id_2 = 0x00AA;
  std::string topic_name = "topic";

  eCAL::eh5::SChannel channel_1{ topic_name, id_1 };
  eCAL::eh5::SChannel channel_2{ topic_name, id_2 };

  // Define data that will be written to the file
  std::vector<TestingMeasEntry> meas_entries = {
    TestingMeasEntry{ {topic_name, 0}, "topic: test data", 1001, 1002, 0, 0 },
    TestingMeasEntry{ {topic_name, 0}, "topic: test data", 2001, 2002, 0, 1 },
    TestingMeasEntry{ {topic_name, 0}, "topic: test data", 2051, 2052, 0, 0 },
    TestingMeasEntry{ {topic_name, 0}, "topic: test data", 3001, 3002, 0, 2 },
    TestingMeasEntry{ {topic_name, 0}, "topic: test data", 3051, 3052, 0, 1 },
    TestingMeasEntry{ {topic_name, 0}, "topic: test data", 4001, 4002, 0, 3 },
    TestingMeasEntry{ {topic_name, 0}, "topic: test data", 5001, 5002, 0, 4 },
  };

  std::string base_name = "read_write_v5";
  std::string meas_root_dir = output_dir + "/" + base_name;

  // Write HDF5 file
  {
    LegacyAPI hdf5_writer;
    CreateMeasurement<LegacyAPI, LegacyAPIAccess>(hdf5_writer, meas_root_dir, base_name);

    for (const auto& entry : meas_entries)
    {
      EXPECT_TRUE(WriteToHDF(hdf5_writer, entry));
    }

    EXPECT_TRUE(hdf5_writer.Close());
  }

  // Read entries with HDF5 dir API
  {
    LegacyAPI hdf5_reader;
    EXPECT_TRUE(hdf5_reader.Open(meas_root_dir));
    EXPECT_EQ(hdf5_reader.GetFileVersion(), "5.0");

    // Deprecated API
    EXPECT_EQ(hdf5_reader.GetMinTimestamp(topic_name), 1002);

    // Deprecated API
    EXPECT_EQ(hdf5_reader.GetMaxTimestamp(topic_name), 5002);


    for (const auto& entry : meas_entries)
    {
      ValidateDataInMeasurement(hdf5_reader, entry);
    }
  }
}




TEST(HDF5, ParsePrintHex)
{
  std::vector<std::string> hex_values =
  {
    "0000000000000000",
    "0000000000000001",
    "000000000000000F",
    "ABCDEF0123456789",
    "EFFFFFFFFFFFFFFF",
    "FFFFFFFFFFFFFFFF"
  };

  for (const auto& value : hex_values)
  {
    auto int_value = parseHexID(value);
    auto string_value = printHex(int_value);
    EXPECT_EQ(string_value, value);
  }
}

TEST(HDF5, PrintParseHex)
{
  std::vector<Channel::id_t> numeric_values =
  {
    0,
    1,
    std::numeric_limits<Channel::id_t>::min(),
    std::numeric_limits<Channel::id_t>::max()
  };

  for (const auto& value : numeric_values)
  {
    auto string_value = printHex(value);
    auto int_value = parseHexID(string_value);

    EXPECT_EQ(int_value, value);
  }
}


