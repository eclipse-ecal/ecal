#include <writer_hdf5_v6.h>
#include <hdf5_helper.h>

#include <algorithm>
#include <numeric>
#include <set>

using namespace  eCAL::experimental::measurement;
using namespace  eCAL::experimental::measurement::hdf5;


eCAL::experimental::measurement::hdf5::V6Writer::V6Writer(const base::FileOptions& writer_options)
{
  std::string filename = CreateFilename(writer_options);
  root = CreateFile(filename, "6.0");
}

eCAL::experimental::measurement::hdf5::V6Writer::~V6Writer()
{
  WriteChannelSummary();
  CloseFile(root);
}

void eCAL::experimental::measurement::hdf5::V6Writer::SetChannelDataTypeInformation(const base::Channel& channel, const base::DataTypeInformation& info)
{
  channel_summaries[channel].datatype_info = info;
}

void eCAL::experimental::measurement::hdf5::V6Writer::AddEntryToFile(const eCAL::experimental::measurement::base::Channel& channel, const base::WriterEntry& entry)
{
  BinaryEntry binary_entry{ entry.data, entry.size };
  WriteBinaryDataset(root, current_id, binary_entry);

  channel_summaries[channel].entry_info.push_back(CreateEntryInfo(channel, entry));
  current_id++;
}

base::WriterCreator eCAL::experimental::measurement::hdf5::V6Writer::GetCreator()
{
  return [](const base::FileOptions& options_) {
    return std::make_unique<V6Writer>(options_);
  }; 
}

std::string eCAL::experimental::measurement::hdf5::V6Writer::CreateFilename(const base::FileOptions& file_options)
{
  std::string filePath = file_options.path + "/" + file_options.base_name + ".hdf5";
  return filePath;
}

base::EntryInfo eCAL::experimental::measurement::hdf5::V6Writer::CreateEntryInfo(const eCAL::experimental::measurement::base::Channel& channel, const base::WriterEntry& entry) const
{
  base::EntryInfo info;
  info.ID = current_id;
  info.SndTimestamp = entry.snd_timestamp;
  info.RcvTimestamp = entry.rcv_timestamp;
  info.SndID = channel.id;    // in the current file format, the information is redundant!
  info.SndClock = entry.clock;
  return info;
}

void eCAL::experimental::measurement::hdf5::V6Writer::WriteChannelSummary()
{
  for (const auto& entry : channel_summaries)
  {
    v6::WriteChannelSummary(root, entry.first, entry.second);
  }
 
  std::set<std::string> channel_names;
  std::transform(channel_summaries.begin(), channel_summaries.end(), std::inserter(channel_names, channel_names.end()),
    [](const auto& pair) { return pair.first.name; });

  CreateChannelNamesAttribute(root, channel_names);
}

