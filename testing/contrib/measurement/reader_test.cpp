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

#include <gtest/gtest.h>
#include <ecal/measurement/base/writer.h>
#include <ecal/measurement/hdf5/writer.h>

#include <writer_escaping.h>
#include <writer_splitting.h>

#include <escape.h>

#include <memory>


using namespace eCAL::experimental::measurement::base;
using namespace eCAL::experimental::measurement::hdf5;

struct StatisticInfo
{
  DataTypeInformation   datatype;
  std::set<WriterEntry> entries;
};

struct WriterHandle
{
  int id;

  bool operator<(const WriterHandle& other) const
  {
    return id < other.id;
  }
};

class Statistics
{
public:
  WriterHandle RegisterConstructorCall(const FileOptions& options)
  {
    WriterHandle handle{ number_constructor_calls };
    number_constructor_calls++;
  }

  void RegisterDatatypeInformation(const WriterHandle& options, const Channel& channel, const DataTypeInformation& info)
  {
    entries[options][channel].datatype = info;
  }

  void RegisterEntries(const WriterHandle& options, const Channel& channel, const WriterEntry& entry)
  {
    entries[options][channel].entries.insert(entry);
  }

  int number_constructor_calls;
  std::map<WriterHandle, FileOptions> options;
  std::map<WriterHandle, std::map<Channel, StatisticInfo>> entries;
};

class MockingWriter : public Writer
{
public:
  MockingWriter(const FileOptions& options_, const std::shared_ptr<Statistics>& statistics_)
    : options (options_)
    , staticstics(statistics_)
  {
    handle = staticstics->RegisterConstructorCall(options);
  }

  void SetChannelDataTypeInformation(const eCAL::experimental::measurement::base::Channel& channel, const DataTypeInformation& info) override
  {
    staticstics->RegisterDatatypeInformation(handle, channel, info);
  }

  void AddEntryToFile(const eCAL::experimental::measurement::base::Channel& channel, const WriterEntry& entry) override
  {
    staticstics->RegisterEntries(handle, channel, entry);
  }

private:
  FileOptions options;
  std::shared_ptr<Statistics> staticstics;
  WriterHandle handle;
};


struct TestingMeasEntry
{
  std::string channel_name = "topic";
  std::string data = "Hello World";
  long long   snd_timestamp = 1001LL;
  long long   rcv_timestamp = 2001LL;
  long long   id = 1LL;
  long long   clock = 11LL;
};


TEST(Measurement, WriterEscapingTopicnames)
{
  std::shared_ptr<Statistics> statistics = std::make_shared<Statistics>();

  WriterCreator create_mockingWriter = [&statistics](const FileOptions& options){
    return std::make_unique<MockingWriter>(options, statistics);
  };
  FileOptions options;
  options.path = "/usr/mypath";
  options.base_name = "test";

  TestingMeasEntry normal_ascii;
  normal_ascii.channel_name = "NormalAscii";

  TestingMeasEntry escape_ascii;
  escape_ascii.channel_name = "Escape these: \\/*\"<>|%";

  TestingMeasEntry escape_ascii_2;
  escape_ascii_2.channel_name = "Don't escape these +~#,;.²³🍺";


  auto writer_creator = EscapingWriter::GetCreator(create_mockingWriter);
  auto writer = writer_creator(options);
}


TEST(Measurement, EscapeUnescape)
{
  std::string test_string = "This string contains commata,slashes/ spaces, a percent sign (%), a bell\a, nasty line endings\r\n and a german letter oe from the Latin-1 codepage: \xF8";
  std::string expected_escaped_string = "This string contains commata%2Cslashes%2F spaces%2C a percent sign (%25)%2C a bell%07%2C nasty line endings%0D%0A and a german letter oe from the Latin-1 codepage: %F8";

  std::string escaped_string = eCAL::GetEscapedTopicname(test_string);

  EXPECT_EQ(escaped_string, expected_escaped_string);
  EXPECT_EQ(eCAL::GetUnescapedString(escaped_string), test_string);

  std::string faulty_escaped_string = "This%20String%20has some escapes, a % that is not escaped / one at the end %";
  std::string expected_unescaped_string = "This String has some escapes, a % that is not escaped / one at the end %";
  EXPECT_EQ(eCAL::GetUnescapedString(faulty_escaped_string), expected_unescaped_string);
}